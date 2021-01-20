package com.example.signcheck;

import android.content.ContentValues;
import android.content.Context;

public class ContextHolder {
    private static Context sContext;
    public static void setContext(Context context)
    {
        ContextHolder.sContext=context;
    }
    public static Context getContext()
    {
        return sContext;
    }
}
