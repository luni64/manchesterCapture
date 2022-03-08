#pragma once
#include "RingBuf.h"
#include "TS5643Field.h"

namespace Manchester
{
    union frame_t
    {
        struct
        {
            uint32_t modemAddress : 2;
            uint32_t payload : 15;
            uint32_t frameAddress : 1;
            uint32_t frameCRC : 3;
        } bits;
        uint32_t data;
    };

    using ResultBuffer = RingBuf<TS5643Field, 10000>; // Keep results

    class Decoder
    {
     public:
        void begin(float baudrate);
        void tick();
        ResultBuffer resultBuffer;

     protected:
        bool decode(uint16_t dt, uint32_t* f0, uint32_t* f1);
        bool initialized = false;

        uint32_t bitCnt;
        frame_t frame0, frame1;
        frame_t* currentFrame;
        TS5643Field received;
        bool edge;

        enum states {
            SYNCING,
            SYNCED,
        } state = states::SYNCING;
    };
}
