#include "Arduino.h"
#include "ringbuf.h"
constexpr unsigned dataPin  = 0;
constexpr unsigned clockPin = 1; // clock output not necessary but useful for debugging

union frame1
{
    struct
    {
        uint32_t modemAddress : 2;
        uint32_t cntr_0_to_14 : 15;
        uint32_t frameAddress : 1;
        uint32_t frameCRC : 3;
        uint32_t stopBit : 1;
    } bits;
    uint32_t data;
};

union frame2
{
    struct
    {
        uint32_t modemAddress : 2;
        uint32_t cntr_15_to_23 : 9;
        uint32_t BE : 1;
        uint32_t OF : 1;
        uint32_t OS : 1;
        uint32_t BA : 1;
        uint32_t PS : 1;
        uint32_t CE : 1;
        uint32_t frameAddress : 1;
        uint32_t frameCRC : 3;
        uint32_t stopBit : 1;

    } bits;
    uint32_t data;
};

uint16_t bitreverse(uint16_t source)
{
    uint16_t target = 0;
    for (int i = 0; i < 16; i++)
    {
        bool b = source & (1 << i);
        target |= b << (15 - i);
    }
    return target;
}

class Manchester : public Stream
{
 public:
    Manchester(unsigned pin, unsigned clockFreq){};

    int available() { return 1; }
    int read() { return 42; };
    int peek() { return 42; };
    size_t write(uint8_t b) override
    {
        Serial.println("  write single");
        // Serial.write(b);
        return 1;
    };

    void writeStartbit(float T)
    {
        digitalWriteFast(dataPin, HIGH);
        delayNanoseconds(5.25 * T);
        digitalWriteFast(dataPin, LOW);
        delayNanoseconds(T); // total (5.25 +1.00) * 500 = 3125ns
    }

    void writeFrame(uint32_t frame, float T)
    {
        noInterrupts();
        for (unsigned i = 0; i < 22; i++) // loop over bits in Frame
        {
            uint8_t bit = frame & 1;

            digitalWriteFast(clockPin, HIGH);        // clock output not necessary but useful for debugging
            digitalWriteFast(dataPin, (bit ^ LOW));  // data XOR clock
            delayNanoseconds(T - 0);                // we want a ~1MHz clock
                                                     //
            digitalWriteFast(clockPin, LOW);         // clock
            digitalWriteFast(dataPin, (bit ^ HIGH)); // data XOR clock
            delayNanoseconds(T - 0);

            frame >>= 1; // prepare next bit
        }
        interrupts();
        digitalWriteFast(dataPin, LOW);
        delayMicroseconds(17);
    }

    void writeEnc(uint32_t cnt, bool BE, bool OF, bool OS, bool BA, bool PS, bool CE)
    {
        frame1 f1;
        f1.bits.modemAddress = 0;
        f1.bits.cntr_0_to_14 = cnt;
        f1.bits.frameAddress = 0;
        f1.bits.frameCRC     = 7;
        f1.bits.stopBit      = 0;

        frame2 f2;
        f2.bits.modemAddress  = 0;
        f2.bits.cntr_15_to_23 = cnt >> 15;
        f2.bits.frameAddress  = 1;
        f2.bits.BE            = BE;
        f2.bits.OF            = OF;
        f2.bits.OS            = OS;
        f2.bits.BA            = BA;
        f2.bits.PS            = PS;
        f2.bits.CE            = CE;
        f2.bits.frameCRC      = 5;
        f2.bits.stopBit       = 0;
        f2.bits.frameCRC      = 5;
        f2.bits.stopBit       = 0;

        writeStartbit(500);
        writeFrame(f1.data, 500);

        writeStartbit(500);
        writeFrame(f2.data, 500);
//        Serial.printf("cnt: %d, 0: %d  1: %d\n",cnt,f1.bits.cntr_0_to_14, f2.bits.cntr_15_to_23);
    }

    size_t write(const uint8_t* buffer, size_t size) override
    {
        constexpr unsigned halfPeriod = 500; // ns
        digitalWriteFast(dataPin, HIGH);
        delayNanoseconds(2625);
        digitalWriteFast(dataPin, LOW);
        delayNanoseconds(halfPeriod); // total of 2.625+0.500=3.125µs

        for (unsigned byteCnt = 0; byteCnt < size; byteCnt++) // loop over chars in buffer
        {

            uint8_t current = buffer[byteCnt];
            for (int bitCnt = 0; bitCnt < 8; bitCnt++)  // loop over bits in current char
            {                                           //
                uint8_t bit = (current & 0x80) != 0x80; // get bit
                current <<= 1;                          // prepare next bit

                digitalWriteFast(clockPin, HIGH);        // clock output not necessary but useful for debugging
                digitalWriteFast(dataPin, (bit ^ HIGH)); // data XOR clock
                delayNanoseconds(halfPeriod);            // we want a ~1MHz clock
                                                         //
                digitalWriteFast(clockPin, LOW);         // clock
                digitalWriteFast(dataPin, (bit ^ LOW));  // data XOR clock
                delayNanoseconds(halfPeriod);
            }
        }
        digitalWriteFast(dataPin, LOW);
        return size;
    }
};

Manchester mc(11, 100'000);

RingBuf<byte, 1024> sendBuf;

void setup()
{
    while (!Serial) {}
    Serial.println("start");

    pinMode(dataPin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    digitalWriteFast(clockPin, LOW);

    // Serial.println(0b1'0100'1110);
    // Serial.println(bitreverse(0b1'0100'1110)>>7);
    // while(1)
        ;

    // mc.writeEnc(0x4, LOW, HIGH, HIGH, LOW, HIGH, LOW);
}

uint32_t i = 0;

void loop()
{
    mc.writeEnc(i++, LOW, HIGH, HIGH, LOW, HIGH, LOW);
    // delayMicroseconds(10);
}
