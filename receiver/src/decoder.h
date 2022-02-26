#pragma once
#include "RingBuf.h"

namespace Manchester
{
    using ResultBuffer = RingBuf<char, 65000>;     // 1000 chars

    class Decoder //: public Print
    {
     public:
        void begin();
        void tick();
        int read();
        int read(char* buf, size_t size);

     protected:
        ResultBuffer resultBuffer;

        void decode(uint32_t edge);

        struct // used internally to store bits until a full byte is received
        {
            byte buffer;
            uint8_t pos;
        } bits;

        enum states {
            SYNCING,
            SYNCED,
        } state = states::SYNCING;
    };

}
