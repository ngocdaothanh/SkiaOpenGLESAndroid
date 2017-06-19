.. image:: screenshot.png

Skia OpenGL ES 2 Android sample.

This is just a combination of:

* Native Activity sample in Android NDK
* `Hello World <https://skia.googlesource.com/skia/+/master/example/>`_ sample in Skia source code
* `Ejecta sample <https://github.com/phoboslab/Ejecta/blob/master/index.js>`_
  (see `normal_browser.html <https://github.com/ngocdaothanh/SkiaOpenGLESAndroid/blob/master/normal_browser.html>`_ or `JSFiddle <https://jsfiddle.net/to9fo0r8/1/>`_)

See also:

* https://skia.org/user/build
* https://groups.google.com/forum/#!topic/skia-discuss/c9vsrU0OWlE

``jni/skia/libskia_android.so`` is compiled with ``BUILDTYPE=Release`` option (see the guide above):

::

  cd skia/source/code
  gclient sync
  cd trunk
  export ANDROID_SDK_ROOT=/path/to/android/sdk
  export TARGET_DEVICE=xoom
  ./platform_tools/android/bin/android_make -d $TARGET_DEVICE BUILDTYPE=Release
