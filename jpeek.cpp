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
MethodExit(jvmtiEnv *jvmti, JNIEnv *jni, jthread thread, jmethodID method, jboolean byException, jvalue retVal)
{
  char *name = nullptr;
  char *sig = nullptr;
  char *gen = nullptr;
  
  jvmti->GetMethodName(method, &name, &sig, &gen);
  cerr << "Returned from method: " << name << "(" << sig << ")";
  if (byException)
    cerr << " with Exception";
  cerr << endl;
  DEALLOCATE(name);
  DEALLOCATE(sig);
  DEALLOCATE(gen);
}

void JNICALL
ThreadStart(jvmtiEnv *jvmti, JNIEnv * jni, jthread thread)
{
  jvmtiError          err;
  jvmtiThreadInfo     threadInfo;

  cerr << "Thread started: ";
  err = jvmti->GetThreadInfo(thread, &threadInfo);
  if (err) { cerr << "Error reading thread info!" << endl; return; }
  cerr << threadInfo.name << endl;
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
  jvmtiError          err;
  jvmtiEventCallbacks *callbacks;
  jvmtiCapabilities   capabilities;

  cerr << "JPeek agent loaded\n";

  res = jvm->GetEnv((void**)&jvmti, JVMTI_VERSION_1_0);
  if (res != 0) { cerr << "Error getting JVM environment!\n"; return -1; }

  err = jvmti->GetCapabilities(&capabilities);
  if (err != JVMTI_ERROR_NONE) { cerr << "Error getting capabilities!" << endl; return -5; }
  capabilities.can_generate_method_entry_events = true;
  capabilities.can_generate_method_exit_events = true;
  err = jvmti->AddCapabilities(&capabilities);
  if (err != JVMTI_ERROR_NONE) { cerr << "Error setting capabilities!" << endl; return -6; }

  callbacks = (jvmtiEventCallbacks*)calloc(1, sizeof(jvmtiEventCallbacks));
  if (!callbacks) { cerr << "Couldn't allocate memory for JVM TI callbacks!" << endl; return -2; }
  callbacks->VMInit = &VMInit;
  callbacks->VMDeath = &VMDeath;
  callbacks->ThreadStart = &ThreadStart;
  callbacks->ThreadEnd = &ThreadEnd;
  callbacks->MethodEntry = &MethodEntry;
  callbacks->MethodExit = &MethodExit;
  res = jvmti->SetEventCallbacks(callbacks, sizeof(jvmtiEventCallbacks));
  if (res != JNI_OK) { cerr << "Couldn't set JVM TI callbacks!" << endl; return -3; }
  res = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_VM_INIT, nullptr);
  res = res || jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_VM_DEATH, nullptr);
  res = res || jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_THREAD_START, nullptr);
  res = res || jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_THREAD_END, nullptr);
  res = res || jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_METHOD_ENTRY, nullptr);
  res = res || jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_METHOD_EXIT, nullptr);
  if (res != JNI_OK) { cerr << "Couldn't enable events!" << endl; return -4; }

  return JNI_OK;
}

JNIEXPORT void JNICALL
Agent_OnUnload(JavaVM *jvm)
{
  cerr << "JPeek agent unloaded\n";
}

