#include "Arduino.h"
#include "ringbuf.h"
constexpr unsigned dataPin  = 0;
constexpr unsigned clockPin = 1; // clock output not necessary but useful for debugging

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

    size_t write(const uint8_t* buffer, size_t size) override
    {
        constexpr unsigned halfPeriod = 500; //ns
        digitalWriteFast(dataPin, HIGH);
        delayNanoseconds(7*halfPeriod);
        digitalWriteFast(dataPin, LOW);
        delayNanoseconds(halfPeriod);

        for (unsigned byteCnt = 0; byteCnt < size; byteCnt++) // loop over chars in buffer
        {

            uint8_t current      = buffer[byteCnt];
            for (int bitCnt = 0; bitCnt < 8; bitCnt++)  // loop over bits in current char
            {                                           //
                 uint8_t bit = (current & 0x80) != 0x80; // get bit
                 current <<= 1;                          // prepare next bit

                digitalWriteFast(clockPin, HIGH);        // clock output not necessary but useful for debugging
                digitalWriteFast(dataPin, (bit ^ HIGH)); // data XOR clock
                delayNanoseconds(halfPeriod);                  // we want a ~1MHz clock
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
}

uint16_t i = 0;

void loop()
{
    mc.printf("*%d*\n", i++);
    delayMicroseconds(10);
}

