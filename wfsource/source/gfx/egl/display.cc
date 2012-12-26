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

//==============================================================================

Display::Display(int orderTableSize, int xPos, int yPos, int xSize, int ySize, Memory& memory,bool /*interlace*/) :
_drawPage(0),
#if defined(USE_ORDER_TABLES)
_constructionOrderTableIndex(0),
_renderOrderTableIndex(1),
#endif
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

//  while (1) {
//      event_loop(halDisplay.mainDisplay, halDisplay.win, halDisplay.eglDisplay, halDisplay.eglSurface);
//  }

//    AssertGLOK();
//
//   WFInitGL();
//
//    assert(orderTableSize > 0);
//#if defined(USE_ORDER_TABLES)
//    for(int index=0;index<ORDER_TABLES;index++)
//    {
//        _orderTable[index] = new (_memory) OrderTable(orderTableSize,_memory);
//        assert(ValidPtr(_orderTable[index]));
//    }
//#endif

    // set up GL 
   //glLightModelf(GL_LIGHT_MODEL_TWO_SIDE,1);
   //glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    _drawPage = 0;
    ResetTime();

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

#if defined(USE_ORDER_TABLES)
    for(int index=ORDER_TABLES-1;index>= 0;index--)
        _memory.Free(_orderTable[index],sizeof(OrderTable));
#endif

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
#if defined(RENDERER_PIPELINE_GL)
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glEnable(GL_LIGHT1);
   glEnable(GL_LIGHT2);
   glEnable(GL_NORMALIZE);
   glEnable(GL_FOG);

   GLfloat lightWhite[] = {
       1.0, 1.0, 1.0, 1.0
   };


   GLfloat lightBlack[] = {
       0.0, 0.0, 0.0, 0.0
   };
   glMaterialfv(GL_FRONT,GL_AMBIENT,lightWhite);
   glMaterialfv(GL_FRONT,GL_DIFFUSE,lightWhite);
   glMaterialfv(GL_FRONT,GL_SPECULAR,lightBlack);
   AssertGLOK();
#elif defined(RENDERER_PIPELINE_GLES)
// glEnable(GL_LIGHTING);
// glEnable(GL_LIGHT0);
// glEnable(GL_LIGHT1);
// glEnable(GL_LIGHT2);
// glEnable(GL_NORMALIZE);
// glEnable(GL_FOG);
//
// GLfloat lightWhite[] = {
//     1.0, 1.0, 1.0, 1.0
// };
//
//
// GLfloat lightBlack[] = {
//     0.0, 0.0, 0.0, 0.0
// };
// glMaterialfv(GL_FRONT,GL_AMBIENT,lightWhite);
// glMaterialfv(GL_FRONT,GL_DIFFUSE,lightWhite);
// glMaterialfv(GL_FRONT,GL_SPECULAR,lightBlack);
// AssertGLOK();

#else
   glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   glDisable(GL_LIGHTING);
#endif

   AssertGLOK();
   //glEnable( GL_TEXTURE_2D );
   AssertGLOK();

#if 0
   static GLfloat cameraZ = -5.0;
   static GLfloat zOffset = 0;
   //cameraZ += 0.1;
   //zOffset += 0.1;
   //cout << "cz: " << cameraZ << ", zo:" << zOffset << endl;

   glMatrixMode (GL_MODELVIEW);
   glLoadIdentity();
   //glTranslatef(0.0,0.0,cameraZ);

    glDisable(GL_BLEND);
    glDisable(GL_POLYGON_SMOOTH);

   GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
   GLfloat mat_shininess[] = { 50.0 };
   GLfloat light_position[] = { 1.0,1.0,1.0,1.0 };
   GLfloat white_light[] = { 1.0,1.0,1.0,1.0 };
   GLfloat mat_ambient_color[] = { 0.8,0.8,0.2,1.0 };
   GLfloat mat_diffuse[] = { 0.1,0.5, 0.8, 1.0 };
   glClearColor(0.0,0.0,0.0,0.0);
   glShadeModel(GL_SMOOTH);
   glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
   glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
   glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
   glLightfv(GL_LIGHT0, GL_POSITION, light_position);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
   glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glEnable(GL_DEPTH_TEST);

    glBegin(GL_TRIANGLES);
    glColor3f(1.0, 1.0, 1.0);
    glVertex3f( 0.9, -0.9, -10.0 + zOffset);
    glVertex3f( 0.9,  0.9, -10.0 + zOffset);
    glVertex3f(-0.9,  0.0, -10.0 + zOffset);
    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(-0.9, -0.9, -20.0 + zOffset);
    glVertex3f(-0.9,  0.9, -20.0 + zOffset);
    glVertex3f( 0.9,  0.0, -5.0 + zOffset);
    glEnd();
#endif
    //glLoadIdentity ();
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

#if defined(RENDERER_PIPELINE_SOFTWARE)
#if defined(USE_ORDER_TABLES)
    SetConstructionOrderTableIndex(_drawPage);
    SetRenderOrderTableIndex(1-_drawPage);

    _orderTable[GetRenderOrderTableIndex()]->Render();
    _drawPage ^= 1;
    _orderTable[GetConstructionOrderTableIndex()]->Clear();
#else // USE_ORDER_TABLES
#error now what?
    assert(0);
#endif
#elif defined(RENDERER_PIPELINE_GL) // defined(RENDERER_PIPELINE_SOFTWARE)

#if 0
    static float xRot;
    xRot += 1.0f;
    static float yRot;
    yRot += 1.0f;
    glPushMatrix();
    AssertGLOK();
    glRotatef(xRot,1.0f,0.0f,0.0f);
    AssertGLOK();
    glRotatef(yRot,0.0f,1.0f,0.0f);
    AssertGLOK();

    GLfloat x,y,z,angle;
    glClearColor(0.0f, 0.0f,0.0f,1.0f);
    AssertGLOK();
    glColor3f(0.0f,1.0f,0.0f);
    AssertGLOK();

    glClear(GL_COLOR_BUFFER_BIT);
    AssertGLOK();
    glDisable( GL_TEXTURE_2D );
    AssertGLOK();
    glBegin(GL_TRIANGLES);

    glColor3ub( 128, 128, 0 );
    z = -50.0f;
    for(angle=0.0f; angle <= (2.0f*3.1415)*3.0f; angle += 0.1f)
    {
        x = 50.0f*sin(angle);
        y = 50.0f*cos(angle);
        glVertex3f(0.0f+200.0f,0.0f,0.0f);
        glVertex3f(x+200.0f,y,z);
        x = 50.0f*sin(angle+10.0);
        y = 50.0f*cos(angle+10.0);
        glVertex3f(x+200.0f,y,z);
        z += 0.5f;
    }
    glEnd();
    AssertGLOK();
    glPopMatrix();

#endif // 0

#elif defined(RENDERER_PIPELINE_GLES) 

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
    GLfloat mat[16], rot[16], scale[16];

    /* Set modelview/projection matrix */
    make_z_rot_matrix(view_rotx, rot);
    make_scale_matrix(0.5, 0.5, 0.5, scale);
    mul_matrix(mat, rot, scale);


    std::cout << "orig matrix = " <<std::endl;
    for(int y=0;y<4;y++)
    {
        std::cout << "  ";
        for(int x=0;x<4;x++)
        {
            std::cout << mat[(y*4)+x] << " ";
        }
        std::cout << std::endl;
    }


    glUniformMatrix4fv(u_matrix, 1, GL_FALSE, mat);

 //   view_rotx += 5.0;

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

    AssertGLOK();
    eglSwapBuffers(halDisplay.eglDisplay, halDisplay.eglSurface);
    AssertGLOK();
#else
#error renderer pipeline not defined!
#endif

    glFlush();
    AssertGLOK();

#if defined(__WIN__)
    // Call function to swap the buffers
	SwapBuffers(hDC);					// Swap Buffers (Double Buffering)
#elif defined(__LINUX__)
    //glXSwapBuffers(halDisplay.mainDisplay, halDisplay.win);
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

//============================================================================

#if defined(USE_ORDER_TABLES)
#error order tables not supported in egl (no reason for them)
#endif									// defined(USE_ORDER_TABLES)

//==============================================================================

void
LoadGLMatrixFromMatrix34(const Matrix34& matrix)
{
    AssertGLOK();

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

    std::cout << "matrix = " << matrix << std::endl;
    std::cout << "matrix = " <<std::endl;
    for(int y=0;y<4;y++)
    {
        std::cout << "  ";
        for(int x=0;x<4;x++)
        {
            std::cout << mat[(y*4)+x] << " ";
        }
        std::cout << std::endl;
    }
    glUniformMatrix4fv(u_matrix, 1, GL_TRUE, mat);
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

                reshape(event.xconfigure.width, event.xconfigure.height);
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

void XEventLoop()
{
    XEvent xev;
    int num_events;

    XFlush(halDisplay.mainDisplay);
    num_events = XPending(halDisplay.mainDisplay);
    while((num_events != 0))
    {
        num_events--;
        XNextEvent(halDisplay.mainDisplay, &xev);
        ProcessXEvents(xev);
    }

    // printf("_joystickButtons = %x\n", _joystickButtons);
}

//==============================================================================

