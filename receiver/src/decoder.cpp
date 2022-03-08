#include "decoder.h"
#include "edgeproviderDMA.h"

namespace Manchester
{
    void Decoder::begin(float baudrate)
    {
        resultBuffer.clear();
        EdgeProviderDMA::init();
        state        = states::SYNCING;
        currentFrame = &frame0;
        initialized  = true;
    }

    void Decoder::tick()
    {
        if (!initialized) return;

        //static uint32_t oldCount     = 0;
        static uint16_t oldTimestamp = 0;

        while (EdgeProviderDMA::hasElements())
        {
            uint32_t payload_0, payload_1;
            uint16_t timestamp = EdgeProviderDMA::popTimestamp();
            uint16_t dt        = timestamp - oldTimestamp;
            oldTimestamp       = timestamp;

            if (decode(dt, &payload_0, &payload_1)) // If decoder returns true, the payloads of the first and second frame are valid
            {
                TS5643Field field;
                field.count = (payload_0 & 0x7FFF) | ((payload_1 & 0x1FF) << 15); // bit 0-14 in frame0 bit 15-23 in frame1
                field.BE_OS = payload_1 & 1 << 9;
                field.OF    = payload_1 & 1 << 10;
                field.OS    = payload_1 & 1 << 11;
                field.BA    = payload_1 & 1 << 12;
                field.PS    = payload_1 & 1 << 13;
                field.CE    = payload_1 & 1 << 14;

                // if (field.count != oldCount + 1)  // error detection
                // {
                //     digitalWriteFast(10, HIGH);
                // }
                // oldCount = field.count;
                resultBuffer.push(field);
                // digitalWriteFast(10, LOW);
            }
        }
    }

    bool Decoder::decode(uint16_t delta_t, uint32_t* payload_0, uint32_t* payload_1)
    {
        constexpr uint16_t T       = 500;                   // timing of a half bit
        constexpr uint16_t syncMin = 2625 - 100;            // min duration of sync pulse
        constexpr uint16_t syncMax = 2625 + 100;            // max duration of sync pulse
        static bool first          = true;                  // keep track of the 2T timing
                                                            //
        uint16_t dt = delta_t * 8.0f * 1E9f / F_BUS_ACTUAL; // time in ns since last timestamp
        edge        = !edge;                                // we detect both edges -> direction will necessarily toggle at each event

        switch (state)
        {
            case SYNCING:
                if (dt > syncMin && dt < syncMax)
                {
                    currentFrame->data = 0;
                    bitCnt             = 0;
                    first              = true;

                    state = SYNCED;
                    // Serial.println("Sync");
                    edge = 0;
                }
                break;

            case SYNCED:
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
                        digitalWriteFast(8, LOW); //
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
        digitalWriteFast(8, LOW); //
        return false;
    }
}