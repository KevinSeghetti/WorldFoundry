//=============================================================================
// gfx/egl/egl.cc: egl specific portion of interface, included by display.cc
// currently includes X windows interface portions as well
// Copyright ( c ) 2012 World Foundry Group  
// Part of the World Foundry 3D video game engine/production environment
// for more information about World Foundry, see www.worldfoundry.org
//==============================================================================
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// Version 2 as published by the Free Software Foundation
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// or see www.fsf.org

// ===========================================================================
// Description: The Display class encapsulates data and behavior for a single
//       hardware screen
// Original Author: Kevin T. Seghetti
//============================================================================

// kts why is this here? 
struct HalDisplay
{
    XDisplay *mainDisplay;
    EGLDisplay eglDisplay;
    EGLSurface eglSurface;
    EGLContext eglContext;

    //int screenIndex;
    //XVisualInfo *visInfo;
    Window win;
};

HalDisplay halDisplay;

//===============================================================================

#include <time.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <GLES2/gl2.h>  /* use OpenGL ES 2.x */
#include <EGL/egl.h>

static GLint u_matrix = -1;
static GLint attr_pos = 0, attr_color = 1;

static void
create_shaders(void)
{
   static const char *fragShaderText =
      "varying vec4 v_color;\n"
      "void main() {\n"
      "   gl_FragColor = v_color;\n"
      "}\n";
   static const char *vertShaderText =
      "uniform mat4 modelviewProjection;\n"
      "attribute vec4 pos;\n"
      "attribute vec4 color;\n"
      "varying vec4 v_color;\n"
      "void main() {\n"
      "   gl_Position = modelviewProjection * pos;\n"
      "   float clippedZ = clamp(gl_Position.z,0.0,127.0);"
      "   float hsz = 110.0/clippedZ;"
      "   hsz = clamp(hsz,-512.0,512.0);"
      "   gl_Position.x *= hsz;"
      "   gl_Position.y *= hsz;" 
      "   gl_Position.x *= 0.01;"
      "   gl_Position.y *= 0.01;" 
      "   v_color = color;\n"
      "   gl_Position.z = clamp(gl_Position.z,0.1,0.9);\n"
      "}\n";

   GLuint fragShader, vertShader, program;
   GLint stat;

   fragShader = glCreateShader(GL_FRAGMENT_SHADER);
   glShaderSource(fragShader, 1, (const char **) &fragShaderText, NULL);
   glCompileShader(fragShader);
   glGetShaderiv(fragShader, GL_COMPILE_STATUS, &stat);
   if (!stat) {
      printf("Error: fragment shader did not compile!\n");
#define LOG_LENGTH 5000

      char infoLog[LOG_LENGTH+1];
      int length;
      glGetShaderInfoLog(fragShader,LOG_LENGTH,&length,infoLog);
      infoLog[length]=0;
      printf("log = \n");
      printf("%s\n",infoLog);

      exit(1);
   }

   vertShader = glCreateShader(GL_VERTEX_SHADER);
   glShaderSource(vertShader, 1, (const char **) &vertShaderText, NULL);
   glCompileShader(vertShader);
   glGetShaderiv(vertShader, GL_COMPILE_STATUS, &stat);
   if (!stat) {
      char infoLog[LOG_LENGTH+1];
      int length;
      glGetShaderInfoLog(vertShader,LOG_LENGTH,&length,infoLog);
      infoLog[length]=0;
      printf("Error: vertex shader did not compile!\n");
      printf("log = \n");
      printf("%s\n",infoLog);
      exit(1);
   }

   program = glCreateProgram();
   glAttachShader(program, fragShader);
   glAttachShader(program, vertShader);
   glLinkProgram(program);

   glGetProgramiv(program, GL_LINK_STATUS, &stat);
   if (!stat) {
      char log[1000];
      GLsizei len;
      glGetProgramInfoLog(program, 1000, &len, log);
      printf("Error: linking:\n%s\n", log);
      exit(1);
   }

   glUseProgram(program);

   if (1) {
      /* test setting attrib locations */
      glBindAttribLocation(program, attr_pos, "pos");
      glBindAttribLocation(program, attr_color, "color");
      glLinkProgram(program);  /* needed to put attribs into effect */
   }
   else {
      /* test automatic attrib locations */
      attr_pos = glGetAttribLocation(program, "pos");
      attr_color = glGetAttribLocation(program, "color");
   }

   u_matrix = glGetUniformLocation(program, "modelviewProjection");
   printf("Uniform modelviewProjection at %d\n", u_matrix);
   printf("Attrib pos at %d\n", attr_pos);
   printf("Attrib color at %d\n", attr_color);
}


/*
 * Create an RGB, double-buffered X window.
 * Return the window and context handles.
 */
static void
make_x_window(XDisplay *x_dpy, EGLDisplay egl_dpy,
              const char *name,
              int x, int y, int width, int height,
              Window *winRet,
              EGLContext *ctxRet,
              EGLSurface *surfRet)
{
   static const EGLint attribs[] = {
      EGL_RED_SIZE, 1,
      EGL_GREEN_SIZE, 1,
      EGL_BLUE_SIZE, 1,
      EGL_DEPTH_SIZE, 16,
      EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
      EGL_NONE
   };
   static const EGLint ctx_attribs[] = {
      EGL_CONTEXT_CLIENT_VERSION, 2,
      EGL_NONE
   };
   int scrnum;
   XSetWindowAttributes attr;
   unsigned long mask;
   Window root;
   Window win;
   XVisualInfo *visInfo, visTemplate;
   int num_visuals;
   EGLContext ctx;
   EGLConfig config;
   EGLint num_configs;
   EGLint vid;

   scrnum = DefaultScreen( x_dpy );
   root = RootWindow( x_dpy, scrnum );

   if (!eglChooseConfig( egl_dpy, attribs, &config, 1, &num_configs)) {
      printf("Error: couldn't get an EGL visual config\n");
      exit(1);
   }

   assert(config);
   assert(num_configs > 0);

   if (!eglGetConfigAttrib(egl_dpy, config, EGL_NATIVE_VISUAL_ID, &vid)) {
      printf("Error: eglGetConfigAttrib() failed\n");
      exit(1);
   }

   /* The X window visual must match the EGL config */
   visTemplate.visualid = vid;
   visInfo = XGetVisualInfo(x_dpy, VisualIDMask, &visTemplate, &num_visuals);
   if (!visInfo) {
      printf("Error: couldn't get X visual\n");
      exit(1);
   }

   /* window attributes */
   attr.background_pixel = 0;
   attr.border_pixel = 0;
   attr.colormap = XCreateColormap( x_dpy, root, visInfo->visual, AllocNone);
   attr.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask | KeyReleaseMask;
   mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

   win = XCreateWindow( x_dpy, root, 0, 0, width, height,
                0, visInfo->depth, InputOutput,
                visInfo->visual, mask, &attr );

   /* set hints and properties */
   {
      XSizeHints sizehints;
      sizehints.x = x;
      sizehints.y = y;
      sizehints.width  = width;
      sizehints.height = height;
      sizehints.flags = USSize | USPosition;
      XSetNormalHints(x_dpy, win, &sizehints);
      XSetStandardProperties(x_dpy, win, name, name,
                              None, (char **)NULL, 0, &sizehints);
   }

   eglBindAPI(EGL_OPENGL_ES_API);

   ctx = eglCreateContext(egl_dpy, config, EGL_NO_CONTEXT, ctx_attribs );
   if (!ctx) {
      printf("Error: eglCreateContext failed\n");
      exit(1);
   }

   /* test eglQueryContext() */
   {
      EGLint val;
      eglQueryContext(egl_dpy, ctx, EGL_CONTEXT_CLIENT_VERSION, &val);
      assert(val == 2);
   }

   *surfRet = eglCreateWindowSurface(egl_dpy, config, win, NULL);
   if (!*surfRet) {
      printf("Error: eglCreateWindowSurface failed\n");
      exit(1);
   }

   /* sanity checks */
   {
      EGLint val;
      eglQuerySurface(egl_dpy, *surfRet, EGL_WIDTH, &val);
      assert(val == width);
      eglQuerySurface(egl_dpy, *surfRet, EGL_HEIGHT, &val);
      assert(val == height);
      assert(eglGetConfigAttrib(egl_dpy, config, EGL_SURFACE_TYPE, &val));
      assert(val & EGL_WINDOW_BIT);
   }

   XFree(visInfo);

   *winRet = win;
   *ctxRet = ctx;
}

void 
OpenMainWindow( char *title, int winWidth, int winHeight)
{
   char *dpyName = NULL;
   EGLint egl_major, egl_minor;
   int i;
   const char *s;


   halDisplay.mainDisplay = XOpenDisplay(dpyName);
   if (!halDisplay.mainDisplay) {
      printf("Error: couldn't open display %s\n",
         dpyName ? dpyName : getenv("DISPLAY"));
      exit -1;
   }

   halDisplay.eglDisplay = eglGetDisplay(halDisplay.mainDisplay);
   if (!halDisplay.eglDisplay) {
      printf("Error: eglGetDisplay() failed\n");
      exit -1;
   }

   if (!eglInitialize(halDisplay.eglDisplay, &egl_major, &egl_minor)) {
      printf("Error: eglInitialize() failed\n");
      exit -1;
   }

   s = eglQueryString(halDisplay.eglDisplay, EGL_VERSION);
   printf("EGL_VERSION = %s\n", s);

   s = eglQueryString(halDisplay.eglDisplay, EGL_VENDOR);
   printf("EGL_VENDOR = %s\n", s);

   s = eglQueryString(halDisplay.eglDisplay, EGL_EXTENSIONS);
   printf("EGL_EXTENSIONS = %s\n", s);

   s = eglQueryString(halDisplay.eglDisplay, EGL_CLIENT_APIS);
   printf("EGL_CLIENT_APIS = %s\n", s);

   make_x_window(halDisplay.mainDisplay, halDisplay.eglDisplay,
                 "OpenGL ES 2.x tri", 0, 0, winWidth, winHeight,
                 &halDisplay.win, &halDisplay.eglContext, &halDisplay.eglSurface);

   XMapWindow(halDisplay.mainDisplay, halDisplay.win);
   if (!eglMakeCurrent(halDisplay.eglDisplay, halDisplay.eglSurface, halDisplay.eglSurface, halDisplay.eglContext)) {
      printf("Error: eglMakeCurrent() failed\n");
      exit -1;
   }

   glClearColor(0.4, 0.4, 0.4, 0.0);
   create_shaders();

}  


// new window size
static void
reshape(int width, int height)
{
   glViewport(0, 0, (GLint) width, (GLint) height);
}


bool
InitWindow( int /*xPos*/, int /*yPos*/, int /*xSize*/, int /*ySize*/ )
{
   const int winWidth = 500, winHeight = 300;
    OpenMainWindow( "World Foundry", winWidth, winHeight);

    /* Set initial projection/viewing transformation.
     * We can't be sure we'll get a ConfigureNotify event when the window
     * first appears.
     */
    reshape(winWidth, winHeight);
    return true;
}

void
DestroyWindow(void)
{
   eglDestroyContext(halDisplay.eglDisplay, halDisplay.eglContext);
   eglDestroySurface(halDisplay.eglDisplay, halDisplay.eglSurface);
   eglTerminate(halDisplay.eglDisplay);

   XDestroyWindow(halDisplay.mainDisplay, halDisplay.win);
   XCloseDisplay(halDisplay.mainDisplay);

   exit (0);
}

//===============================================================================

