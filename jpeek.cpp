#include <jvmti.h>

#include <iostream>
#include <cstring>

using namespace std;

#define DEALLOCATE(s)                      \
{                                          \
 if (s)                                    \
    jvmti->Deallocate((unsigned char *)s); \
}                                          \

inline int printClass(jvmtiEnv *jvmti, jclass klazz)
{
  jvmtiError err;
  char *sig = nullptr;
  char *gen = nullptr;

  err = jvmti->GetClassSignature(klazz, &sig, &gen);
  if (err != JVMTI_ERROR_NONE) { cerr << "ERROR GETTING CLASS SIGNATURE!"; return -1; }
  cerr << sig;
  if (gen)
    cerr << "<" << gen << ">";
  DEALLOCATE(sig)
  DEALLOCATE(gen)

  return 0;
}

inline int printMethod(jvmtiEnv *jvmti, jmethodID method)
{
  jvmtiError err;
  char *name = nullptr;
  char *sig = nullptr;
  char *gen = nullptr;
  
  err = jvmti->GetMethodName(method, &name, &sig, &gen);
  if (err != JVMTI_ERROR_NONE) { cerr << "ERROR GETTING METHOD NAME AND SIGNATURE!"; return -1; }
  cerr << name << " " << sig;
  if (gen)
    cerr << "<" << gen << ">";
  DEALLOCATE(name);
  DEALLOCATE(sig);
  DEALLOCATE(gen);
  
  return 0;
}

inline int printThread(jvmtiEnv *jvmti, jthread thread)
{
  jvmtiError          err;
  jvmtiThreadInfo     threadInfo;

  err = jvmti->GetThreadInfo(thread, &threadInfo);
  if (err != JVMTI_ERROR_NONE) { cerr << "ERROR READING THREAD INFO!" << endl; return -1; }
  cerr << threadInfo.name;

  return 0;
}

void JNICALL
VMInit(jvmtiEnv *jvmti, JNIEnv *jni, jthread thread)
{
  jvmtiError err;
  jint       class_count;
  jclass     *classes;

  cerr << "JPeek initialized." << endl;

  err = jvmti->GetLoadedClasses(&class_count, &classes);
  if (err != JVMTI_ERROR_NONE) { cerr << "ERROR GETTING LOADED CLASSES!\n"; return; }
  cerr << "Number of classes loaded: " << class_count << endl;
  for (int i = 0; i < class_count; i++)
  {
    cerr << "  ";
    printClass(jvmti, classes[i]);
    cerr << endl;
  }
}

void JNICALL
MethodEntry(jvmtiEnv *jvmti, JNIEnv *jni, jthread thread, jmethodID method)
{
  cerr << "[";
  printThread(jvmti, thread);
  cerr << "] ";
  cerr << "Entered method: ";
  printMethod(jvmti, method);
  cerr << endl;
}

void JNICALL
MethodExit(jvmtiEnv *jvmti, JNIEnv *jni, jthread thread, jmethodID method, jboolean byException, jvalue retVal)
{
  jvmtiError err;
  jclass     declaringClass;

  cerr << "[";
  printThread(jvmti, thread);
  cerr << "] ";
  cerr << "Returned from method: ";
  err = jvmti->GetMethodDeclaringClass(method, &declaringClass);
  if (err == JVMTI_ERROR_NONE)
    printClass(jvmti, declaringClass);
  cerr << ".";
  printMethod(jvmti, method);
  if (byException)
    cerr << " with Exception";
  
  cerr << endl;
}

void JNICALL
ThreadStart(jvmtiEnv *jvmti, JNIEnv * jni, jthread thread)
{
  cerr << "Thread started: ";
  printThread(jvmti, thread);
  cerr << endl;
}

void JNICALL
ThreadEnd(jvmtiEnv *jvmti, JNIEnv * jni, jthread thread)
{
  cerr << "Thread ended: ";
  printThread(jvmti, thread);
  cerr << endl;
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
  if (res != 0) { cerr << "ERROR GETTING JVM ENVIRONMENT!\n"; return -1; }

  err = jvmti->GetCapabilities(&capabilities);
  if (err != JVMTI_ERROR_NONE) { cerr << "ERROR GETTING CAPABILITIES!" << endl; return -5; }
  capabilities.can_generate_method_entry_events = true;
  capabilities.can_generate_method_exit_events = true;
  err = jvmti->AddCapabilities(&capabilities);
  if (err != JVMTI_ERROR_NONE) { cerr << "ERROR SETTING CAPABILITIES!" << endl; return -6; }

  callbacks = (jvmtiEventCallbacks*)calloc(1, sizeof(jvmtiEventCallbacks));
  if (!callbacks) { cerr << "ERROR ALLOCATING MEMORY FOR JVM TI CALLBACKS!" << endl; return -2; }
  callbacks->VMInit = &VMInit;
  callbacks->VMDeath = &VMDeath;
  callbacks->ThreadStart = &ThreadStart;
  callbacks->ThreadEnd = &ThreadEnd;
  callbacks->MethodEntry = &MethodEntry;
  callbacks->MethodExit = &MethodExit;
  res = jvmti->SetEventCallbacks(callbacks, sizeof(jvmtiEventCallbacks));
  if (res != JNI_OK) { cerr << "ERROR SETTING JVM TI CALLBACKS!" << endl; return -3; }
  res = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_VM_INIT, nullptr);
  res = res || jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_VM_DEATH, nullptr);
  res = res || jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_THREAD_START, nullptr);
  res = res || jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_THREAD_END, nullptr);
  res = res || jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_METHOD_ENTRY, nullptr);
  res = res || jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_METHOD_EXIT, nullptr);
  if (res != JNI_OK) { cerr << "ERROR ENABLING EVENTS!" << endl; return -4; }

  return JNI_OK;
}

JNIEXPORT void JNICALL
Agent_OnUnload(JavaVM *jvm)
{
  cerr << "JPeek agent unloaded\n";
}

