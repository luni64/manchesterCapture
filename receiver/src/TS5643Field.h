#pragma once
#include "Arduino.h"

struct TS5643Field
{
    uint32_t count; // counter value
    bool BE_OS;     // Battery error | Overspeed (ored)
    bool OF;        // Overflow
    bool OS;        // Overspeed
    bool BA;        // Battery alarm
    bool PS;        // Preload status
    bool CE;        // Count Error
    bool valid;

    TS5643Field();
    void clear();
    size_t printTo(Print& p) const;
};