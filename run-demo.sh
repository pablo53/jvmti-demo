#!/bin/sh

echo Java without agent:
java JPeekDemo "Another World"

echo Java with agent:
java -agentpath:./libjpeek.so JPeekDemo "Another World"
