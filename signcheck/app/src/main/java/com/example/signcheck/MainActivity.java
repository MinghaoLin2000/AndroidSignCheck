package com.example.signcheck;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.Signature;
import android.os.Bundle;
import android.os.Message;
import android.util.Log;
import android.widget.TextView;

import java.security.MessageDigest;
import java.util.Base64;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    private String TAG="YenKoc ";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        String content=SignatureValidator.getSign(this.getApplicationContext());
        Log.d(TAG, "onCreate:"+content);
        TextView tv = findViewById(R.id.sample_text);
           //
        Log.d(TAG, "xxxx: "+check(ContextHolder.getContext()));

    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native boolean check(Context context);
}