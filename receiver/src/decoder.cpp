#include "decoder.h"
#include "edgeprovider.h"

namespace Manchester
{
    void Decoder::begin()
    {
        resultBuffer.clear();
        EdgeProvider::init();
    }

    int Decoder::read()
    {
        if (!resultBuffer.isEmpty())
        {
            char result;
            resultBuffer.lockedPop(result);
            return result;
        }
        else
            return -1;
    }

    int Decoder::read(char* buf, size_t bufSize)
    {
        size_t maxRead = min(bufSize, resultBuffer.size());
        for (int i = 0; i < maxRead; i++)
        {
           // buf[i] = resultBuffer[i];
             char c;
             resultBuffer.pop(c);
             buf[i] = c;
        }
        return maxRead;
    }

    void Decoder::tick()
    {
        while (!EdgeProvider::buffer.isEmpty())
        {
            uint32_t edge;
            EdgeProvider::buffer.lockedPop(edge); // pop an edge from the buffer..
            decode(edge);               // and decode it
        }
    }

    void Decoder::decode(uint32_t event)
    {
        constexpr uint16_t T = 500; // timing of a half bit
        static bool first    = true; // keep track of the 2T timing

        uint16_t dt   = (event & 0xFFFF) * 8.0f * 1E9f / F_BUS_ACTUAL; // time since last edge
        uint16_t edge = event >> 16;                                   // 0 -> down, 1 -> up

        switch (state)
        {
            case SYNCING:
                if (dt > 3400 && dt < 3600)
                {
                    bits.buffer = 0x0; // clear bits buffer
                    bits.pos    = 0;
                    first       = true;

                    state = SYNCED;
                }
                break;

            case SYNCED:
                if (dt > T * 2.5 || dt < T * 0.75) // timing to much off, probably a break, need to resync
                {
                    state = SYNCING;
                    break;
                }

                // decode
                if (dt > T * 1.5) // dt = 2T
                {
                    bits.buffer = bits.buffer | !edge << (7 - bits.pos++); // if falling edge, set bit = 1
                }
                else // dt = T
                {
                    if (!first)
                    {
                        bits.buffer = bits.buffer | !edge << (7 - bits.pos++);
                    }
                    first = !first;
                }

                if (bits.pos > 7)
                {
                    // Serial.printf("%c", bits.buffer);
                    resultBuffer.push(bits.buffer);

                    bits.buffer = 0x0; // clear
                    bits.pos    = 0;
                }

            default:
                break;
        }
    }
}