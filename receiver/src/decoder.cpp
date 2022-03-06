#include "decoder.h"
#include "edgeprovider.h"

namespace Manchester
{

    void Decoder::begin(float baudrate)
    {
        period = 1E9 / baudrate;
        resultBuffer.clear();
        EdgeProvider::init();
        state        = states::SYNCING;
        needFrame0   = true;
        currentFrame = &frame0;
    }

    void Decoder::tick()
    {
        static uint32_t oldCount = 0;

        while (EdgeProvider::buff.hasElements())
        {
            uint32_t edge = EdgeProvider::buff.pop();           // pop an edge from the buffer...

            uint32_t  payload_0, payload_1;
            if (decode(edge, &payload_0, &payload_1)) // ...and decode it. if decoder return true, the payloads of the first and second frame are set
            {
                TS5643Field field;
                field.count = (payload_0 & 0x7FFF) | ((payload_1 & 0x1FF) << 15); // bit 0-14 in frame0 bit 15-23 in frame1
                field.BE_OS = payload_1 & 1 << 9;
                field.OF    = payload_1 & 1 << 10;
                field.OS    = payload_1 & 1 << 11;
                field.BA    = payload_1 & 1 << 12;
                field.PS    = payload_1 & 1 << 13;
                field.CE    = payload_1 & 1 << 14;
                field.valid = true;

                if (field.count != oldCount + 1)
                {
                    digitalWriteFast(10, HIGH);
               //     Serial.printf("err pl0: %d pl1:%d\n", payload_0, payload_1);
                }


                resultBuffer.push(field);

                digitalWriteFast(10, LOW);

                oldCount = field.count;
            }
        }
    }

    bool Decoder::decode(uint32_t event, uint32_t* payload_0, uint32_t* payload_1)
    {
        constexpr uint16_t T       = 500;        // timing of a half bit
        constexpr uint16_t syncMin = 2625 - 100; // min duration of sync pulse
        constexpr uint16_t syncMax = 2625 + 100; // max duration of sync pulse
        static bool first          = true;       // keep track of the 2T timing

        uint16_t dt   = (event & 0xFFFF) * 8.0f * 1E9f / F_BUS_ACTUAL; // time since last edge
        bool edge = event >> 16;                                   // 0 -> down, 1 -> up

        //Serial.printf("%d:%d ",edge, event);

        switch (state)
        {
            case SYNCING:
                if (dt > syncMin && dt < syncMax)
                {
                    currentFrame->data = 0;
                    bitCnt             = 0;
                    first              = true;

                    state = SYNCED;
                  //  Serial.println("Sync");
                }

                break;

            case SYNCED:
                if (dt == 0) break;

                if (dt > T * 3.5 || dt < T * 0.5) // timing to much off, probably a break, need to resync
                {
                    Serial.printf("bad %d\n", dt);
                    state = SYNCING;
                    break;
                }

                // decode
                if (dt > T * 1.5) // dt = 2T
                {
                    currentFrame->data |= (!edge << bitCnt++);
                }
                else // dt = T
                {
                    if (!first)
                    {
                        currentFrame->data |= (!edge << bitCnt++);
                    }
                    first = !first;
                }

                if (bitCnt > 21) // got complete frame
                {
                    if (currentFrame == &frame0 && currentFrame->bits.frameAddress == 0)
                    {
                        currentFrame = &frame1;
                    }
                    else if (currentFrame == &frame1 && currentFrame->bits.frameAddress == 1)
                    {
                        currentFrame = &frame0;
                        *payload_0   = frame0.bits.payload;
                        *payload_1   = frame1.bits.payload;
                        state        = states::SYNCING;
                        return true;
                    }
                    else
                    {
                        currentFrame = &frame0;
                        Serial.println("framesync");
                    }
                    state = states::SYNCING;
                }
                break;

            default:
                break;
        }
        return false;
    }
}