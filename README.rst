Skia OpenGL ES 2 Android sample.

This is just a combination of:

* Native Activity sample in Android NDK
* Hello Skia sample in Skia source code
* `Ejecta sample <https://github.com/phoboslab/Ejecta/blob/master/index.js>`_ (see normal_browser.html)

See also:

* https://sites.google.com/site/skiadocs/user-documentation/quick-start-guides/android
* https://groups.google.com/forum/#!topic/skia-discuss/c9vsrU0OWlE
* https://code.google.com/p/skia/source/browse/trunk/experimental/SkiaExamples/BaseExample.cpp

jni/skia/libskia_android.so is compiled with BUILDTYPE=Release option (see the guide above):

::

  cd skia/source/code
  gclient sync
  cd trunk
  export ANDROID_SDK_ROOT=/path/to/android/sdk
  export TARGET_DEVICE=xoom
  ./platform_tools/android/bin/android_make -d $TARGET_DEVICE BUILDTYPE=Release
