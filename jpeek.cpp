#include <jvmti.h>

#include <iostream>
#include <cstring>

using namespace std;

#define DEALLOCATE(s)                      \
{                                          \
 if (s)                                    \
    jvmti->Deallocate((unsigned char *)s); \
}                                          \

void JNICALL
VMInit(jvmtiEnv *jvmti, JNIEnv *jni, jthread thread)
{
  jvmtiError err;
  jint       class_count;
  jclass     *classes;
  jint       res;

  cerr << "JPeek initialized." << endl;

  err = jvmti->GetLoadedClasses(&class_count, &classes);
  if (err != JVMTI_ERROR_NONE) { cerr << "Error getting loaded classes!\n"; return; }
  cerr << "Number of classes loaded: " << class_count << endl;
  for (int i = 0; i < class_count; i++)
  {
    char *sig = nullptr;
    char *gen = nullptr;
    res = jvmti->GetClassSignature(classes[i], &sig, &gen);
    if (res != JNI_OK) { cerr << "  Error getting class signature!\n"; continue; }
    cerr << "  " << sig << endl;
    DEALLOCATE(sig)
    DEALLOCATE(gen)
  }
}

void JNICALL
MethodEntry(jvmtiEnv *jvmti, JNIEnv *jni, jthread thread, jmethodID method)
{
  char *name = nullptr;
  char *sig = nullptr;
  char *gen = nullptr;
  
  jvmti->GetMethodName(method, &name, &sig, &gen);
  cerr << "Entered method: " << name << "(" << sig << ")" << endl;
  DEALLOCATE(name);
  DEALLOCATE(sig);
  DEALLOCATE(gen);
}

void JNICALL
ThreadStart(jvmtiEnv *jvmti, JNIEnv * jni, jthread thread)
{
  jvmtiError          err;
  jvmtiThreadInfo     threadInfo;
  jvmtiEventCallbacks *callbacks;
  jint                res;

  cerr << "Thread started: ";
  err = jvmti->GetThreadInfo(thread, &threadInfo);
  if (err) { cerr << "Error reading thread info!" << endl; return; }
  cerr << threadInfo.name << endl;

  callbacks = (jvmtiEventCallbacks*)calloc(1, sizeof(jvmtiEventCallbacks));
  if (!callbacks) { cerr << "Couldn't allocate memory for JVM TI callbacks in thread " << threadInfo.name << "!" << endl; return; }
  res = jvmti->SetEventCallbacks(callbacks, sizeof(jvmtiEventCallbacks));
  if (res != JNI_OK) { cerr << "Couldn't set JVM TI callbacks in thread " << threadInfo.name << "!" << endl; return; }
  callbacks->MethodEntry = &MethodEntry;
  res = res || jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_METHOD_ENTRY, thread);
  if (res != JNI_OK) { cerr << "Couldn't enable events in thread " << threadInfo.name << "!" << endl; return; }
}

void JNICALL
ThreadEnd(jvmtiEnv *jvmti, JNIEnv * jni, jthread thread)
{
  jvmtiError err;
  jvmtiThreadInfo threadInfo;

  cerr << "Thread ended: ";
  err = jvmti->GetThreadInfo(thread, &threadInfo);
  if (err) { cerr << "Error reading thread info!" << endl; return; }
  cerr << threadInfo.name << endl;
}

void JNICALL
VMDeath(jvmtiEnv *jvmti, JNIEnv* jni)
{
  cerr << "JPeek died." << endl;
}


JNIEXPORT jint JNICALL
Agent_OnLoad(JavaVM *jvm, char *options, void *reserved)
{
  jvmtiEnv            *jvmti;
  jint                res;
  jvmtiEventCallbacks *callbacks;

  cerr << "JPeek agent loaded\n";

  res = jvm->GetEnv((void**)&jvmti, JVMTI_VERSION_1_0);
  if (res != 0) { cerr << "Error getting JVM environment!\n"; return -1; }

  callbacks = (jvmtiEventCallbacks*)calloc(1, sizeof(jvmtiEventCallbacks));
  if (!callbacks) { cerr << "Couldn't allocate memory for JVM TI callbacks!" << endl; return -2; }
  callbacks->VMInit = &VMInit;
  callbacks->VMDeath = &VMDeath;
  callbacks->ThreadStart = &ThreadStart;
  callbacks->ThreadEnd = &ThreadEnd;
//  callbacks->MethodEntry = &MethodEntry;
  res = jvmti->SetEventCallbacks(callbacks, sizeof(jvmtiEventCallbacks));
  if (res != JNI_OK) { cerr << "Couldn't set JVM TI callbacks!" << endl; return -3; }
  res = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_VM_INIT, nullptr);
  res = res || jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_VM_DEATH, nullptr);
  res = res || jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_THREAD_START, nullptr);
  res = res || jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_THREAD_END, nullptr);
//  res = res || jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_METHOD_ENTRY, nullptr);
  if (res != JNI_OK) { cerr << "Couldn't enable events!" << endl; return -4; }

  return JNI_OK;
}

JNIEXPORT void JNICALL
Agent_OnUnload(JavaVM *jvm)
{
  cerr << "JPeek agent unloaded\n";
}

