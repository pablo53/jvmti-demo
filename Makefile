CC=gcc
CFLAGS=-Wall
OS=linux
LIBDIR_JVMTI=/usr/lib/jvm/default-java/include
LIBS=-I$(LIBDIR_JVMTI) -I$(LIBDIR_JVMTI)/$(OS)

JAVAC=javac

.PHONY: all clean

all: libjpeek.so JPeekDemo.class

libjpeek.so: jpeek.o
	$(CC) -shared -o $@ $^

jpeek.o: jpeek.c
	$(CC) -c $(CFLAGS) $(LIBS) -fPIC $< -o $@

JPeekDemo.class: JPeekDemo.java
	$(JAVAC) $<

clean:
	rm -f *.o *.so *.class
