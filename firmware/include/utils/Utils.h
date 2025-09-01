#ifndef __UTILS_H__
#define __UTILS_H__
#include <Arduino.h>

inline String repeatChar(char c, int times) {
    String s = "";
    for (int i = 0; i < times; ++i) s += c;
    return s;
}

#endif // __UTILS_H__
