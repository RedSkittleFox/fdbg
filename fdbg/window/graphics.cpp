#include <fdbg/window/graphics.hpp>

#include <cstdint>
#include <fdbg/winmin.hpp>
#include <gl/GL.h>
#include <gl/GLU.h>

static HDC hdc;     // device context
static HGLRC hrc;   // opengl context

void create_graphics(HWND wnd)
{
    int32_t pf;
    PIXELFORMATDESCRIPTOR pfd;

    hdc = GetDC(wnd);

    /* there is no guarantee that the contents of the stack that become
       the pfd are zeroed, therefore _make sure_ to clear these bits. */
    memset(&pfd, 0, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;

    pf = ChoosePixelFormat(hdc, &pfd);
    if (pf == 0) 
    {
        MessageBox(NULL, "ChoosePixelFormat() failed:  "
            "Cannot find a suitable pixel format.", "Error", MB_OK);
        ExitProcess(-1);
    }

    if (SetPixelFormat(hdc, pf, &pfd) == FALSE) {
        MessageBox(NULL, "SetPixelFormat() failed:  "
            "Cannot set format specified.", "Error", MB_OK);
        ExitProcess(-1);
    }

    DescribePixelFormat(hdc, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

    hrc = wglCreateContext(hdc);
    wglMakeCurrent(hdc, hrc);
}

void update_graphics()
{
    // glClear(GL_COLOR_BUFFER_BIT);
    // glBegin(GL_TRIANGLES);
    // glColor3f(1.0f, 0.0f, 0.0f);
    // glVertex2i(0, 1);
    // glColor3f(0.0f, 1.0f, 0.0f);
    // glVertex2i(-1, -1);
    // glColor3f(0.0f, 0.0f, 1.0f);
    // glVertex2i(1, -1);
    // glEnd();
    glFlush();
}

void destroy_graphics(HWND wnd)
{
    wglMakeCurrent(NULL, NULL);
    ReleaseDC(wnd, hdc);
    wglDeleteContext(hrc);
}