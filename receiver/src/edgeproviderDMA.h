#pragma once

#include "DMAChannel.h"

namespace Manchester
{
    class EdgeProviderDMA
    {
     public:
        static void init();

        static uint16_t popTimestamp();
        static bool hasElements();

     protected:
        static uint8_t tail;
        static DMAChannel dmaChannel;
        static constexpr IMXRT_TMR_CH_t* ch = &IMXRT_TMR1.CH[2]; // TMR1 channel 2 -> input pin 11,
    };
}
