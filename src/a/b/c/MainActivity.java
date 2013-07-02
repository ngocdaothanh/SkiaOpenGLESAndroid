package a.b.c;

import android.app.NativeActivity;
import android.os.Bundle;

public class MainActivity extends NativeActivity {
    static {
        System.loadLibrary("skia_android");
        System.loadLibrary("native_activity");
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }
}
