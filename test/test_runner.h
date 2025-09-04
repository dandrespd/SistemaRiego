#ifndef TEST_RUNNER_H
#define TEST_RUNNER_H

#include "arduino_minimal.h"
#include "../../lib/unity/src/unity.h"

// Minimal Arduino functions for native testing

// Entry point for native tests
int main(int argc, char **argv) {
    UNITY_BEGIN();
    setup();
    loop();
    UNITY_END();
    return 0;
}

#endif
