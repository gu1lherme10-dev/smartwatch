#ifndef ACTIVITY_EVENT_H
#define ACTIVITY_EVENT_H

#include <stdint.h>

struct ActivityEvent {
    uint32_t timestamp;
    uint32_t data;
};

#endif // ACTIVITY_EVENT_H
