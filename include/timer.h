#ifndef TIMER_HEADER
#define TIMER_HEADER


#include <Arduino.h>

namespace timer
{
    void start(long dur);
    bool check();
}
#endif