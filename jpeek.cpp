#include <jvmti.h>

#include <iostream>
#include <cstring>

using namespace std;


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
    char *sig = NULL;
    char *gen = NULL;
    res = jvmti->GetClassSignature(classes[i], &sig, &gen);
    if (res != JNI_OK) { cerr << "  Error getting class signature!\n"; continue; }
    cerr << "  " << sig << endl;
    if (sig)
       jvmti->Deallocate((unsigned char *)sig);
    if (gen)
       jvmti->Deallocate((unsigned char *)gen);
  }
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
  res = jvmti->SetEventCallbacks(callbacks, sizeof(jvmtiEventCallbacks));
  if (res != JNI_OK) { cerr << "Couldn't set JVM TI callbacks!" << endl; return -3; }
  res = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_VM_INIT, nullptr);
  if (res != JNI_OK) { cerr << "Couldn't enable JVMTI_EVENT_VM_INIT!" << endl; return -4; }

  return JNI_OK;
}

JNIEXPORT void JNICALL
Agent_OnUnload(JavaVM *jvm)
{
  cerr << "JPeek agent unloaded\n";
}

