#!/bin/sh

echo
echo Java with agent:
java -agentpath:./libjpeek.so JPeekDemo "Another World"

echo
echo Java without agent:
java JPeekDemo "Another World"
