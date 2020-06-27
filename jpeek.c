#include <jvmti.h>

#include "stdio.h"

JNIEXPORT jint JNICALL
Agent_OnLoad(JavaVM *jvm, char *options, void *reserved)
{
  fprintf(stderr, "JPeek agent loaded\n");
  return 0; // no error on agent load
}
