#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES2/gl2.h>

#include <android/log.h>
#include <android_native_app_glue.h>

#include <GrContext.h>
#include <SkGpuDevice.h>
#include <gl/GrGLInterface.h>

#include <SkCanvas.h>
#include <SkGraphics.h>
#include <SkSurface.h>
#include <SkString.h>
#include <SkTime.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))

/**
 * Shared state for our app.
 */
struct engine {
    struct android_app* app;

    int animating;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;

    GrContext* skiaContext;
    SkCanvas* skiaCanvas;
};

/**
 * Initialize an EGL context for the current display.
 */
static int engine_init_display(struct engine* engine) {
    // Initialize OpenGL ES and EGL

    const EGLint configAttribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
        EGL_BLUE_SIZE,       8,
        EGL_GREEN_SIZE,      8,
        EGL_RED_SIZE,        8,
        EGL_ALPHA_SIZE,      8,
        EGL_STENCIL_SIZE,    8,
        EGL_NONE
    };

    const EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    const EGLint surfaceAttribs[] = {
        EGL_RENDER_BUFFER, EGL_BACK_BUFFER,
        EGL_NONE
    };

    EGLint w, h, dummy, format;
    EGLint numConfigs;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, NULL, NULL);

    /* Here, the application chooses the configuration it desires. In this
     * sample, we have a very simplified selection process, where we pick
     * the first EGLConfig that matches our criteria */
    eglChooseConfig(display, configAttribs, &config, 1, &numConfigs);

    /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
     * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
     * As soon as we picked a EGLConfig, we can safely reconfigure the
     * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

    ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

    surface = eglCreateWindowSurface(display, config, engine->app->window, surfaceAttribs);
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);

    if (!eglMakeCurrent(display, surface, surface, context)) {
        LOGW("Unable to eglMakeCurrent");
        return -1;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    LOGI("Canvas size: %d x %d", w, h);
    glViewport(0, 0, w, h);

    engine->display = display;
    engine->context = context;
    engine->surface = surface;
    engine->width   = w;
    engine->height  = h;

    // Initialize GL state.
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glShadeModel(GL_SMOOTH);

    // eglSwapBuffers should not automatically clear the screen
    eglSurfaceAttrib(display, surface, EGL_SWAP_BEHAVIOR, EGL_BUFFER_PRESERVED);

    // Initialize Skia OpenGL ES

    SkGraphics::Init();

    const GrGLInterface* fInterface = GrGLCreateNativeInterface();
    engine->skiaContext = GrContext::Create(kOpenGL_GrBackend, (GrBackendContext) fInterface);

    GrBackendRenderTargetDesc desc;
    desc.fWidth       = w;
    desc.fHeight      = h;
    desc.fConfig      = kSkia8888_GrPixelConfig;
    desc.fOrigin      = kBottomLeft_GrSurfaceOrigin;
    desc.fSampleCnt   = 0;
    desc.fStencilBits = 8;

    GrGLint buffer;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &buffer);
    // Alternative:
    // GR_GL_GetIntegerv(fInterface, GR_GL_FRAMEBUFFER_BINDING, &buffer);
    desc.fRenderTargetHandle = buffer;

    GrRenderTarget* renderTarget = engine->skiaContext->wrapBackendRenderTarget(desc);
    SkAutoTUnref<SkBaseDevice> device(new SkGpuDevice(engine->skiaContext, renderTarget));

    // Leaking fRenderTarget. Either wrap it in an SkAutoTUnref<> or unref it
    // after creating the device.
    SkSafeUnref(renderTarget);

    engine->skiaCanvas = new SkCanvas(device);

    return 0;
}

//------------------------------------------------------------------------------

static void demo_hello_skia(SkCanvas* canvas, int w, int h) {
    // Clear
    canvas->drawColor(SK_ColorWHITE);

    // Setup a SkPaint for drawing our text
    SkPaint paint;
    paint.setColor(SK_ColorBLACK); // This is a solid black color for our text
    paint.setTextSize(SkIntToScalar(30)); // Sets the text size to 30 pixels
    paint.setAntiAlias(true); // We turn on anti-aliasing so that the text to looks good

    // Draw some text
    SkString text("Skia is Best!");
    SkScalar fontHeight = paint.getFontSpacing();
    canvas->drawText(text.c_str(), text.size(), // text's data and length
         10, fontHeight,            // X and Y coordinates to place the text
         paint);                    // SkPaint to tell how to draw the text

    // Adapt the SkPaint for drawing blue lines
    //paint.setAntiAlias(false); // Turning off anti-aliasing speeds up the line drawing
    paint.setColor(0xFF0000FF); // This is a solid blue color for our lines
    paint.setStrokeWidth(SkIntToScalar(2)); // This makes the lines have a thickness of 2 pixels

    // Draw some interesting lines using trig functions
    long elapsedTime = clock() / (CLOCKS_PER_SEC / 1000);
    for (int i = 0; i < 100; i++) {
        float x = (float) i / 99.0f;
        float offset = elapsedTime / 1000.0f;
        canvas->drawLine(
            sin(x * M_PI + offset) * w, 0,  // first endpoint
            cos(x * M_PI + offset) * w, h,  // second endpoint
            paint);                         // SkPapint to tell how to draw the line
    }
}

//------------------------------------------------------------------------------
// https://github.com/phoboslab/Ejecta/blob/master/index.js
// https://github.com/3dseals/Ejecta-X/blob/master/project/android/assets/build/index.js

struct Curve {
  double  current;
  double  inc;
  SkColor color;
};

const int NCURVES = 70;
Curve curves[NCURVES];

static void initCurvesOnce() {
    static bool initedCurves = false;
    if (initedCurves) return;
    initedCurves = true;

    for (int i = 0; i < NCURVES; i++ ) {
        Curve curve;
        curve.current = ((double) rand() / RAND_MAX) * 1000;
        curve.inc     = ((double) rand() / RAND_MAX) * 0.005 + 0.002;

        int color   = rand();
        int r       = SkColorGetR(color);
        int g       = SkColorGetG(color);
        int b       = SkColorGetB(color);
        curve.color = SkColorSetRGB(r, g, b);

        curves[i] = curve;
    }
}

const int NP = 8;
double p[] = {0,0, 0,0, 0,0, 0,0};

static void demo_ejectax(SkCanvas* canvas, int w, int h) {
    initCurvesOnce();

    int w2 = w / 2;
    int h2 = h / 2;

    // Blur out
    canvas->drawARGB(31, 0, 0, 0);

    // Calculate curve positions and draw
    for (int i = 0; i < NCURVES; i++) {
        Curve curve = curves[i];
        curve.current += curve.inc;
        curves[i] = curve;

        for (int j = 0; j < NP; j+=2) {
            double a = sin(curve.current * (j+3) * 373 * 0.0001);
            double b = sin(curve.current * (j+5) * 927 * 0.0002);
            double c = sin(curve.current * (j+5) * 573 * 0.0001);

            p[j] = (a * a * b + c * a + b) * w * c + w2;
            p[j+1] = (a * b * b + c - a * b *c) * h2 + h2;
        }

        SkPath path;
        path.moveTo(p[0], p[1]);
        path.cubicTo(p[2], p[3], p[4], p[5], p[6], p[7]);

        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(curve.color);
        paint.setAlpha(127);

        paint.setDither(true);

        canvas->drawPath(path, paint);
    }
}

//------------------------------------------------------------------------------

/**
 * Just the current frame in the display.
 */
static void engine_draw_frame(struct engine* engine) {
    if (engine->display == NULL) return;

    //demo_hello_skia(engine->skiaCanvas, engine->width, engine->height);
    demo_ejectax(engine->skiaCanvas, engine->width, engine->height);

    engine->skiaContext->flush();
    eglSwapBuffers(engine->display, engine->surface);
}

/**
 * Tear down the EGL context currently associated with the display.
 */
static void engine_term_display(struct engine* engine) {
    if (engine->display != EGL_NO_DISPLAY) {
        eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (engine->context != EGL_NO_CONTEXT) {
            eglDestroyContext(engine->display, engine->context);
        }
        if (engine->surface != EGL_NO_SURFACE) {
            eglDestroySurface(engine->display, engine->surface);
        }
        eglTerminate(engine->display);
    }
    engine->animating = 0;
    engine->display   = EGL_NO_DISPLAY;
    engine->context   = EGL_NO_CONTEXT;
    engine->surface   = EGL_NO_SURFACE;
}

/**
 * Process the next input event.
 */
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
    return 0;
}

/**
 * Process the next main command.
 */
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
    struct engine* engine = (struct engine*)app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            break;
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if (engine->app->window != NULL) {
                engine_init_display(engine);
                engine_draw_frame(engine);
            }
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            engine_term_display(engine);
            break;
        case APP_CMD_GAINED_FOCUS:
            engine->animating = 1;
            break;
        case APP_CMD_LOST_FOCUS:
            engine->animating = 0;
            engine_draw_frame(engine);
            break;
    }
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* state) {
    struct engine engine;

    // Make sure glue isn't stripped.
    app_dummy();

    memset(&engine, 0, sizeof(engine));
    state->userData     = &engine;
    state->onAppCmd     = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;
    engine.app = state;

    if (state->savedState != NULL) {
    }

    // Loop waiting for stuff to do.
    engine.animating = 1;
    while (1) {
        // Read all pending events.
        int ident;
        int events;
        struct android_poll_source* source;

        // If not animating, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
        while ((ident=ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events,
                (void**)&source)) >= 0) {

            // Process this event.
            if (source != NULL) {
                source->process(state, source);
            }

            // Check if we are exiting.
            if (state->destroyRequested != 0) {
                engine_term_display(&engine);
                return;
            }
        }

        if (engine.animating) {
            // Drawing is throttled to the screen update rate, so there
            // is no need to do timing here.
            engine_draw_frame(&engine);
        }
    }
}
