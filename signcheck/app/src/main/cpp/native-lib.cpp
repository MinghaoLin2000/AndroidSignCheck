#include <jni.h>
#include <string>
#include<android/log.h>

#define  LOG_TAG    "native-dev"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

const char *APP_SIGNATURE = "63E90909BA44DB1AC73104785E71C63C";
const char *CLASS_NAME_NATIVECONTEXT = "com/example/signcheck/ContextHolder";
const char *METHOD_NAME_GET_CONTEXT = "getContext";
const char *METHOD_SIGNATURE_GETCONTEXT = "()Landroid/content/Context;";

void ByteToHexStr(const char *source, char *dest, int sourceLen) {
    short i;
    char highByte, lowByte;

    for (i = 0; i < sourceLen; i++) {
        highByte = source[i] >> 4;
        lowByte = source[i] & 0x0f;
        highByte += 0x30;

        if (highByte > 0x39) {
            dest[i * 2] = highByte + 0x07;
        } else {
            dest[i * 2] = highByte;
        }

        lowByte += 0x30;
        if (lowByte > 0x39) {
            dest[i * 2 + 1] = lowByte + 0x07;
        } else {
            dest[i * 2 + 1] = lowByte;
        }
    }
}

jstring ToMd5(JNIEnv *env, jbyteArray source) {
    // MessageDigest类
    jclass classMessageDigest = env->FindClass("java/security/MessageDigest");
    // MessageDigest.getInstance()静态方法
    jmethodID midGetInstance = env->GetStaticMethodID(classMessageDigest, "getInstance",
                                                      "(Ljava/lang/String;)Ljava/security/MessageDigest;");
    // MessageDigest object
    jobject objMessageDigest = env->CallStaticObjectMethod(classMessageDigest, midGetInstance,
                                                           env->NewStringUTF("md5"));

    // update方法，这个函数的返回值是void，写V
    jmethodID midUpdate = env->GetMethodID(classMessageDigest, "update", "([B)V");
    env->CallVoidMethod(objMessageDigest, midUpdate, source);

    // digest方法
    jmethodID midDigest = env->GetMethodID(classMessageDigest, "digest", "()[B");
    jbyteArray objArraySign = (jbyteArray) env->CallObjectMethod(objMessageDigest, midDigest);

    jsize intArrayLength = env->GetArrayLength(objArraySign);
    jbyte *byte_array_elements = env->GetByteArrayElements(objArraySign, NULL);
    size_t length = (size_t) intArrayLength * 2 + 1;
    char *char_result = (char *) malloc(length);
    memset(char_result, 0, length);

    // 将byte数组转换成16进制字符串，发现这里不用强转，jbyte和unsigned char应该字节数是一样的
    ByteToHexStr((const char *) byte_array_elements, char_result, intArrayLength);
    // 在末尾补\0
    *(char_result + intArrayLength * 2) = '\0';

    jstring stringResult = env->NewStringUTF(char_result);
    // release
    env->ReleaseByteArrayElements(objArraySign, byte_array_elements, JNI_ABORT);
    // 释放指针使用free
    free(char_result);

    return stringResult;
}

jstring loadSignature(JNIEnv *env, jobject context) {
    // 获得Context类
    jclass cls = env->GetObjectClass(context);
    // 得到getPackageManager方法的ID
    jmethodID mid = env->GetMethodID(cls, "getPackageManager",
                                     "()Landroid/content/pm/PackageManager;");

    // 获得应用包的管理器
    jobject pm = env->CallObjectMethod(context, mid);

    // 得到getPackageName方法的ID
    mid = env->GetMethodID(cls, "getPackageName", "()Ljava/lang/String;");
    // 获得当前应用包名
    jstring packageName = (jstring) env->CallObjectMethod(context, mid);

    // 获得PackageManager类
    cls = env->GetObjectClass(pm);
    // 得到getPackageInfo方法的ID
    mid = env->GetMethodID(cls, "getPackageInfo",
                           "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;");
    // 获得应用包的信息
    jobject packageInfo = env->CallObjectMethod(pm, mid, packageName, 0x40); //GET_SIGNATURES = 64;
    // 获得PackageInfo 类
    cls = env->GetObjectClass(packageInfo);
    // 获得签名数组属性的ID
    jfieldID fid = env->GetFieldID(cls, "signatures", "[Landroid/content/pm/Signature;");
    // 得到签名数组
    jobjectArray signatures = (jobjectArray) env->GetObjectField(packageInfo, fid);
    // 得到签名
    jobject signature = env->GetObjectArrayElement(signatures, 0);

    // 获得Signature类
    cls = env->GetObjectClass(signature);
    // 得到toCharsString方法的ID
    mid = env->GetMethodID(cls, "toByteArray", "()[B");
    // 返回当前应用签名信息
    jbyteArray signatureByteArray = (jbyteArray) env->CallObjectMethod(signature, mid);

    return ToMd5(env, signatureByteArray);
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_signcheck_MainActivity_check(JNIEnv *env, jobject context2,jobject context1) {

    jstring appSignature = loadSignature(env, context1); // 当前 App 的签名
    jstring releaseSignature = env->NewStringUTF(APP_SIGNATURE); // 发布时候的签名
    const char *charAppSignature = env->GetStringUTFChars(appSignature, NULL);
    const char *charReleaseSignature = env->GetStringUTFChars(releaseSignature, NULL);

    LOGI("  start cmp  getSignature");
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "%s", charAppSignature);
    LOGI("  start cmp  getReleaseSignature");
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "%s", charAppSignature);

    jboolean result = JNI_FALSE;
    // 比较是否相等
    if (charAppSignature != NULL && charReleaseSignature != NULL) {
        if (strcmp(charAppSignature, charReleaseSignature) == 0) {
            result = JNI_TRUE;
        }
    }

    env->ReleaseStringUTFChars(appSignature, charAppSignature);
    env->ReleaseStringUTFChars(releaseSignature, charReleaseSignature);

    return result;
}

/**
 * 检查加载该so的应用的签名，与预置的签名是否一致
 */

static jboolean checkSignature(JNIEnv *env) {
    // 为了拿到 Context，需要通过反射，调用静态方法来获取：这里是在 Java 层有一个 ContextHolder 持有 Application 的 Context

    // 得到当前 App 的 ContextHolder 类
    jclass classNativeContextHolder = env->FindClass(CLASS_NAME_NATIVECONTEXT);

    // 得到 getContext 静态方法
    jmethodID midGetContext = env->GetStaticMethodID(classNativeContextHolder,
                                                     METHOD_NAME_GET_CONTEXT,
                                                     METHOD_SIGNATURE_GETCONTEXT);

    // 调用 getContext 方法得到 Context 对象
    jobject appContext = env->CallStaticObjectMethod(classNativeContextHolder, midGetContext);
    jobject tt=NULL;
    if (appContext != NULL) {
        jboolean signatureValid =Java_com_example_signcheck_MainActivity_check(env,tt,appContext);
        return signatureValid;
    }

    return JNI_FALSE;
}

/**
 * 加载 so 文件的时候，会触发 OnLoad
 * 检测失败，返回 -1，App 就会 Crash
 */

JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    LOGI("  JNI_OnLoad  ");
    if (vm->GetEnv((void **) (&env), JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }
    LOGI("  start checkSignature  ");
    if (checkSignature(env) != JNI_TRUE) {
        LOGI("  checkSignature = false ");
        // 检测不通过，返回 -1 就会使 App crash
//        return -1;
    }

    return JNI_VERSION_1_6;
}

