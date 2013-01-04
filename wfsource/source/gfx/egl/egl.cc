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
#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES

#include <sys/time.h>
#include <unistd.h>
//#include <EGL/eglext.h>
#include <stdarg.h>

#include "EGL/egl.h"
#include "EGL/eglext.h"
#include "EGL/egl.h"

//===============================================================================

#ifndef _EGLUTINT_H_
#define _EGLUTINT_H_

struct eglut_window {
   EGLConfig config;
   EGLContext context;

   /* initialized by native display */
   struct {
      union {
         EGLNativeWindowType window;
         EGLNativePixmapType pixmap;
         EGLSurface surface; /* pbuffer or screen surface */
      } u;
      int width, height;
   } native;

   EGLSurface surface;
};

#endif /* _EGLUTINT_H_ */

//==============================================================================

HalDisplay halDisplay;

//===============================================================================

void
_eglutNativeInitWindow(struct eglut_window *win, const char *title,
                       int x, int y, int w, int h)
{
   XVisualInfo *visInfo, visTemplate;
   int num_visuals;
   Window root, xwin;
   XSetWindowAttributes attr;
   unsigned long mask;
   EGLint vid;

   if (!eglGetConfigAttrib(halDisplay.eglDisplay,
            win->config, EGL_NATIVE_VISUAL_ID, &vid))
      Fail("failed to get visual id");

   /* The X window visual must match the EGL config */
   visTemplate.visualid = vid;
   visInfo = XGetVisualInfo(halDisplay.native_dpy,
         VisualIDMask, &visTemplate, &num_visuals);
   if (!visInfo)
      Fail("failed to get an visual of id 0x" << std::hex << vid);

   root = RootWindow(halDisplay.native_dpy, DefaultScreen(halDisplay.native_dpy));

   /* window attributes */
   attr.background_pixel = 0;
   attr.border_pixel = 0;
   attr.colormap = XCreateColormap(halDisplay.native_dpy,
         root, visInfo->visual, AllocNone);
   attr.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask | KeyReleaseMask;
   mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

   xwin = XCreateWindow(halDisplay.native_dpy, root, x, y, w, h,
         0, visInfo->depth, InputOutput, visInfo->visual, mask, &attr);
   if (!xwin)
      Fail("failed to create a window");

   XFree(visInfo);

   /* set hints and properties */
   {
      XSizeHints sizehints;
      sizehints.x = x;
      sizehints.y = y;
      sizehints.width  = w;
      sizehints.height = h;
      sizehints.flags = USSize | USPosition;
      XSetNormalHints(halDisplay.native_dpy, xwin, &sizehints);
      XSetStandardProperties(halDisplay.native_dpy, xwin,
            title, title, None, (char **) NULL, 0, &sizehints);
   }

   XMapWindow(halDisplay.native_dpy, xwin);

   win->native.u.window = xwin;
   win->native.width = w;
   win->native.height = h;
}

//===============================================================================

static struct eglut_window *
_eglutCreateWindow(const char *title, int x, int y, int w, int h)
{
   struct eglut_window *win = reinterpret_cast<eglut_window*>(calloc(1, sizeof(*win)));
   if (!win)
      Fail("failed to allocate window");

   EGLConfig config;
   EGLint num_configs;

   EGLint config_attribs[] = 
   {
      EGL_RED_SIZE, 1,
      EGL_GREEN_SIZE,1,
      EGL_BLUE_SIZE, 1,
      EGL_DEPTH_SIZE,1,
      EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
      EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
      EGL_NONE
   };

   if (!eglChooseConfig(halDisplay.eglDisplay,
            config_attribs, &win->config, 1,  &num_configs) || !num_configs)
      Fail("failed to choose a config");

   EGLint context_attribs[] = 
   {
      EGL_CONTEXT_CLIENT_VERSION, 2,
      EGL_NONE
   };

   eglBindAPI(EGL_OPENGL_ES_API);
   win->context = eglCreateContext(halDisplay.eglDisplay,
         win->config, EGL_NO_CONTEXT, context_attribs);
   if (!win->context)
      Fail("failed to create context");

   _eglutNativeInitWindow(win, title, x, y, w, h);
   win->surface = eglCreateWindowSurface(halDisplay.eglDisplay,
         win->config, win->native.u.window, NULL);
   if (win->surface == EGL_NO_SURFACE)
      Fail("failed to create surface");

   return win;
}

//===============================================================================

void
LinkShaderProgram(GLuint program)
{
   GLint stat;
   glLinkProgram(program);

   glGetProgramiv(program, GL_LINK_STATUS, &stat);
   if (!stat) {
      char log[1000];
      GLsizei len;
      glGetProgramInfoLog(program, 1000, &len, log);
      printf("Error: linking:\n%s\n", log);
      exit(1);
   }
}

//===============================================================================

static void
create_shaders(void)
{
   static const char *vertShaderText =
      "uniform mat4 modelviewProjection;\n"
      "attribute vec4 pos;\n"
      "attribute vec4 color;\n"
      "attribute vec2 a_texCoord;\n"
      "varying vec2 v_texCoord;\n"
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
      "   v_texCoord = a_texCoord;\n"
      "}\n";

   static const char *fragShaderText =
      "uniform bool u_textured;                            \n"
      "uniform sampler2D s_texture;                        \n"
      "varying vec4 v_color;\n"
      "varying vec2 v_texCoord;\n"
      "void main() {\n"
      "   if(u_textured)\n"
      "   {\n"
      "       gl_FragColor = texture2D(s_texture, v_texCoord);\n"
      "   }\n"
      "   else\n"
      "   {\n"
      "       gl_FragColor =  v_color;\n"
      "   }\n"
      "}\n";

   GLuint fragShader, vertShader;
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

   halDisplay.wfProgram = glCreateProgram();
   std::cout << "wfProgram = " << halDisplay.wfProgram << std::endl;
   glAttachShader(halDisplay.wfProgram, fragShader);
   glAttachShader(halDisplay.wfProgram, vertShader);
   LinkShaderProgram(halDisplay.wfProgram);

   glUseProgram(halDisplay.wfProgram);

   halDisplay.attr_pos = 0;
   halDisplay.attr_color = 1;
   halDisplay.a_texCoord = 2;

   if (1) {
      /* test setting attrib locations */
      glBindAttribLocation(halDisplay.wfProgram, halDisplay.attr_pos, "pos");
      glBindAttribLocation(halDisplay.wfProgram, halDisplay.attr_color, "color");
      glBindAttribLocation(halDisplay.wfProgram, halDisplay.a_texCoord, "a_texCoord");
      LinkShaderProgram(halDisplay.wfProgram); /* needed to put attribs into effect */
   }
   else {
      /* test automatic attrib locations */
      halDisplay.attr_pos = glGetAttribLocation(halDisplay.wfProgram, "pos");
      halDisplay.attr_color = glGetAttribLocation(halDisplay.wfProgram, "color");
   }

   glGetProgramiv(halDisplay.wfProgram, GL_ACTIVE_ATTRIBUTES,&stat);
   std::cout << "program stat: active attributes = " << stat << std::endl;
   glGetProgramiv(halDisplay.wfProgram, GL_ACTIVE_UNIFORMS,&stat);
   std::cout << "program stat: active uniforms = " << stat << std::endl;
   glGetProgramiv(halDisplay.wfProgram, GL_VALIDATE_STATUS,&stat);
   std::cout << "program stat: validate status = " << stat << std::endl;

   halDisplay.u_matrix = glGetUniformLocation(halDisplay.wfProgram, "modelviewProjection");
   RangeCheck(0,halDisplay.u_matrix,128);

   // Get the sampler locations
   halDisplay.s_texture  = glGetUniformLocation(halDisplay.wfProgram,"s_texture");
   RangeCheck(65536,halDisplay.s_texture,65536+128);

   // get the bool location
   halDisplay.u_textured  = glGetUniformLocation(halDisplay.wfProgram,"u_textured");
   assertNe(0,halDisplay.u_textured);

   printf("Uniform modelviewProjection at %d\n", halDisplay.u_matrix);
   printf("Attrib pos at %d\n", halDisplay.attr_pos);
   printf("Attrib color at %d\n", halDisplay.attr_color);
   printf("s_texture at %d\n", halDisplay.s_texture);
   printf("u_textured at %d\n", halDisplay.u_textured);
}

bool
InitWindow( int /*xPos*/, int /*yPos*/, int /*xSize*/, int /*ySize*/ )
{
   const int winWidth = 300, winHeight = 300;
   const char* title = "World Foundry";

   halDisplay.native_dpy = XOpenDisplay(NULL);
   if (!halDisplay.native_dpy)
      Fail("failed to initialize native display");

   halDisplay.eglDisplay = eglGetDisplay(halDisplay.native_dpy);
   if (!halDisplay.eglDisplay) {
      printf("Error: couldn't open display %s\n",
         getenv("DISPLAY"));
      exit -1;
   }

   if (!eglInitialize(halDisplay.eglDisplay, &halDisplay.major, &halDisplay.minor))
      Fail("failed to initialize EGL display");

   printf("EGL_VERSION = %s\n", eglQueryString(halDisplay.eglDisplay, EGL_VERSION));
   printf("EGL_VENDOR = %s\n", eglQueryString(halDisplay.eglDisplay, EGL_VENDOR));
   printf("EGL_EXTENSIONS = %s\n",
         eglQueryString(halDisplay.eglDisplay, EGL_EXTENSIONS));
   printf("EGL_CLIENT_APIS = %s\n",
         eglQueryString(halDisplay.eglDisplay, EGL_CLIENT_APIS));


   struct eglut_window *win;

   win = _eglutCreateWindow(title, 0, 0,
         winWidth, winHeight);

   if (!eglMakeCurrent(halDisplay.eglDisplay, win->surface, win->surface, win->context))
      Fail("failed to make window current");
   halDisplay.current = win;




   glEnable(GL_CULL_FACE);
   glEnable(GL_DEPTH_TEST);


   create_shaders();       // wf program

    return true;
}


void
DestroyWindow(void)
{
   if (halDisplay.current)
   {

      struct eglut_window *window = halDisplay.current;

      XDestroyWindow(halDisplay.native_dpy, halDisplay.current->native.u.window);
      eglDestroyContext(halDisplay.eglDisplay, halDisplay.current->context);
   }
   eglTerminate(halDisplay.eglDisplay);
   XCloseDisplay(halDisplay.native_dpy);
   exit (0);
}

//===============================================================================

static GLfloat proj[16];

/* new window size or exposure */
static void
WindowReshape(int width, int height)
{              
   GLfloat ar, m[16] = {
      1.0, 0.0, 0.0, 0.0,
      0.0, 1.0, 0.0, 0.0,
      0.0, 0.0, 0.1, 0.0,
      0.0, 0.0, 0.0, 1.0,
   };
      
   if (width < height)
      ar = width;
   else
      ar = height;

   m[0] = 0.1 * ar / width;
   m[5] = 0.1 * ar / height;
   memcpy(proj, m, sizeof proj);
   glViewport(0, 0, (GLint) width, (GLint) height);
}

//===============================================================================

