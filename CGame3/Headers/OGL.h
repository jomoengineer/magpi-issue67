#ifndef OGL_H_INCLUDED
#define OGL_H_INCLUDED

#pragma once

// these headers give the code here
#include <stdio.h>
#include <iostream>
#include <sys/time.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>

// this header is very important
#define FALSE 0
#define TRUE 1
/// esCreateWindow flag - RGB color buffer
#define ES_WINDOW_RGB           0
/// esCreateWindow flag - ALPHA color buffer
#define ES_WINDOW_ALPHA         1
/// esCreateWindow flag - depth buffer
#define ES_WINDOW_DEPTH         2
/// esCreateWindow flag - stencil buffer
#define ES_WINDOW_STENCIL       4
/// esCreateWindow flat - multi-sample buffer
#define ES_WINDOW_MULTISAMPLE   8


class OGL
{
public:
    OGL();
    ~OGL();

    // out Pi to have an OGLES
    typedef struct Target_State
    {
        GLint width;
        GLint height;
        /// Put your user data here...
        //void*       userData;
        EGLDisplay display;
        EGLSurface surface;
        EGLContext context;
        //UserData userData;
        EGLNativeWindowType nativewindow;
       //EGL_DISPMANX_WINDOW_T nativewindow;
    } Target_State;

    GLuint LoadShader(GLenum type, const char *shaderSrc);
    int Init();
    void init_EGL(Target_State *state, int width, int height);
    void esInitContext(Target_State *state);
    GLuint CreateTexture2D(int width, int height,  char* TheData); // this is a handy routine for making textures
    /*Now that we have created a type called a Target_State, we can use it in code.	We can create it here */

    Target_State state;


    GLint programObject; // having only one global P0 is limiting, but for now it will make sense
    GLuint textureID;
};


#endif // OGL_H_INCLUDED
