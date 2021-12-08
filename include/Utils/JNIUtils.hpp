#pragma once

#include <jni.h>

// I fucking hate everything.

// This is used to pass literally nothing to a parameter
#define NOTHING

// Log Function

#define LOG_JNI_FUNCTION(env, objectToTest) \
if (objectToTest == nullptr) { __android_log_print(ANDROID_LOG_ERROR, "HotSwappableMods", "Failed to get \"%s\"", ""#objectToTest); } \
else { __android_log_print(ANDROID_LOG_VERBOSE, "HotSwappableMods", "Got \"%s\"", ""#objectToTest); } \

// Get Class

#define GET_JCLASS(env, className, classPath, type) \
type className = env->FindClass(classPath); \
LOG_JNI_FUNCTION(env, className)

#define GET_JOBJECT_JCLASS(env, className, object, type) \
type className = env->GetObjectClass(object); \
LOG_JNI_FUNCTION(env, className)

// New Object

#define NEW_JOBJECT(env, objectName, clazz, sig, type, ...) \
jmethodID GET_JMETHODID(env, objectName##_MethodID, clazz, "<init>", sig); \
type objectName = CALL_METHOD_FROM_JMETHODID(env, objectName, clazz, NewObject, objectName##_MethodID, __VA_ARGS__ )

// Call Void Method

#define CALL_VOID_METHOD(env, object, methodName, sig, ...) \
GET_JOBJECT_JCLASS(env, methodName##_Class, object, jclass); \
jmethodID GET_JMETHODID(env, methodName##_MethodID, methodName##_Class, ""#methodName, sig); \
CALL_METHOD_FROM_JMETHODID_WITHOUT_LOG(env, object, CallVoidMethod, methodName##_MethodID, __VA_ARGS__ )

#define CALL_STATIC_VOID_METHOD(env, clazz, methodName, sig, ...) \
jmethodID GET_STATIC_JMETHODID(env, methodName##_MethodID, clazz, ""#methodName, sig); \
CALL_METHOD_FROM_JMETHODID_WITHOUT_LOG(env, clazz, CallStaticVoidMethod, methodName##_MethodID, __VA_ARGS__ )

// Call Object Method

#define CALL_JOBJECT_METHOD(env, objectName, object, methodName, sig, type, ...) \
GET_JOBJECT_JCLASS(env, objectName##_Class, object, jclass); \
jmethodID GET_JMETHODID(env, objectName##_MethodID, objectName##_Class, methodName, sig); \
type objectName = CALL_METHOD_FROM_JMETHODID(env, objectName, object, CallObjectMethod, objectName##_MethodID, __VA_ARGS__ )

#define CALL_STATIC_JOBJECT_METHOD(env, objectName, clazz, methodName, sig, type, ...) \
jmethodID GET_STATIC_JMETHODID(env, objectName##_MethodID, clazz, methodName, sig); \
type objectName = CALL_METHOD_FROM_JMETHODID(env, objectName, clazz, CallStaticObjectMethod, objectName##_MethodID, __VA_ARGS__ )

// Call String Method

#define CALL_JSTRING_METHOD(env, stringName, object, methodName, sig, type, ...) \
GET_JOBJECT_JCLASS(env, objectName##_Class, object, jclass); \
jmethodID GET_JMETHODID(env, stringName##_MethodID, objectName##_Class, methodName, sig); \
type stringName = (jstring)CALL_METHOD_FROM_JMETHODID(env, stringName, object, CallObjectMethod, stringName##_MethodID, __VA_ARGS__ )

#define CALL_STATIC_JSTRING_METHOD(env, stringName, clazz, methodName, sig, type, ...) \
jmethodID GET_STATIC_JMETHODID(env, stringName##_MethodID, clazz, methodName, sig); \
type stringName = (jstring)CALL_METHOD_FROM_JMETHODID(env, stringName, clazz, CallStaticObjectMethod, stringName##_MethodID, __VA_ARGS__ )

// Call Long Method

#define CALL_JLONG_METHOD(env, longName, object, methodName, sig, type, ...) \
GET_JOBJECT_JCLASS(env, objectName##_Class, object, jclass); \
jmethodID GET_JMETHODID(env, longName##_MethodID, objectName##_Class, methodName, sig); \
type longName = CALL_METHOD_FROM_JMETHODID_WITHOUT_LOG(env, object, CallLongMethod, longName##_MethodID, __VA_ARGS__ )

#define CALL_STATIC_JLONG_METHOD(env, longName, clazz, methodName, sig, type, ...) \
jmethodID GET_STATIC_JMETHODID(env, longName##_MethodID, clazz, methodName, sig); \
type longName = CALL_METHOD_FROM_JMETHODID_WITHOUT_LOG(env, clazz, CallStaticLongMethod, longName##_MethodID, __VA_ARGS__ )

// Call Int Method

#define CALL_JINT_METHOD(env, intName, object, methodName, sig, type, ...) \
GET_JOBJECT_JCLASS(env, objectName##_Class, object, jclass); \
jmethodID GET_JMETHODID(env, intName##_MethodID, objectName##_Class, methodName, sig); \
type intName = CALL_METHOD_FROM_JMETHODID_WITHOUT_LOG(env, object, CallIntMethod, intName##_MethodID, __VA_ARGS__ )

#define CALL_STATIC_JINT_METHOD(env, intName, clazz, methodName, sig, type, ...) \
jmethodID GET_STATIC_JMETHODID(env, intName##_MethodID, clazz, methodName, sig); \
type intName = CALL_METHOD_FROM_JMETHODID_WITHOUT_LOG(env, clazz, CallStaticIntMethod, intName##_MethodID, __VA_ARGS__ )

// Get MethodID

#define GET_JMETHODID(env, methodIDName, clazz, methodName, sig) \
methodIDName = env->GetMethodID(clazz, methodName, sig); \
LOG_JNI_FUNCTION(env, methodIDName)

#define GET_STATIC_JMETHODID(env, methodIDName, clazz, methodName, sig) \
methodIDName = env->GetStaticMethodID(clazz, methodName, sig); \
LOG_JNI_FUNCTION(env, methodIDName)

// Call Method From MethodID

#define CALL_METHOD_FROM_JMETHODID(env, objectName, object, method, methodID, ...) \
env->method(object, methodID __VA_OPT__(,) __VA_ARGS__); \
LOG_JNI_FUNCTION(env, objectName)

#define CALL_METHOD_FROM_JMETHODID_WITHOUT_LOG(env, object, method, methodID, ...) \
env->method(object, methodID __VA_OPT__(,) __VA_ARGS__); \

// Get Field

#define GET_JFIELD(env, objectName, object, clazz, fieldName, sig, type) \
jfieldID GET_JFIELDID(env, objectName##_FieldID, clazz, fieldName, sig); \
type GET_JFIELD_FROM_JFIELDID(env, objectName, object, objectName##_FieldID)

#define GET_STATIC_JFIELD(env, objectName, clazz, fieldName, sig, type) \
jfieldID GET_STATIC_JFIELDID(env, objectName##_FieldID, clazz, fieldName, sig); \
type GET_STATIC_JFIELD_FROM_JFIELDID(env, objectName, clazz, objectName##_FieldID)

// Get FieldID

#define GET_JFIELDID(env, fieldIDName, clazz, fieldName, sig) \
fieldIDName = env->GetFieldID(clazz, fieldName, sig); \
LOG_JNI_FUNCTION(env, fieldIDName)

#define GET_STATIC_JFIELDID(env, fieldIDName, clazz, fieldName, sig) \
fieldIDName = env->GetStaticFieldID(clazz, fieldName, sig); \
LOG_JNI_FUNCTION(env, fieldIDName)

// Get Field From FieldID

#define GET_JFIELD_FROM_JFIELDID(env, objectName, object, fieldID) \
objectName = env->GetObjectField(object, fieldID); \
LOG_JNI_FUNCTION(env, objectName)

#define GET_STATIC_JFIELD_FROM_JFIELDID(env, objectName, clazz, fieldID) \
objectName = env->GetStaticObjectField(clazz, fieldID); \
LOG_JNI_FUNCTION(env, objectName)

// Create Global Object
#define CREATE_GLOBAL_JOBJECT(env, objectName, object, type) \
type objectName = env->NewGlobalRef(object); \
LOG_JNI_FUNCTION(env, objectName)