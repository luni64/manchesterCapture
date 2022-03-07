#pragma once

#include "RingBuf.h"

//using EdgeBuffer = RingBuf<uint32_t, 50000>; // LOW WORD holds ticks since last edge, HIGH WORD holds edge (rising/falling)

class EdgeProvider
{
 public:
    static void init();
    static RingBuffer buff;

 protected:
    static uint16_t oldCap;
    static void onCapture();
    static constexpr IMXRT_TMR_CH_t* ch = &IMXRT_TMR1.CH[2]; // TMR1 channel 2 -> input pin 11,
};
