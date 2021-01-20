package com.example.signcheck;

import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.Signature;
import android.util.Log;
import android.widget.ExpandableListAdapter;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.cert.CertificateEncodingException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;

public class SignatureValidator {
    /**
     * md5加密
     */
    public static String xmd5(byte[] byteStr)
    {
        MessageDigest messageDigest;
        StringBuffer stringBuffer=new StringBuffer();
        try{
            messageDigest=MessageDigest.getInstance("MD5");
            messageDigest.reset();
            messageDigest.update(byteStr);
            byte[] byteArray=messageDigest.digest();
            for(int i=0;i<byteArray.length;i++)
            {
                if(Integer.toHexString(0xFF&byteArray[i]).length()==1){
                        stringBuffer.append("0").append(Integer.toHexString(0xFF&byteArray[i]));
                }else
                {
                    stringBuffer.append(Integer.toHexString(0xFF&byteArray[i]));
                }
            }

        }catch (Exception e)
        {
            e.printStackTrace();
        }
        return stringBuffer.toString().toUpperCase();
    }
    //获取app签名，并进行md5加密
    public static String getSign(Context context)
    {
        try {
            PackageInfo packageInfo=context.getPackageManager().getPackageInfo(context.getPackageName(),PackageManager.GET_SIGNATURES);
            Log.d("YenKoc", "getSign: "+PackageManager.GET_SIGNATURES);
            Signature[] signs=packageInfo.signatures;
            Signature sign=signs[0];
            String signStr=xmd5(sign.toByteArray());
            return signStr;
        }catch (Exception e){
            e.printStackTrace();
        }
        return "";
    }

}
