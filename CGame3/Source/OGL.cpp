/*
 * OGL.cpp
 */
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include  "OGL.h"
#include <assert.h> // assert is a way of making sure that things are what we expect and if not it will stop and tell us.
#include  <X11/Xlib.h>
#include  <X11/Xatom.h>
#include  <X11/Xutil.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define EGL_FALSE            0
#define EGL_TRUE            1
// X11 related local variables
static Display *x_display = NULL;


using namespace std;

/*  For beginners this might be a bit too much, but just include the file for now and use the OGL.h file when you need to use
	OpenGLES2.0 instructions.
	In order to set up a screen, we will need to ask the video system to create a couple of buffer areas with
	the right type of pixels and also indicate some of the hardware features we want to switch on. This is done using EGL libs.
	The process of setting up a graphic screen is a little technical but mainly we must create an EGLscreen
	which itself is optimised on RaspPi to use a special format our Broadcom GPU givs us for its display, all handled in
	the bcm_host.h file so we can forget about it, but we do need to tell EGL we are using DISPMANX systems
	Once we have our EGL/DISMPANX surface and displays systems it means OpenGLES2.0 has something to work with and we can
	draw things using our shaders.
	For convenience we will set up a simple default shader, but normally we would have a few different ones for different
	effects.
	You can totally ignore the contents of this file until you feel you want to explore how to set things up, but do make
	sure you have an #include OGL.h in any class that wants to play with OpenGL and that you double check that you create
	an instance of OGL and call its init() method to set it all up of us.
	Also note this file makes use of fprintf instead of cout, its just a tidier way to format printed text.
*/

OGL::OGL() {};
OGL::~OGL() {};

// this is a simple definition of an attribute list to create an EGL window, there are many variations we could have
static const EGLint attribute_list[] =
{
// these 1st 4, let us define the size of our colour componants and from that the size of a pixel.
// In this case 32bits made up of 8xR, 8xG,8xB and 8xA
	EGL_RED_SIZE,
	8,
	EGL_GREEN_SIZE,
	8,
	EGL_BLUE_SIZE,
	8,
	EGL_ALPHA_SIZE,
	8,
	EGL_SAMPLE_BUFFERS,
	1, // if you want anti alias at a slight fps cost
	EGL_SAMPLES,
	4,  //keep these 2 lines
	// These lines set up some basic internal features that help
	EGL_DEPTH_SIZE,
	1,
	//	These lines tell EGL how the surface is made up.
	EGL_SURFACE_TYPE,
	EGL_WINDOW_BIT,
	EGL_NONE
};

// we will also need to tell our context what we want.
static const EGLint context_attributes[] =
{
	EGL_CONTEXT_CLIENT_VERSION,
	2,
	EGL_NONE
};

// This will set up the Broadcomm GPU version of an EGL display,

//
// CreateEGLContext()
//
//    Creates an EGL rendering context and all associated elements

/* We've added a simple routine to create textures
 Create a texture with width and height
*/
GLuint OGL::CreateTexture2D(int width, int height,  char* TheData)
{
	// Texture handle
	GLuint textureId;
	// Set the alignment
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	// Generate a texture object
	glGenTextures(1, &textureId);
	// Bind the texture object
	glBindTexture(GL_TEXTURE_2D, textureId);
	// set it up
	glTexImage2D(GL_TEXTURE_2D,
		0,
		GL_RGBA,
		width,
		height,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		TheData);

	if (glGetError() != GL_NO_ERROR) printf("We got an error"); // its a good idea to test for errors.

	// Set the filtering mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	return textureId;
}

void OGL::esInitContext(Target_State *state)
{
   if ( state != NULL )
   {
      memset( state, 0, sizeof( Target_State) );
   }
   else
   {
       std::cout << "Failed to memset state" << std::endl;
   }
}
//void OGL::init_EGL(Target_State *state, int width, int height, int FBResX, int FBResY)
void OGL::init_EGL(Target_State *state, int width, int height)

{
#define ES_WINDOW_RGB           0

    std::cout << "init openGLES started\n";

    state->width = width;
    state->height = height;

    EGLint numConfigs;
    EGLint majorVersion;
    EGLint minorVersion;
    EGLDisplay display;
    EGLContext context;
    EGLSurface surface;
    EGLConfig config;
    EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE };
    EGLint attribute_list[] =
    {
       EGL_RED_SIZE,       5,
       EGL_GREEN_SIZE,     6,
       EGL_BLUE_SIZE,      5,
       EGL_ALPHA_SIZE,     (ES_WINDOW_RGB & ES_WINDOW_ALPHA) ? 8 : EGL_DONT_CARE,
       EGL_DEPTH_SIZE,     (ES_WINDOW_RGB & ES_WINDOW_DEPTH) ? 8 : EGL_DONT_CARE,
       EGL_STENCIL_SIZE,   (ES_WINDOW_RGB & ES_WINDOW_STENCIL) ? 8 : EGL_DONT_CARE,
       EGL_SAMPLE_BUFFERS, (ES_WINDOW_RGB & ES_WINDOW_MULTISAMPLE) ? 1 : 0,
       EGL_NONE
   };
//    EGLint result = eglChooseConfig(state->display, attribute_list, &config, 1, &numConfigs);

      /* create a native window */

    Window root;
    XSetWindowAttributes swa;
    XSetWindowAttributes  xattr;
    Atom wm_state;
    XWMHints hints;
    XEvent xev;
    EGLConfig ecfg;
    EGLint num_config;
    Window win;
    Screen *screen;
    eglBindAPI(EGL_OPENGL_ES_API);
    /*
     * X11 native display initialization
     */
    // WinCreate
    x_display = XOpenDisplay(NULL);

    if (x_display == NULL)
    {
        return; // we need to trap this;
    }
    root = DefaultRootWindow(x_display);
    screen = ScreenOfDisplay(x_display, 0);
    width = screen->width;
    height = screen->height; // resplution of screen

    state->width = width;
    state->height = height;

    swa.event_mask  =  ExposureMask | PointerMotionMask | KeyPressMask | KeyReleaseMask;
    swa.background_pixmap = None;
    swa.background_pixel  = 0;
    swa.border_pixel      = 0;
    swa.override_redirect = true;

    win = XCreateWindow(
        x_display,
        root,
        0,
        0,
        width,
        height,
        0,
        CopyFromParent,
        InputOutput,
        CopyFromParent,
        CWEventMask,
        &swa);

    XSelectInput(x_display, win, KeyPressMask | KeyReleaseMask);

    xattr.override_redirect = FALSE;
    XChangeWindowAttributes(x_display, win, CWOverrideRedirect, &xattr);

    hints.input = TRUE;
    hints.flags = InputHint;
    XSetWMHints(x_display, win, &hints);

    std::cout << "config openGLES X11 window\n";
    char* title = (char*)"x11 window Maze3dHunt";
        // make the window visible on the screen
    XMapWindow(x_display, win);
    XStoreName(x_display, win, title);

        // get identifiers for the provided atom name strings
    wm_state = XInternAtom(x_display, "_NET_WM_STATE", FALSE);
    memset(&xev, 0, sizeof(xev));
    xev.type                 = ClientMessage;
    xev.xclient.window       = win;
    xev.xclient.message_type = wm_state;
    xev.xclient.format       = 32;
    xev.xclient.data.l[0]    = 1;
    xev.xclient.data.l[1]    = FALSE;
    XSendEvent(
      x_display,
        DefaultRootWindow(x_display),
        FALSE,
        SubstructureNotifyMask,
        &xev);

    std::cout << "Initialized XInternAtom\n";
    state->nativewindow = (EGLNativeWindowType) win;

    //  CreateEGLContext
    // Get Display
    //display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    display = eglGetDisplay((EGLNativeDisplayType)x_display);
    if (display == EGL_NO_DISPLAY)
    {
        return; // EGL_FALSE;
    }

    // Initialize EGL
    if (!eglInitialize(display, &minorVersion,&majorVersion))
    {

        EGLint err = eglGetError();
        return;// EGL_FALSE;
    }

    // Get configs
    if (!eglGetConfigs(display, NULL, 0, &numConfigs))
    {
        return;// EGL_FALSE;
    }

    // Choose config
    if (!eglChooseConfig(display, attribute_list, &config, 1, &numConfigs))
    {
        return;// EGL_FALSE;
    }

    // Create a surface
    surface = eglCreateWindowSurface(display, config, state->nativewindow, NULL);
    if (surface == EGL_NO_SURFACE)
    {
        std::cout << "Failed to Create Window Surface\n";
        return;// EGL_FALSE;
    }

    // Create a GL context
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
    if (context == EGL_NO_CONTEXT)
    {
        return;// EGL_FALSE;
    }

    // Make the context current
    if (!eglMakeCurrent(display, surface, surface, context))
    {
        return;// EGL_FALSE;
    }

    state->display = display;
    state->surface = surface;
    state->context = context;
    //return;// EGL_TRUE;

    std::cout << "Print CPU info\n";
    printf("This GPU supplied by  :%s\n", glGetString(GL_VENDOR));
    printf("This GPU supports     :%s\n", glGetString(GL_VERSION));
    printf("This GPU Renders with :%s\n", glGetString(GL_RENDERER));
    printf("This GPU supports     :%s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    printf("This GPU supports these extensions    :%s\n", glGetString(GL_EXTENSIONS));


}



/*
 This is a fairly standard Shader loader and generator
 enter with the type of shader needed and a pointer to where the code text is
*/
GLuint OGL::LoadShader(GLenum type, const char *shaderSrc)
{
// see if we can allcoate a new shader
	GLuint TheShader = glCreateShader(type);
	if (TheShader == 0) return FALSE; // return FALSE (0) to indicate we failed to allocate this shader.
// note...allocating a shader is not the same as generating one. It just means a space was made available
// and our next step is to put the code in there.

	// pass the source
	glShaderSource(TheShader, 1, &shaderSrc, NULL);

	// Compile the shader
	glCompileShader(TheShader);

	GLint  IsCompiled; // we need a value to be set if we fail or succeed.

// Compiling can result in errors, so we need to make sure we find out if it compiled,
// but also its useful to report the error
	glGetShaderiv(TheShader, GL_COMPILE_STATUS, &IsCompiled);  // if compile works, TheShader will have a non 0 value

// a compiler fail means we need to fix the error manually, so output the compiler errors to help us
	if (!IsCompiled)
	{
		GLint RetinfoLen = 0;
		glGetShaderiv(TheShader, GL_INFO_LOG_LENGTH, &RetinfoLen);
		if (RetinfoLen > 1)
		{ // standard output for errors
			char* infoLog = (char*) malloc(sizeof(char) * RetinfoLen);
			glGetShaderInfoLog(TheShader, RetinfoLen, NULL, infoLog);
			fprintf(stderr, "Error compiling this shader:\n%s\n", infoLog); // this is a useful formatting print system
			free(infoLog);
		}
		glDeleteShader(TheShader); // we did allocate a shader, but since we failed to compile we need to remove it.
		return FALSE; // indicate our failure to make the shader
	}
	return TheShader; // if we succeeded we will have a valuer in TheShader, return it.
}

///
// Initialize open GL and create our shaders
//
int OGL::Init()
{
	//bcm_host_init(); // make sure we have set up our Broadcom
    //ESContext esContext;
    //programObject userData;

    esInitContext(&state);
    //state.userData = &userData;
	//init_EGL(&state, 1024, 768); // this will set up the EGL stuff, its complex so put in a seperate method
	init_EGL(&state, 320, 240); // this will set up the EGL stuff, its complex so put in a seperate method

		// HERE we will load in our graphic image to play with and convert it into a texture
	int Grwidth, Grheight, comp; // these are variables we will use to get info

// this is a strange instruction with * and & symbols which we'll discuss soon, but what it does is simple
// it loads a graphic image into memory and stores its width and height into variables Grwidth and Grheight, don't worry about comp or the 4, but they are needed
	unsigned char *data = stbi_load("images/MagPicomp.png", &Grwidth, &Grheight, &comp, 4); // ask it to load 4 componants since its rgba

	//now its loaded we have to create a texture, which will have a "handle" ID that can be stored, we have added a nice function to make this easy
    textureID = CreateTexture2D(Grwidth, Grheight, (char*) data); //just pass the width and height of the graphic, and where it is located and we can make a texture

/*
 *
 *we're done making the texture but something to think about,  where is the data we made it from? We'll find out soon.
 *
 **/


// we'll need a base colour to clear the screen this sets it up here (we can change it anytime
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

// once the BCM has been initialsed and EGL/DISPMANX is up and runnng, all re really need now is some shaders to work with
// since OpenGLES can't actually draw without them, so we will set up a very simple default pair of shaders, and turn them
// into a Program Object which we will set as the default


// this is our shader code, contained in an array of text
// there are much better ways to do this, we will cover later
	GLbyte vShaderStr[] =
		"attribute vec4 a_position;\n"
		"uniform vec4 u_position;\n"
		"attribute vec2 a_texCoord;\n"
		"varying vec2 v_texCoord;\n"
		"void main()\n"
		"{gl_Position = a_position + u_position; \n"
		" v_texCoord=a_texCoord;"
		"}\n";

	GLbyte fShaderStr[] =
        "precision mediump float;\n"
		"varying vec2 v_texCoord;\n"
		"uniform sampler2D s_texture;\n"
		"void main()\n"
	    "{\n"
        "gl_FragColor = texture2D( s_texture, v_texCoord );\n"
        "}\n";
// shaders next, lets create variables to hold these
	GLuint vertexShader, fragmentShader; // we need some variables


// Load and compile the vertex/fragment shaders
	vertexShader = LoadShader(GL_VERTEX_SHADER, (char*)vShaderStr);
	fragmentShader = LoadShader(GL_FRAGMENT_SHADER, (char*)fShaderStr);

	if (vertexShader == 0 || fragmentShader == 0) // test if they both have non 0 values
	{
		return FALSE; // one of them is false, the report will have been printed, and we cannot continue
	}

// Create the program object
	programObject = glCreateProgram();

	if (programObject == 0)
	{
		cout << "Unable to create our Program Object " << endl;
		return FALSE; // there was a failure here

	}

// now we have teh V and F shaders  attach them to the progam object
	glAttachShader(programObject, vertexShader);
	glAttachShader(programObject, fragmentShader);

// Link the program
	glLinkProgram(programObject);
// Check the link status
	GLint AreTheylinked;

	glGetProgramiv(programObject, GL_LINK_STATUS, &AreTheylinked);
	if (!AreTheylinked)
	{
		GLint RetinfoLen = 0;
// check and report any errors
		glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &RetinfoLen);
		if (RetinfoLen > 1)
		{
			GLchar* infoLog = (GLchar*)malloc(sizeof(char) * RetinfoLen);
			glGetProgramInfoLog(programObject, RetinfoLen, NULL, infoLog);
			fprintf(stderr, "Error linking program:\n%s\n", infoLog);
			free(infoLog);
		}
		glDeleteProgram(programObject);
		return FALSE;
	}


	glUseProgram(programObject); // we only plan to use 1 program object so tell openGL to use this one
	return TRUE;
}
