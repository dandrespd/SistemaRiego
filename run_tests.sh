#!/bin/bash

cd test || exit 1
g++ -I../lib/unity -I../firmware/include -I. test_rtc.cpp test_runner.h -o test_runner -lstdc++
if [ $? -ne 0 ]; then
  echo "Compilation failed"
  exit 1
fi

./test_runner
