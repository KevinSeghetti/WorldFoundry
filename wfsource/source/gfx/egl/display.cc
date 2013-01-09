//==============================================================================
// gfx/egl/display.cc: display hardware abstraction class, windows openGL specific code
// Copyright ( c ) 1997,1998,1999,2000,2001,2002 World Foundry Group  
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

#include <hal/hal.h>
//#include <GL/gl.h>

#include <gfx/egl/wfprim.h>

#include <cpplib/libstrm.hp>
#include <memory/memory.hp>
#include <gfx/pixelmap.hp>
#include <gfx/rendobj3.hp>
extern bool bFullScreen;
extern int _halWindowWidth;
extern int _halWindowHeight;

extern RendererVariables globalRendererVariables;

//#       include <gl/glaux.h>

#include <math.h>

// Keep track of windows changing width and height
GLfloat windowXPos;
GLfloat windowYPos;
GLfloat windowWidth;
GLfloat windowHeight;
int wfWindowWidth = 640;
int wfWindowHeight = 480;


#if defined(__WIN__)
#include <Mmsystem.h>
#include <time.h>
#include "wgl.cc"
#endif
#if defined(__LINUX__)
#include "egl.cc" 
#include <sys/time.h>
#include <unistd.h>
#endif

extern void XEventLoop();

#if defined(USE_ORDER_TABLES)
#error order tables not supported in egl (no reason for them)
#endif									// defined(USE_ORDER_TABLES)

//==============================================================================


void
DumpMatrix(const char* title, GLfloat* mat)
{
    std::cout << title << " = " <<std::endl;
    for(int row=0;row<4;row++)
    {
        std::cout << "  ";
        for(int col=0;col<4;col++)
        {
            std::cout << mat[(col*4)+row] << " ";
        }
        std::cout << std::endl;
    }

    for(int index=0;index<16;index++)
    {
        std::cout << mat[index] << " ";
    }
    std::cout << std::endl;
}


//==============================================================================
// local copy of gluPerspective


#define __glPi 3.14159265358979323846

static void wf__gluMakeIdentityd(GLfloat m[16])
{
    m[0+4*0] = 1; m[0+4*1] = 0; m[0+4*2] = 0; m[0+4*3] = 0;
    m[1+4*0] = 0; m[1+4*1] = 1; m[1+4*2] = 0; m[1+4*3] = 0;
    m[2+4*0] = 0; m[2+4*1] = 0; m[2+4*2] = 1; m[2+4*3] = 0;
    m[3+4*0] = 0; m[3+4*1] = 0; m[3+4*2] = 0; m[3+4*3] = 1;
}

void 
wfgluPerspective(GLfloat m[4][4],GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar)
{
    float sine, cotangent, deltaZ;
    float radians = fovy / 2 * __glPi / 180;

    deltaZ = zFar - zNear;
    sine = sin(radians);
    if ((deltaZ == 0) || (sine == 0) || (aspect == 0)) {
	return;
    }
    cotangent = cos(radians) / sine;

    wf__gluMakeIdentityd(&m[0][0]);
    m[0][0] = cotangent / aspect;
    m[1][1] = cotangent;
    m[2][2] = -(zFar + zNear) / deltaZ;
    m[2][3] = -1;
    m[3][2] = -2 * zNear * zFar / deltaZ;
    m[3][3] = 0;
}

//===============================================================================

Display::Display(int orderTableSize, int xPos, int yPos, int xSize, int ySize, Memory& memory,bool /*interlace*/) :
_drawPage(0),
_xPos(xPos),
_yPos(yPos),
_xSize(xSize),
_ySize(ySize),
_memory(memory)
{

    _memory.Validate();
    if(!InitWindow(xPos, yPos, _halWindowWidth, _halWindowHeight ))
    {
        printf("Display::Display:doInit Failed!\n");
        sys_exit(1);
    }

    AssertGLOK();
    //glDepthRangef(-0.5f, 1000.0f);
    AssertGLOK();

    _drawPage = 0;
    ResetTime();


    float fAspect = 1.0;                           // kts I am correcting for this elsewhere, eventually in the case of PIPELINE_GL this will need to be changed

    //GLfloat m[4][4];
    wfgluPerspective(halDisplay.perspectiveMatrix,60.0f,fAspect,1.0,1000.0f);
    //glLoadMatrixf(	&m[0][0]);

    DumpMatrix("perspective matrix",&halDisplay.perspectiveMatrix[0][0]);


#if 0
#pragma message ("KTS " __FILE__ ": temp test code")
    for(int y=0; y<VRAM_HEIGHT; ++y)
    {
        for(int x=0; x<VRAM_WIDTH; ++x)
        {
#if SIXTEEN_BIT_VRAM
            vram[ y ][ x ] = 0x7fff;
#else
            vram[ y ][ x ][0] = 128;
            vram[ y ][ x ][1] = 128;
            vram[ y ][ x ][2] = 255;
            vram[ y ][ x ][3] = 255;
#endif
        }
    }
#endif

#if defined(VIDEO_MEMORY_IN_ONE_PIXELMAP)
    _videoMemory = new (HALLmalloc) PixelMap( PixelMap::MEMORY_VIDEO, VRAMWidth, VRAMHeight );
    assert( ValidPtr( _videoMemory ) );
#else

// do nothing
#endif
}

//==============================================================================

Display::~Display()
{
    Validate();
    if(_drawPage == 0)
        PageFlip();

    PageFlip();

#if defined(VIDEO_MEMORY_IN_ONE_PIXELMAP)
    delete _videoMemory;
#else

// do nothing
#endif

    DestroyWindow();
}

//============================================================================

#if defined(__LINUX__)
inline Scalar
ConvertTimeToScalar(const struct timeval&  tv)
{
    int16 whole = tv.tv_sec;
    uint16 frac;

    frac = uint16(float(tv.tv_usec)/(15.2587890625));
    assert(tv.tv_sec < USHRT_MAX);
    whole = tv.tv_sec;
    return(Scalar(whole,frac));
}
#endif

//==============================================================================

void
Display::ResetTime()                    // used to reset delta timer for PageFlip
{
    //_clockLastTime = timeGetTime();  	//clock();

#if defined(__WIN__)
    _clockLastTime = timeGetTime();                 //clock();
#elif defined(__LINUX__)
    struct timeval tv;
    gettimeofday(&tv,NULL);
    _clockLastTime = tv;                
#else
#error platform not supported
#endif
}

//============================================================================

void
Display::RenderBegin()
{
   Validate();
   AssertGLOK();
   AssertMsg( _drawPage == 0 || _drawPage == 1, "_drawPage = " << _drawPage );

   glClearColor( _backgroundColorRed, _backgroundColorGreen, _backgroundColorBlue, 1.0 );
   AssertGLOK();
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     // Clear the window with current clearing color
   AssertGLOK();
#if !defined(RENDERER_PIPELINE_GLES)
# error WTF? 
#endif
   // kts does this do anything? 
   AssertGLOK();
}

//==============================================================================

void
Display::RenderEnd()
{
}

//==============================================================================

extern bool	windowActive;		// Window windowActive Flag Set To TRUE By Default

Scalar
Display::PageFlip()
{

#if defined(__WIN__)
	MSG		msg;									// Windows Message Structure
		  
	while (PeekMessage(&msg,NULL,0,0,PM_REMOVE))	// Is There A Message Waiting?
	{
		if (msg.message==WM_QUIT)				// Have We Received A Quit Message?
		{
            printf("Display::PageFlip: gl\\display.cc calling sys_exit\n");
			KillGLWindow();									// Kill The Window
			sys_exit(1);
		}
		else									// If Not, Deal With Window Messages
		{
			TranslateMessage(&msg);				// Translate The Message
			DispatchMessage(&msg);				// Dispatch The Message
		}
	}
	{
		// Watch For ESC Key 
		if (keys[VK_ESCAPE])	// Active?  Was There A Quit Received?
		{
            printf("Display::PageFlip: gl\\display.cc calling sys_exit\n");
			KillGLWindow();									// Kill The Window
			sys_exit(1);
		}

		if (keys[VK_F1])						// Is F1 Being Pressed?
		{
			keys[VK_F1]=FALSE;					// If So Make Key FALSE
			KillGLWindow();						// Kill Our Current Window
			fullscreen=!fullscreen;				// Toggle Fullscreen / Windowed Mode
			// Recreate Our OpenGL Window
			if (!CreateGLWindow("WorldFoundry",_xPos, _yPos, _halWindowWidth, _halWindowHeight,16,fullscreen))
			{
				return 0;						// Quit If Window Was Not Created
			}
			WFInitGL();
		}
	}
#endif
#if defined(__LINUX__)
   XEventLoop(); 
#endif

    Validate();

//FntPrint("\nWorld Foundry Display: page %d\n",_drawPage);
#if 0
        // kts test code
    glColor3f( 1.0, 0.0, 1.0 );
    AssertGLOK();
    glBegin( GL_TRIANGLES );
    glVertex3f(  100.0,  100.0,  -5.0);
    glVertex3f( -100.0,  100.0,  -5.0);
    glVertex3f( -100.0, -100.0,  -5.0);
    glEnd();
    AssertGLOK();

          //glDisable( GL_TEXTURE_2D );

    glBegin( GL_TRIANGLES );
          //glColor3f( 1.0, 1.0, 0.0 );
    glColor3ub( 200, 200, 200 );

    glVertex2i( 200, 200 );
    glVertex2i( 0, 200 );
    glVertex2i( 0, -200 );
    glEnd();
    AssertGLOK();
        // end test code
#endif

//  static const GLfloat verts[3][2] = {
//     { -1, -1 },
//     {  1, -1 },
//     {  0,  1 }
//  };
//  static const GLfloat colors[3][3] = {
//     { 1, 0, 0 },
//     { 0, 1, 0 },
//     { 0, 0, 1 }
//  };
//    GLfloat mat[16], rot[16], scale[16];

    /* Set modelview/projection matrix */
//  make_z_rot_matrix(view_rotx, rot);
//  make_scale_matrix(0.25, 0.25, 0.25, scale);
//  mul_matrix(mat, rot, scale);
//
//
//std::cout << "orig matrix = " <<std::endl;
//for(int y=0;y<4;y++)
//{
//    std::cout << "  ";
//    for(int x=0;x<4;x++)
//    {
//        std::cout << mat[(y*4)+x] << " ";
//    }
//    std::cout << std::endl;
//}


    //glUniformMatrix4fv(halDisplay.u_matrix, 1, GL_FALSE, mat);

 // {
 //    glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
 //    glVertexAttribPointer(attr_color, 3, GL_FLOAT, GL_FALSE, 0, colors);
 //    glEnableVertexAttribArray(attr_pos);
 //    glEnableVertexAttribArray(attr_color);
 //
 //    glDrawArrays(GL_TRIANGLES, 0, 3);
 //
 //    glDisableVertexAttribArray(attr_pos);
 //    glDisableVertexAttribArray(attr_color);
 // }

    // switch back to our program
//  glUseProgram(halDisplay.wfProgram);
//
//     /* test setting attrib locations */
//     glBindAttribLocation(halDisplay.wfProgram, attr_pos, "pos");
//     glBindAttribLocation(halDisplay.wfProgram, attr_color, "color");
//     glLinkProgram(halDisplay.wfProgram);  /* needed to put attribs into effect */

    AssertGLOK();
    
    glFlush();
    AssertGLOK();

#if defined(__WIN__)
    // Call function to swap the buffers
	 SwapBuffers(hDC);					// Swap Buffers (Double Buffering)
#elif defined(__LINUX__)
    eglSwapBuffers(halDisplay.eglDisplay, halDisplay.current->surface);
    //glXSwapBuffers(halDisplay.eglDisplay, halDisplay.win);
    AssertGLOK();

         // glFinish();
#else
#error platform not defined
#endif


//	if(!wglMakeCurrent(hardwaredevicecontext,NULL))
//	{
//		assert(0);
//	}
//	wglDeleteContext(hRC);
//	hRC = 0;

    // now calc how long it has been since last frame
#if defined(__WIN__)
    unsigned long now = timeGetTime();                  //clock();
    unsigned long delta = now - _clockLastTime;
    _clockLastTime = now;

    AssertMsg(delta/CLOCKS_PER_SEC < 65536, 
              "delta/CLOCKS_PER_SEC = " << delta/CLOCKS_PER_SEC << ", delta = " << delta << ", CLOCKS_PER_SEC = " << CLOCKS_PER_SEC);
    Scalar whole(short(delta/CLOCKS_PER_SEC),0);
    Scalar frac(short(delta%CLOCKS_PER_SEC),0);
    frac /= SCALAR_CONSTANT(CLOCKS_PER_SEC);
    return(whole+frac);
#elif defined(__LINUX__)
    struct timeval tv;
    gettimeofday(&tv,NULL);

    struct timeval deltatime;
    deltatime.tv_usec = tv.tv_usec - _clockLastTime.tv_usec;
    deltatime.tv_sec = tv.tv_sec - _clockLastTime.tv_sec;
    assert(deltatime.tv_sec < 5);               // if more than 5 seconds something is really wrong
    int tempCounter = 0;
    while(deltatime.tv_usec < 0)
    {
        deltatime.tv_usec += 1000000;
        deltatime.tv_sec--;
        tempCounter++;
    }

    assert(tempCounter < 5);

    Scalar delta = ConvertTimeToScalar(deltatime);


    if(delta > SCALAR_CONSTANT(1.0/5.0))            // if delta less than 1/5 of a second, prop it up
    {
        std::cout << "delta too large: " << delta << std::endl;
        std::cout << "timeofday:" << tv.tv_sec << ":" << tv.tv_usec << std::endl;
        std::cout << "lasttimeofday: " << _clockLastTime.tv_sec << ":" << _clockLastTime.tv_usec << std::endl;
        std::cout << "deltatime:" << deltatime.tv_sec << ":" << deltatime.tv_usec << std::endl;
        std::cout << "delta: " << delta << std::endl;
        delta = SCALAR_CONSTANT(1.0/5.0);
    }

    // don't allow framerate to exceed 1200 fps ;-)
    if(delta < Scalar(SCALAR_CONSTANT(1.0/1200)))
    {
        delta = Scalar(SCALAR_CONSTANT(1.0/1200));
    }

    _clockLastTime = tv;
    return(delta);
#else
#error platform not defined
#endif

}

//===============================================================================
#pragma message ("KTS " __FILE__ ": major kludge to get projection matrix. Update our matrix class to handle 4x4")


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

//==============================================================================

void
LoadGLMatrixFromMatrix34(const Matrix34& matrix)
{
    AssertGLOK();
    //std::cout << "LoadGLMatrix: " << matrix << std::endl;
    static GLfloat mat[16];

    mat[(0*4)+0] = matrix[0][0].AsFloat();
    mat[(0*4)+1] = matrix[0][1].AsFloat();
    mat[(0*4)+2] = matrix[0][2].AsFloat();
    mat[(0*4)+3] = 0;

    mat[(1*4)+0] = matrix[1][0].AsFloat();
    mat[(1*4)+1] = matrix[1][1].AsFloat();
    mat[(1*4)+2] = matrix[1][2].AsFloat();
    mat[(1*4)+3] = 0;

    mat[(2*4)+0] = matrix[2][0].AsFloat();
    mat[(2*4)+1] = matrix[2][1].AsFloat();
    mat[(2*4)+2] = matrix[2][2].AsFloat();
    mat[(2*4)+3] = 0;

    mat[(3*4)+0] = matrix[3][0].AsFloat();
    mat[(3*4)+1] = matrix[3][1].AsFloat();
    mat[(3*4)+2] = matrix[3][2].AsFloat();
    mat[(3*4)+3] = 1.0;

    //std::cout << "LoadGLMatrixFrommatrix34: matrix = " << matrix << std::endl;

    //DumpMatrix("translated matrix",mat);
    glUniformMatrix4fv(halDisplay.u_matrix, 1, GL_FALSE, mat);
    AssertGLOK();


    // multiply in projection matrix
    GLfloat temp[16];

    memcpy(&temp[0],&halDisplay.perspectiveMatrix[0][0],sizeof(GLfloat)*16);
    multiply(temp,mat);
    //DumpMatrix("final matrix",temp);

    //glViewport(0, 0, (GLint) width, (GLint) height);
    glUniformMatrix4fv(halDisplay.u_matrix, 1, GL_FALSE, temp);
    AssertGLOK();
}

//==============================================================================

#include <X11/keysym.h>

extern void
_HALSetJoystickButtons(joystickButtonsF joystickButtons);

//==============================================================================
// kts FIX: quick hack to get onto linux, relace with a key table

static joystickButtonsF _joystickButtons = 0;

static 
void ProcessXEvents(XEvent event)
{
    KeySym key;

    switch(event.type)
    {
        case ConfigureNotify: 
            {
                halDisplay.current->native.width = event.xconfigure.width;
                halDisplay.current->native.height = event.xconfigure.height;
                WindowReshape(halDisplay.current->native.width, halDisplay.current->native.height);
//
//
//              /* this approach preserves a 1:1 viewport aspect ratio */
//              int vX, vY, vW, vH;
//              int eW = event.xconfigure.width, eH = event.xconfigure.height;
//              if(eW >= eH)
//              {
//                  vX = 0;
//                  vY = (eH - eW) >> 1;
//                  vW = vH = eW;
//              }
//              else
//              {
//                  vX = (eW - eH) >> 1;
//                  vY = 0;
//                  vW = vH = eH;
//              }
//              glViewport(vX, vY, vW, vH);
//              AssertGLOK();
            }
            break;

        case KeyPress:
            // printf("key %x pressed\n",key);

            // world foundry key handling
            key = XLookupKeysym(&event.xkey, 0);

            switch(key)
            {
                case(XK_KP_4):
                case(XK_Left):
                case(XK_KP_Left):
                case(XK_j):
                case(XK_J):
                    _joystickButtons |= EJ_BUTTONF_LEFT;
                    break;
                case(XK_KP_6):
                case(XK_Right):
                case(XK_KP_Right):
                case(XK_L):
                case(XK_l):
                    _joystickButtons |= EJ_BUTTONF_RIGHT;
                    break;
                case(XK_KP_8):
                case(XK_Up):
                case(XK_KP_Up):
                case(XK_i):
                case(XK_I):
                    _joystickButtons |= EJ_BUTTONF_UP;
                    break;
                case(XK_KP_2):
                case(XK_Down):
                case(XK_KP_Down):
                case(XK_k):
                case(XK_K):
                    _joystickButtons |= EJ_BUTTONF_DOWN;
                    break;
                case XK_KP_Insert:
                case XK_1:
                    _joystickButtons |= EJ_BUTTONF_A;
                    break;
                case XK_KP_Delete:
                case XK_2:
                    _joystickButtons |= EJ_BUTTONF_B;
                    break;
                case XK_KP_Enter:
                case XK_3:
                    _joystickButtons |= EJ_BUTTONF_C;
                    break;
                case(XK_KP_Add):
                case XK_4:
                    _joystickButtons |= EJ_BUTTONF_D;
                    break;
                case(XK_KP_Subtract):
                case XK_5:
                    _joystickButtons |= EJ_BUTTONF_E;
                    break;
                case(XK_KP_Multiply):
                case XK_6:
                    _joystickButtons |= EJ_BUTTONF_F;
                    break;
                case(XK_KP_Divide):
                case(XK_7):
                    _joystickButtons |= EJ_BUTTONF_G;
                    break;
                case(XK_KP_7):
                case(XK_8):
                    _joystickButtons |= EJ_BUTTONF_H;
                    break;
                case(XK_KP_9):
                case XK_9:
                    _joystickButtons |= EJ_BUTTONF_I;
                    break;
                case(XK_KP_3):
                case XK_0:
                    _joystickButtons |= EJ_BUTTONF_J;
                    break;
                case(XK_KP_1):
                case XK_exclam:
                    _joystickButtons |= EJ_BUTTONF_K;
                    break;
                case XK_Escape:
                    sys_exit(0);
//                 case XK_Escape:
//                     _joystickButtons |= 0x80000000;
                    break;
                default: 
                    printf("unknown key %x pressed, XK_KP_Enter = %x\n",key,XK_KP_Enter);
                    break;
            }
            break;

        case KeyRelease:
            // printf("key %x released\n",key);
            key = XLookupKeysym(&event.xkey, 0);
            switch(key)
            {
                case(XK_KP_4):
                case(XK_Left):
                case(XK_KP_Left):
                case(XK_J):
                case(XK_j):
                    _joystickButtons &= ~EJ_BUTTONF_LEFT;
                    break;
                case(XK_KP_6):
                case(XK_Right):
                case(XK_KP_Right):
                case(XK_L):
                case(XK_l):
                    _joystickButtons &= ~EJ_BUTTONF_RIGHT;
                    break;
                case(XK_KP_8):
                case(XK_Up):
                case(XK_KP_Up):
                case(XK_i):
                case(XK_I):
                    _joystickButtons &= ~EJ_BUTTONF_UP;
                    break;
                case(XK_KP_2):
                case(XK_Down):
                case(XK_KP_Down):
                case(XK_K):
                case(XK_k):
                    _joystickButtons &= ~EJ_BUTTONF_DOWN;
                    break;
                case XK_KP_Insert:
                case XK_1:
                    _joystickButtons &= ~EJ_BUTTONF_A;
                    break;
                case XK_KP_Delete:
                case XK_2:
                    _joystickButtons &= ~EJ_BUTTONF_B;
                    break;
                case XK_KP_Enter:
                case XK_3:
                    _joystickButtons &= ~EJ_BUTTONF_C;
                    break;
                case(XK_KP_Add):
                case XK_4:
                    _joystickButtons &= ~EJ_BUTTONF_D;
                    break;
                case(XK_KP_Subtract):
                case XK_5:
                    _joystickButtons &= ~EJ_BUTTONF_E;
                    break;
                case(XK_KP_Multiply):
                case XK_6:
                    _joystickButtons &= ~EJ_BUTTONF_F;
                    break;
                case(XK_KP_Divide):
                case XK_7:
                    _joystickButtons &= ~EJ_BUTTONF_G;
                    break;
                case(XK_KP_7):
                case XK_8:
                    _joystickButtons &= ~EJ_BUTTONF_H;
                    break;
                case(XK_KP_9):
                case XK_9:
                    _joystickButtons &= ~EJ_BUTTONF_I;
                    break;
                case(XK_KP_3):
                case XK_0:
                    _joystickButtons &= ~EJ_BUTTONF_J;
                    break;
                case(XK_KP_1):
                case XK_exclam:
                    _joystickButtons &= ~EJ_BUTTONF_K;
                    break;
//                case XK_Escape:
//                    _joystickButtons &= ~0x8000000;
                    break;
                default: 
                    printf("unknown key %x released\n",key);
                    break;
            }
            break;


        case ButtonPressMask:
            printf("You pressed button %d\n", event.xbutton.button);
            break;

       case FocusIn :
          //SetX11AutoRepeat(0);
          break;
       case FocusOut :
          //SetX11AutoRepeat(1);
          break;

          // kts this doesn't seem to work (at least it doesn't send me a message when the window is being destroyed
       case DestroyNotify:
          printf("destroynotify!!\n");
          //SetX11AutoRepeat(1);
          break;

        default:
            break;
    }

     //printf("pxevent: _joystickButtons = %x\n", _joystickButtons);
    _HALSetJoystickButtons(_joystickButtons);
}

//==============================================================================

void
XEventLoop()
{
    struct eglut_window *win = halDisplay.current;

    XEvent xev;
    int num_events;
  
    num_events = XPending(halDisplay.native_dpy);
    while((num_events != 0))
    {
        num_events--;
        XNextEvent(halDisplay.native_dpy, &xev);
        ProcessXEvents(xev);
    }
  
    // printf("_joystickButtons = %x\n", _joystickButtons);
}

//==============================================================================

