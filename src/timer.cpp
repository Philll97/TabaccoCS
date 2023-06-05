#include "timer.h"

bool state = true;
long start_time;
long duration;

void timer::start(long dur)
{
    start_time = millis();
    duration = dur;
    state = false;
}

bool timer::check()
{
    if(start_time + duration < millis())
        state = true;

    return state;
}