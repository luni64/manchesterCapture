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

    class Decoder //: public Print
    {
     public:
        void begin(float baudrate);
        void tick();
        ResultBuffer resultBuffer;

     protected:

        uint32_t period;

        uint32_t bitCnt;

        frame_t frame0, frame1;
        frame_t* currentFrame;
        bool needFrame0;

        bool decode(uint32_t event, uint32_t* f0, uint32_t* f1);

        // struct // used internally to store bits until a full byte is received
        // {
        //     byte buffer;
        //     uint8_t pos;
        // } bits;

        TS5643Field received;

        enum states {
            SYNCING,
            SYNCED,
        } state = states::SYNCING;
    };

}
