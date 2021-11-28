#pragma once

#include "modloader\shared\modloader.hpp"

#include <android/log.h>
#include <string>

// I fucking hate everything

// Log Function

// Code From User "hmjd" on Stack OverFlow. Sauce: https://stackoverflow.com/questions/10408972/how-to-obtain-a-description-of-a-java-exception-in-c-when-using-jni
void _append_exception_trace_messages(
						JNIEnv*      a_jni_env,
						std::string& a_error_msg,
						jthrowable   a_exception,
						jmethodID    a_mid_throwable_getCause,
						jmethodID    a_mid_throwable_getStackTrace,
						jmethodID    a_mid_throwable_toString,
						jmethodID    a_mid_frame_toString)
{
	// Get the array of StackTraceElements.
	jobjectArray frames =
		(jobjectArray) a_jni_env->CallObjectMethod(
										a_exception,
										a_mid_throwable_getStackTrace);
	jsize frames_length = a_jni_env->GetArrayLength(frames);

	// Add Throwable.toString() before descending
	// stack trace messages.
	if (0 != frames)
	{
		jstring msg_obj =
			(jstring) a_jni_env->CallObjectMethod(a_exception,
												a_mid_throwable_toString);
		const char* msg_str = a_jni_env->GetStringUTFChars(msg_obj, 0);

		// If this is not the top-of-the-trace then
		// this is a cause.
		if (!a_error_msg.empty())
		{
			a_error_msg += "\nCaused by: ";
			a_error_msg += msg_str;
		}
		else
		{
			a_error_msg = msg_str;
		}

		a_jni_env->ReleaseStringUTFChars(msg_obj, msg_str);
		a_jni_env->DeleteLocalRef(msg_obj);
	}

	// Append stack trace messages if there are any.
	if (frames_length > 0)
	{
		jsize i = 0;
		for (i = 0; i < frames_length; i++)
		{
			// Get the string returned from the 'toString()'
			// method of the next frame and append it to
			// the error message.
			jobject frame = a_jni_env->GetObjectArrayElement(frames, i);
			jstring msg_obj =
				(jstring) a_jni_env->CallObjectMethod(frame,
													a_mid_frame_toString);

			const char* msg_str = a_jni_env->GetStringUTFChars(msg_obj, 0);

			a_error_msg += "\n    ";
			a_error_msg += msg_str;

			a_jni_env->ReleaseStringUTFChars(msg_obj, msg_str);
			a_jni_env->DeleteLocalRef(msg_obj);
			a_jni_env->DeleteLocalRef(frame);
		}
	}

	// If 'a_exception' has a cause then append the
	// stack trace messages from the cause.
	if (0 != frames)
	{
		jthrowable cause = 
			(jthrowable) a_jni_env->CallObjectMethod(
							a_exception,
							a_mid_throwable_getCause);
		if (0 != cause)
		{
			_append_exception_trace_messages(a_jni_env,
											a_error_msg, 
											cause,
											a_mid_throwable_getCause,
											a_mid_throwable_getStackTrace,
											a_mid_throwable_toString,
											a_mid_frame_toString);
		}
	}
}

std::string LogJNIThrowable(JNIEnv* env, jthrowable exc) {
    jclass throwableClass = env->FindClass("java/lang/Throwable");
	jmethodID midThrowableGetCause = env->GetMethodID(throwableClass, "getCause", "()Ljava/lang/Throwable;");
	jmethodID midThrowableGetStackTrace = env->GetMethodID(throwableClass, "getStackTrace", "()[Ljava/lang/StackTraceElement;");
	jmethodID midThrowableToString = env->GetMethodID(throwableClass, "toString", "()Ljava/lang/String;");

	jclass frameClass = env->FindClass("java/lang/StackTraceElement");
	jmethodID midFrameToString = env->GetMethodID(frameClass, "toString", "()Ljava/lang/String;");

	std::string errorMsg;

	_append_exception_trace_messages(env, errorMsg, exc, midThrowableGetCause, midThrowableGetStackTrace, midThrowableToString, midThrowableToString);
	return errorMsg;
}

#define LOG_ERRORS(env) { \
	jthrowable exc = env->ExceptionOccurred(); \
	env->ExceptionClear(); \
	if (exc) { __android_log_print(ANDROID_LOG_VERBOSE, "HotSwappableMods", "-- Logging Exception --\n%s\n-- Finished Logging Exception --", LogJNIThrowable(env, exc).c_str()); } \
}

#define LOG_JNI_FUNCTION(env, objectToTest) \
if (objectToTest == nullptr) { __android_log_print(ANDROID_LOG_ERROR, "HotSwappableMods", "Failed to get \"%s\"", ""#objectToTest); } \
else { __android_log_print(ANDROID_LOG_VERBOSE, "HotSwappableMods", "Got \"%s\"", ""#objectToTest); } \
LOG_ERRORS(env)


// This is used to pass literally nothing to a parameter
#define NOTHING

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
LOG_ERRORS(env)

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

// Create jstring from const char*
// Code from user "mpolak" on GitHub. Sauce: https://gist.github.com/mpolak/1274759
jobject CreateJString(JNIEnv* env, const char* str)
{
	GET_JCLASS(env, stringClass, "java/lang/String", jclass);

	jbyteArray bytes = 0;
	int len;
	if (env->EnsureLocalCapacity(2) < 0) {
		return NULL; /* out of memory error */
	}
	len = strlen(str);
	bytes = env->NewByteArray(len);
	if (bytes != NULL) {
		env->SetByteArrayRegion(bytes, 0, len, (jbyte *)str);
		NEW_JOBJECT(env, result, stringClass, "([B)V", jobject, bytes);
		env->DeleteLocalRef(bytes);
		return result;
	} /* else fall through */
	return NULL;
}