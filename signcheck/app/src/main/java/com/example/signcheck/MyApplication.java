package com.example.signcheck;

import android.app.Application;

public class MyApplication extends Application {
    public void onCreate() {
        super.onCreate();
        ContextHolder.setContext(this);
    }
}
