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

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>


//===============================================================================

//===============================================================================
//===============================================================================
/*
 * Copyright (C) 1999-2001  Brian Paul   All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * Ported to GLES2.
 * Kristian Høgsberg <krh@bitplanet.net>
 * May 3, 2010
 */

/*
 * Command line options:
 *    -info      print GL implementation information
 *
 */


#ifndef EGLUT_H
#define EGLUT_H

/* used by eglutInitAPIMask */
enum {
   EGLUT_OPENGL_BIT     = 0x1,
   EGLUT_OPENGL_ES1_BIT = 0x2,
   EGLUT_OPENGL_ES2_BIT = 0x4,
   EGLUT_OPENVG_BIT     = 0x8
};

/* used by EGLUTspecialCB */
enum {
   /* function keys */
   EGLUT_KEY_F1,
   EGLUT_KEY_F2,
   EGLUT_KEY_F3,
   EGLUT_KEY_F4,
   EGLUT_KEY_F5,
   EGLUT_KEY_F6,
   EGLUT_KEY_F7,
   EGLUT_KEY_F8,
   EGLUT_KEY_F9,
   EGLUT_KEY_F10,
   EGLUT_KEY_F11,
   EGLUT_KEY_F12,

   /* directional keys */
   EGLUT_KEY_LEFT,
   EGLUT_KEY_UP,
   EGLUT_KEY_RIGHT,
   EGLUT_KEY_DOWN,
};

/* used by eglutGet */
enum {
   EGLUT_ELAPSED_TIME
};

typedef void (*EGLUTspecialCB)(int);

void eglutInitAPIMask(int mask);
void eglutInitWindowSize(int width, int height);
void eglutInit(int argc, char **argv);

int eglutGet(int state);

void eglutMainLoop(void);

int eglutCreateWindow(const char *title);
void eglutDestroyWindow(int win);

int eglutGetWindowWidth(void);
int eglutGetWindowHeight(void);

void eglutSpecialFunc(EGLUTspecialCB func);

#endif /* EGLUT_H */

//===============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>

#include "EGL/egl.h"
#include "EGL/eglext.h"

//===============================================================================
#ifndef _EGLUTINT_H_
#define _EGLUTINT_H_

#include "EGL/egl.h"
#include "eglut.h"

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

   int index;
};

struct eglut_state {
   int api_mask;
   int window_width, window_height;
   const char *display_name;
   int verbose;
   int init_time;

   int num_windows;

   /* initialized by native display */
   EGLNativeDisplayType native_dpy;
   EGLint surface_type;

   EGLDisplay dpy;
   EGLint major, minor;

   struct eglut_window *current;
};

extern struct eglut_state *_eglut;

void
_eglutFatal(char *format, ...);

int
_eglutNow(void);

void
_eglutNativeInitDisplay(void);

void
_eglutNativeFiniDisplay(void);

void
_eglutNativeInitWindow(struct eglut_window *win, const char *title,
                       int x, int y, int w, int h);

void
_eglutNativeFiniWindow(struct eglut_window *win);

void
_eglutNativeEventLoop(void);

#endif /* _EGLUTINT_H_ */



//===============================================================================

//===============================================================================


static struct eglut_state _eglut_state;
// = {
//   .api_mask = EGLUT_OPENGL_ES1_BIT,
//   .window_width = 300,
//   .window_height = 300,
//   .verbose = 0,
//   .num_windows = 0,
//};

struct eglut_state *_eglut = &_eglut_state;

void
_eglutFatal(char *format, ...)
{
  va_list args;

  va_start(args, format);

  fprintf(stderr, "EGLUT: ");
  vfprintf(stderr, format, args);
  va_end(args);
  putc('\n', stderr);

  exit(1);
}

/* return current time (in milliseconds) */
int
_eglutNow(void)
{
   struct timeval tv;
#ifdef __VMS
   (void) gettimeofday(&tv, NULL );
#else
   struct timezone tz;
   (void) gettimeofday(&tv, &tz);
#endif
   return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static void
_eglutDestroyWindow(struct eglut_window *win)
{
   if (_eglut->surface_type != EGL_PBUFFER_BIT &&
       _eglut->surface_type != EGL_SCREEN_BIT_MESA)
      eglDestroySurface(_eglut->dpy, win->surface);

   _eglutNativeFiniWindow(win);

   eglDestroyContext(_eglut->dpy, win->context);
}

static EGLConfig
_eglutChooseConfig(void)
{
   EGLConfig config;
   EGLint config_attribs[32];
   EGLint renderable_type, num_configs, i;

   i = 0;
   config_attribs[i++] = EGL_RED_SIZE;
   config_attribs[i++] = 1;
   config_attribs[i++] = EGL_GREEN_SIZE;
   config_attribs[i++] = 1;
   config_attribs[i++] = EGL_BLUE_SIZE;
   config_attribs[i++] = 1;
   config_attribs[i++] = EGL_DEPTH_SIZE;
   config_attribs[i++] = 1;

   config_attribs[i++] = EGL_SURFACE_TYPE;
   config_attribs[i++] = _eglut->surface_type;

   config_attribs[i++] = EGL_RENDERABLE_TYPE;
   renderable_type = 0x0;
   if (_eglut->api_mask & EGLUT_OPENGL_BIT)
      renderable_type |= EGL_OPENGL_BIT;
   if (_eglut->api_mask & EGLUT_OPENGL_ES1_BIT)
      renderable_type |= EGL_OPENGL_ES_BIT;
   if (_eglut->api_mask & EGLUT_OPENGL_ES2_BIT)
      renderable_type |= EGL_OPENGL_ES2_BIT;
   if (_eglut->api_mask & EGLUT_OPENVG_BIT)
      renderable_type |= EGL_OPENVG_BIT;
   config_attribs[i++] = renderable_type;

   config_attribs[i] = EGL_NONE;

   if (!eglChooseConfig(_eglut->dpy,
            config_attribs, &config, 1, &num_configs) || !num_configs)
      _eglutFatal("failed to choose a config");

   return config;
}

static struct eglut_window *
_eglutCreateWindow(const char *title, int x, int y, int w, int h)
{
   struct eglut_window *win;
   EGLint context_attribs[4];
   EGLint api, i;

   win = reinterpret_cast<eglut_window*>(calloc(1, sizeof(*win)));
   if (!win)
      _eglutFatal("failed to allocate window");

   win->config = _eglutChooseConfig();

   i = 0;
   context_attribs[i] = EGL_NONE;

   /* multiple APIs? */

   api = EGL_OPENGL_ES_API;
   if (_eglut->api_mask & EGLUT_OPENGL_BIT) {
      api = EGL_OPENGL_API;
   }
   else if (_eglut->api_mask & EGLUT_OPENVG_BIT) {
      api = EGL_OPENVG_API;
   }
   else if (_eglut->api_mask & EGLUT_OPENGL_ES2_BIT) {
      context_attribs[i++] = EGL_CONTEXT_CLIENT_VERSION;
      context_attribs[i++] = 2;
   }

   context_attribs[i] = EGL_NONE;

   eglBindAPI(api);
   win->context = eglCreateContext(_eglut->dpy,
         win->config, EGL_NO_CONTEXT, context_attribs);
   if (!win->context)
      _eglutFatal("failed to create context");

   _eglutNativeInitWindow(win, title, x, y, w, h);
   switch (_eglut->surface_type) {
   case EGL_WINDOW_BIT:
      win->surface = eglCreateWindowSurface(_eglut->dpy,
            win->config, win->native.u.window, NULL);
      break;
   case EGL_PIXMAP_BIT:
      win->surface = eglCreatePixmapSurface(_eglut->dpy,
            win->config, win->native.u.pixmap, NULL);
      break;
   case EGL_PBUFFER_BIT:
   case EGL_SCREEN_BIT_MESA:
      win->surface = win->native.u.surface;
      break;
   default:
      break;
   }
   if (win->surface == EGL_NO_SURFACE)
      _eglutFatal("failed to create surface");

   return win;
}

void
eglutInitAPIMask(int mask)
{
   _eglut->api_mask = mask;
}

void
eglutInitWindowSize(int width, int height)
{
   _eglut->window_width = width;
   _eglut->window_height = height;
}

void
eglutInit(int argc, char **argv)
{
   int i;

   for (i = 1; i < argc; i++) {
      if (strcmp(argv[i], "-display") == 0)
         _eglut->display_name = argv[++i];
      else if (strcmp(argv[i], "-info") == 0) {
         _eglut->verbose = 1;
      }
   }

   _eglutNativeInitDisplay();
   _eglut->dpy = eglGetDisplay(_eglut->native_dpy);

   if (!eglInitialize(_eglut->dpy, &_eglut->major, &_eglut->minor))
      _eglutFatal("failed to initialize EGL display");

   _eglut->init_time = _eglutNow();

   printf("EGL_VERSION = %s\n", eglQueryString(_eglut->dpy, EGL_VERSION));
   if (_eglut->verbose) {
      printf("EGL_VENDOR = %s\n", eglQueryString(_eglut->dpy, EGL_VENDOR));
      printf("EGL_EXTENSIONS = %s\n",
            eglQueryString(_eglut->dpy, EGL_EXTENSIONS));
      printf("EGL_CLIENT_APIS = %s\n",
            eglQueryString(_eglut->dpy, EGL_CLIENT_APIS));
   }
}

int
eglutGet(int state)
{
   int val;

   switch (state) {
   case EGLUT_ELAPSED_TIME:
      val = _eglutNow() - _eglut->init_time;
      break;
   default:
      val = -1;
      break;
   }

   return val;
}

static void
_eglutFini(void)
{
   eglTerminate(_eglut->dpy);
   _eglutNativeFiniDisplay();
}

void
eglutDestroyWindow(int win)
{
   struct eglut_window *window = _eglut->current;

   if (window->index != win)
      return;

   /* XXX it causes some bug in st/egl KMS backend */
   if ( _eglut->surface_type != EGL_SCREEN_BIT_MESA)
      eglMakeCurrent(_eglut->dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

   _eglutDestroyWindow(_eglut->current);
}

static void
_eglutDefaultKeyboard(unsigned char key)
{
   if (key == 27) {
      if (_eglut->current)
         eglutDestroyWindow(_eglut->current->index);
      _eglutFini();

      exit(0);
   }
}

int
eglutCreateWindow(const char *title)
{
   struct eglut_window *win;

   win = _eglutCreateWindow(title, 0, 0,
         _eglut->window_width, _eglut->window_height);

   win->index = _eglut->num_windows++;

   if (!eglMakeCurrent(_eglut->dpy, win->surface, win->surface, win->context))
      _eglutFatal("failed to make window current");
   _eglut->current = win;

   return win->index;
}

int
eglutGetWindowWidth(void)
{
   struct eglut_window *win = _eglut->current;
   return win->native.width;
}

int
eglutGetWindowHeight(void)
{
   struct eglut_window *win = _eglut->current;
   return win->native.height;
}
//===============================================================================

void
_eglutNativeInitDisplay(void)
{
   _eglut->native_dpy = XOpenDisplay(_eglut->display_name);
   if (!_eglut->native_dpy)
      _eglutFatal("failed to initialize native display");

   _eglut->surface_type = EGL_WINDOW_BIT;
}

void
_eglutNativeFiniDisplay(void)
{
   XCloseDisplay(_eglut->native_dpy);
}

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

   if (!eglGetConfigAttrib(_eglut->dpy,
            win->config, EGL_NATIVE_VISUAL_ID, &vid))
      _eglutFatal("failed to get visual id");

   /* The X window visual must match the EGL config */
   visTemplate.visualid = vid;
   visInfo = XGetVisualInfo(_eglut->native_dpy,
         VisualIDMask, &visTemplate, &num_visuals);
   if (!visInfo)
      _eglutFatal("failed to get an visual of id 0x%x", vid);

   root = RootWindow(_eglut->native_dpy, DefaultScreen(_eglut->native_dpy));

   /* window attributes */
   attr.background_pixel = 0;
   attr.border_pixel = 0;
   attr.colormap = XCreateColormap(_eglut->native_dpy,
         root, visInfo->visual, AllocNone);
   attr.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask;
   mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

   xwin = XCreateWindow(_eglut->native_dpy, root, x, y, w, h,
         0, visInfo->depth, InputOutput, visInfo->visual, mask, &attr);
   if (!xwin)
      _eglutFatal("failed to create a window");

   XFree(visInfo);

   /* set hints and properties */
   {
      XSizeHints sizehints;
      sizehints.x = x;
      sizehints.y = y;
      sizehints.width  = w;
      sizehints.height = h;
      sizehints.flags = USSize | USPosition;
      XSetNormalHints(_eglut->native_dpy, xwin, &sizehints);
      XSetStandardProperties(_eglut->native_dpy, xwin,
            title, title, None, (char **) NULL, 0, &sizehints);
   }

   XMapWindow(_eglut->native_dpy, xwin);

   win->native.u.window = xwin;
   win->native.width = w;
   win->native.height = h;
}

void
_eglutNativeFiniWindow(struct eglut_window *win)
{
   XDestroyWindow(_eglut->native_dpy, win->native.u.window);
}

static int
lookup_keysym(KeySym sym)
{
   int special;

   switch (sym) {
   case XK_F1:
      special = EGLUT_KEY_F1;
      break;
   case XK_F2:
      special = EGLUT_KEY_F2;
      break;
   case XK_F3:
      special = EGLUT_KEY_F3;
      break;
   case XK_F4:
      special = EGLUT_KEY_F4;
      break;
   case XK_F5:
      special = EGLUT_KEY_F5;
      break;
   case XK_F6:
      special = EGLUT_KEY_F6;
      break;
   case XK_F7:
      special = EGLUT_KEY_F7;
      break;
   case XK_F8:
      special = EGLUT_KEY_F8;
      break;
   case XK_F9:
      special = EGLUT_KEY_F9;
      break;
   case XK_F10:
      special = EGLUT_KEY_F10;
      break;
   case XK_F11:
      special = EGLUT_KEY_F11;
      break;
   case XK_F12:
      special = EGLUT_KEY_F12;
      break;
   case XK_KP_Left:
   case XK_Left:
      special = EGLUT_KEY_LEFT;
      break;
   case XK_KP_Up:
   case XK_Up:
      special = EGLUT_KEY_UP;
      break;
   case XK_KP_Right:
   case XK_Right:
      special = EGLUT_KEY_RIGHT;
      break;
   case XK_KP_Down:
   case XK_Down:
      special = EGLUT_KEY_DOWN;
      break;
   default:
      special = -1;
      break;
   }

   return special;
}

//==============================================================================

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
/**************************************************************************
 *
 * Copyright 2008 Tungsten Graphics, Inc., Cedar Park, Texas.
 * All Rights Reserved.
 *
 **************************************************************************/


static GLint u_matrix = -1;
static GLint attr_pos = 0, attr_color = 1;

//static void
//make_z_rot_matrix(GLfloat angle, GLfloat *m)
//{
//   float c = cos(angle * M_PI / 180.0);
//   float s = sin(angle * M_PI / 180.0);
//   int i;
//   for (i = 0; i < 16; i++)
//      m[i] = 0.0;
//   m[0] = m[5] = m[10] = m[15] = 1.0;
//
//   m[0] = c;
//   m[1] = s;
//   m[4] = -s;
//   m[5] = c;
//}
//
//static void
//make_scale_matrix(GLfloat xs, GLfloat ys, GLfloat zs, GLfloat *m)
//{
//   int i;
//   for (i = 0; i < 16; i++)
//      m[i] = 0.0;
//   m[0] = xs;
//   m[5] = ys;
//   m[10] = zs;
//   m[15] = 1.0;
//}
//
//
//static void
//mul_matrix(GLfloat *prod, const GLfloat *a, const GLfloat *b)
//{
//#define A(row,col)  a[(col<<2)+row]
//#define B(row,col)  b[(col<<2)+row]
//#define P(row,col)  p[(col<<2)+row]
//   GLfloat p[16];
//   GLint i;
//   for (i = 0; i < 4; i++) {
//      const GLfloat ai0=A(i,0),  ai1=A(i,1),  ai2=A(i,2),  ai3=A(i,3);
//      P(i,0) = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0) + ai3 * B(3,0);
//      P(i,1) = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1) + ai3 * B(3,1);
//      P(i,2) = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2) + ai3 * B(3,2);
//      P(i,3) = ai0 * B(0,3) + ai1 * B(1,3) + ai2 * B(2,3) + ai3 * B(3,3);
//   }
//   memcpy(prod, p, sizeof(p));
//#undef A
//#undef B
//#undef PROD
//}

/* new window size or exposure */
static void
reshape(int width, int height)
{
   glViewport(0, 0, (GLint) width, (GLint) height);
}


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


static void
init(void)
{
   typedef void (*proc)();

#if 1 /* test code */
   proc p = eglGetProcAddress("glMapBufferOES");
   assert(p);
#endif

   glClearColor(0.4, 0.4, 0.4, 0.0);

   create_shaders();
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


static void
usage(void)
{
   printf("Usage:\n");
   printf("  -display <displayname>  set the display to run on\n");
   printf("  -info                   display OpenGL renderer info\n");
}


void 
OpenMainWindow( char *title, int winWidth, int winHeight)
{
   char *dpyName = NULL;
   GLboolean printInfo = GL_FALSE;
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

   if (printInfo) {
      printf("GL_RENDERER   = %s\n", (char *) glGetString(GL_RENDERER));
      printf("GL_VERSION    = %s\n", (char *) glGetString(GL_VERSION));
      printf("GL_VENDOR     = %s\n", (char *) glGetString(GL_VENDOR));
      printf("GL_EXTENSIONS = %s\n", (char *) glGetString(GL_EXTENSIONS));
   }

   init();

}  

int
gearstest(int argc, char *argv[]);


static void
gears_init(void);


bool
InitWindow( int /*xPos*/, int /*yPos*/, int /*xSize*/, int /*ySize*/ )
{
   const int winWidth = 500, winHeight = 300;

   _eglut_state.api_mask = EGLUT_OPENGL_ES1_BIT;
   _eglut_state.window_width = 300;
   _eglut_state.window_height = 300;
   _eglut_state.verbose = 0;
   _eglut_state.num_windows = 0;

   eglutInitWindowSize(300, 300);
   eglutInitAPIMask(EGLUT_OPENGL_ES2_BIT);
   eglutInit(0, NULL);

   eglutCreateWindow("es2gears");

   gears_init();

    //OpenMainWindow( "World Foundry", winWidth, winHeight);
    //reshape(winWidth, winHeight);
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

//===============================================================================

struct gear {
   GLfloat *vertices;
   GLuint vbo;
   int count;
};

static GLfloat view_rotx = 20.0, view_roty = 30.0, view_rotz = 0.0;
static struct gear *gear1, *gear2, *gear3;
static GLfloat angle = 0.0;
static GLuint proj_location, light_location, color_location;
static GLfloat proj[16];

static GLfloat *
vert(GLfloat *p, GLfloat x, GLfloat y, GLfloat z, GLfloat *n)
{
   p[0] = x;
   p[1] = y;
   p[2] = z;
   p[3] = n[0];
   p[4] = n[1];
   p[5] = n[2];

   return p + 6;
}

/*  Draw a gear wheel.  You'll probably want to call this function when
 *  building a display list since we do a lot of trig here.
 * 
 *  Input:  inner_radius - radius of hole at center
 *          outer_radius - radius at center of teeth
 *          width - width of gear
 *          teeth - number of teeth
 *          tooth_depth - depth of tooth
 */
static struct gear *
gear(GLfloat inner_radius, GLfloat outer_radius, GLfloat width,
     GLint teeth, GLfloat tooth_depth)
{
   GLint i;
   GLfloat r0, r1, r2;
   GLfloat da;
   GLfloat *p, *v;
   struct gear *gear;
   double s[5], c[5];
   GLfloat verts[3 * 14], normal[3];
   const int tris_per_tooth = 20;

   gear = reinterpret_cast<struct gear*>(malloc(sizeof *gear));
   if (gear == NULL)
      return NULL;

   r0 = inner_radius;
   r1 = outer_radius - tooth_depth / 2.0;
   r2 = outer_radius + tooth_depth / 2.0;

   da = 2.0 * M_PI / teeth / 4.0;

   gear->vertices = reinterpret_cast<GLfloat*>(calloc(teeth * tris_per_tooth * 3 * 6,
			   sizeof *gear->vertices));
   s[4] = 0;
   c[4] = 1;
   v = gear->vertices;
   for (i = 0; i < teeth; i++) {
      s[0] = s[4];
      c[0] = c[4];
      sincos(i * 2.0 * M_PI / teeth + da, &s[1], &c[1]);
      sincos(i * 2.0 * M_PI / teeth + da * 2, &s[2], &c[2]);
      sincos(i * 2.0 * M_PI / teeth + da * 3, &s[3], &c[3]);
      sincos(i * 2.0 * M_PI / teeth + da * 4, &s[4], &c[4]);

      normal[0] = 0.0;
      normal[1] = 0.0;
      normal[2] = 1.0;

      v = vert(v, r2 * c[1], r2 * s[1], width * 0.5, normal);

      v = vert(v, r2 * c[1], r2 * s[1], width * 0.5, normal);
      v = vert(v, r2 * c[2], r2 * s[2], width * 0.5, normal);
      v = vert(v, r1 * c[0], r1 * s[0], width * 0.5, normal);
      v = vert(v, r1 * c[3], r1 * s[3], width * 0.5, normal);
      v = vert(v, r0 * c[0], r0 * s[0], width * 0.5, normal);
      v = vert(v, r1 * c[4], r1 * s[4], width * 0.5, normal);
      v = vert(v, r0 * c[4], r0 * s[4], width * 0.5, normal);

      v = vert(v, r0 * c[4], r0 * s[4], width * 0.5, normal);
      v = vert(v, r0 * c[0], r0 * s[0], width * 0.5, normal);
      v = vert(v, r0 * c[4], r0 * s[4], -width * 0.5, normal);
      v = vert(v, r0 * c[0], r0 * s[0], -width * 0.5, normal);

      normal[0] = 0.0;
      normal[1] = 0.0;
      normal[2] = -1.0;

      v = vert(v, r0 * c[4], r0 * s[4], -width * 0.5, normal);

      v = vert(v, r0 * c[4], r0 * s[4], -width * 0.5, normal);
      v = vert(v, r1 * c[4], r1 * s[4], -width * 0.5, normal);
      v = vert(v, r0 * c[0], r0 * s[0], -width * 0.5, normal);
      v = vert(v, r1 * c[3], r1 * s[3], -width * 0.5, normal);
      v = vert(v, r1 * c[0], r1 * s[0], -width * 0.5, normal);
      v = vert(v, r2 * c[2], r2 * s[2], -width * 0.5, normal);
      v = vert(v, r2 * c[1], r2 * s[1], -width * 0.5, normal);

      v = vert(v, r1 * c[0], r1 * s[0], width * 0.5, normal);

      v = vert(v, r1 * c[0], r1 * s[0], width * 0.5, normal);
      v = vert(v, r1 * c[0], r1 * s[0], -width * 0.5, normal);
      v = vert(v, r2 * c[1], r2 * s[1], width * 0.5, normal);
      v = vert(v, r2 * c[1], r2 * s[1], -width * 0.5, normal);
      v = vert(v, r2 * c[2], r2 * s[2], width * 0.5, normal);
      v = vert(v, r2 * c[2], r2 * s[2], -width * 0.5, normal);
      v = vert(v, r1 * c[3], r1 * s[3], width * 0.5, normal);
      v = vert(v, r1 * c[3], r1 * s[3], -width * 0.5, normal);
      v = vert(v, r1 * c[4], r1 * s[4], width * 0.5, normal);
      v = vert(v, r1 * c[4], r1 * s[4], -width * 0.5, normal);

      v = vert(v, r1 * c[4], r1 * s[4], -width * 0.5, normal);
   }

   gear->count = (v - gear->vertices) / 6;

   glGenBuffers(1, &gear->vbo);
   glBindBuffer(GL_ARRAY_BUFFER, gear->vbo);
   glBufferData(GL_ARRAY_BUFFER, gear->count * 6 * 4,
		gear->vertices, GL_STATIC_DRAW);

   return gear;
}

static void
multiply(GLfloat *m, const GLfloat *n)
{
   GLfloat tmp[16];
   const GLfloat *row, *column;
   div_t d;
   int i, j;

   for (i = 0; i < 16; i++) {
      tmp[i] = 0;
      d = div(i, 4);
      row = n + d.quot * 4;
      column = m + d.rem;
      for (j = 0; j < 4; j++)
	 tmp[i] += row[j] * column[j * 4];
   }
   memcpy(m, &tmp, sizeof tmp);
}

static void
rotate(GLfloat *m, GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
   double s, c;

   sincos(angle, &s, &c);
   GLfloat r[16] = {
      x * x * (1 - c) + c,     y * x * (1 - c) + z * s, x * z * (1 - c) - y * s, 0,
      x * y * (1 - c) - z * s, y * y * (1 - c) + c,     y * z * (1 - c) + x * s, 0, 
      x * z * (1 - c) + y * s, y * z * (1 - c) - x * s, z * z * (1 - c) + c,     0,
      0, 0, 0, 1
   };

   multiply(m, r);
}

static void
translate(GLfloat *m, GLfloat x, GLfloat y, GLfloat z)
{
   GLfloat t[16] = { 1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  x, y, z, 1 };

   multiply(m, t);
}

static const GLfloat light[3] = { 1.0, 1.0, -1.0 };
	
static void
draw_gear(struct gear *gear, GLfloat *m,
	  GLfloat x, GLfloat y, GLfloat angle, const GLfloat *color)
{
   GLfloat tmp[16];

   memcpy(tmp, m, sizeof tmp);
   translate(tmp, x, y, 0);
   rotate(tmp, 2 * M_PI * angle / 360.0, 0, 0, 1);
   glUniformMatrix4fv(proj_location, 1, GL_FALSE, tmp);
   glUniform3fv(light_location, 1, light);
   glUniform4fv(color_location, 1, color);

   glBindBuffer(GL_ARRAY_BUFFER, gear->vbo);

   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
			 6 * sizeof(GLfloat), NULL);
   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
			 6 * sizeof(GLfloat), (GLfloat *) 0 + 3);
   glEnableVertexAttribArray(0);
   glEnableVertexAttribArray(1);
   glDrawArrays(GL_TRIANGLE_STRIP, 0, gear->count);
}

static void
gears_draw(void)
{
   const static GLfloat red[4] = { 0.8, 0.1, 0.0, 1.0 };
   const static GLfloat green[4] = { 0.0, 0.8, 0.2, 1.0 };
   const static GLfloat blue[4] = { 0.2, 0.2, 1.0, 1.0 };
   GLfloat m[16];


   memcpy(m, proj, sizeof m);
   rotate(m, 2 * M_PI * view_rotx / 360.0, 1, 0, 0);
   rotate(m, 2 * M_PI * view_roty / 360.0, 0, 1, 0);
   rotate(m, 2 * M_PI * view_rotz / 360.0, 0, 0, 1);

   draw_gear(gear1, m, -3.0, -2.0, angle, red);
   draw_gear(gear2, m, 3.1, -2.0, -2 * angle - 9.0, green);
   draw_gear(gear3, m, -3.1, 4.2, -2 * angle - 25.0, blue);
}

/* new window size or exposure */
static void
gears_reshape(int width, int height)
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

static void
gears_special(int special)
{
   switch (special) {
   case EGLUT_KEY_LEFT:
      view_roty += 5.0;
      break;
   case EGLUT_KEY_RIGHT:
      view_roty -= 5.0;
      break;
   case EGLUT_KEY_UP:
      view_rotx += 5.0;
      break;
   case EGLUT_KEY_DOWN:
      view_rotx -= 5.0;
      break;
   }
}

static void
gears_idle(void)
{
   static double tRot0 = -1.0;
   double dt, t = eglutGet(EGLUT_ELAPSED_TIME) / 1000.0;

   if (tRot0 < 0.0)
      tRot0 = t;
   dt = t - tRot0;
   tRot0 = t;

   /* advance rotation for next frame */
   angle += 70.0 * dt;  /* 70 degrees per second */
   if (angle > 3600.0)
      angle -= 3600.0;
}

static const char vertex_shader[] =
   "uniform mat4 proj;\n"
   "attribute vec4 position;\n"
   "attribute vec4 normal;\n"
   "varying vec3 rotated_normal;\n"
   "varying vec3 rotated_position;\n"
   "vec4 tmp;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = proj * position;\n"
   "   rotated_position = gl_Position.xyz;\n"
   "   tmp = proj * normal;\n"
   "   rotated_normal = tmp.xyz;\n"
   "}\n";

 static const char fragment_shader[] =
   //"precision mediump float;\n"
   "uniform vec4 color;\n"
   "uniform vec3 light;\n"
   "varying vec3 rotated_normal;\n"
   "varying vec3 rotated_position;\n"
   "vec3 light_direction;\n"
   "vec4 white = vec4(1.0, 1.0, 1.0, 1.0);\n"
   "void main()\n"
   "{\n"
   "   light_direction = normalize(light - rotated_position);\n"
   "   gl_FragColor = color + white * dot(light_direction, rotated_normal);\n"
   "}\n";

static void
gears_init(void)
{
   GLuint v, f, program;
   const char *p;
   char msg[512];

   glEnable(GL_CULL_FACE);
   glEnable(GL_DEPTH_TEST);

   p = vertex_shader;
   v = glCreateShader(GL_VERTEX_SHADER);
   glShaderSource(v, 1, &p, NULL);
   glCompileShader(v);
   glGetShaderInfoLog(v, sizeof msg, NULL, msg);
   printf("vertex shader info: %s\n", msg);

   p = fragment_shader;
   f = glCreateShader(GL_FRAGMENT_SHADER);
   glShaderSource(f, 1, &p, NULL);
   glCompileShader(f);
   glGetShaderInfoLog(f, sizeof msg, NULL, msg);
   printf("fragment shader info: %s\n", msg);

   program = glCreateProgram();
   glAttachShader(program, v);
   glAttachShader(program, f);
   glBindAttribLocation(program, 0, "position");
   glBindAttribLocation(program, 1, "normal");

   glLinkProgram(program);
   glGetProgramInfoLog(program, sizeof msg, NULL, msg);
   printf("info: %s\n", msg);

   glUseProgram(program);
   proj_location = glGetUniformLocation(program, "proj");
   light_location = glGetUniformLocation(program, "light");
   color_location = glGetUniformLocation(program, "color");

   /* make the gears */
   gear1 = gear(1.0, 4.0, 1.0, 20, 0.7);
   gear2 = gear(0.5, 2.0, 2.0, 10, 0.7);
   gear3 = gear(1.3, 2.0, 0.5, 10, 0.7);
}



static void
next_event(struct eglut_window *win)
{
   int redraw = 0;
   XEvent event;

   if (!XPending(_eglut->native_dpy)) {
      gears_idle();
      return;
   }

   XNextEvent(_eglut->native_dpy, &event);

   switch (event.type) {
   case Expose:
      redraw = 1;
      break;
   case ConfigureNotify:
      win->native.width = event.xconfigure.width;
      win->native.height = event.xconfigure.height;
      gears_reshape(win->native.width, win->native.height);
      break;
   case KeyPress:
      {
         char buffer[1];
         KeySym sym;
         int r;

         r = XLookupString(&event.xkey,
               buffer, sizeof(buffer), &sym, NULL);
         if (r) {
            _eglutDefaultKeyboard(buffer[0]);
         }
         else if (!r) {
            r = lookup_keysym(sym);
            if (r >= 0)
               gears_special(r);
         }
      }
      redraw = 1;
      break;
   default:
      ; /*no-op*/
   }
}



