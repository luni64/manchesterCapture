#include "Arduino.h"

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


class Manchester
{
 public:
    Manchester(unsigned pin, unsigned clockFreq){};

    int available() { return 1; }
    int read() { return 42; };
    int peek() { return 42; };

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
            delayNanoseconds(T - 0);                 // we want a ~1MHz clock
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
        f1.bits.frameCRC     = 7;  // some random crc
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
        f2.bits.frameCRC      = 5;    // some random crc
        f2.bits.stopBit       = 0;
        f2.bits.frameCRC      = 5;
        f2.bits.stopBit       = 0;

        writeStartbit(500);
        writeFrame(f1.data, 500);

        writeStartbit(500);
        writeFrame(f2.data, 500);
    }
};

//---------------------------------------------------

Manchester mc(11, 100'000);

void setup()
{
    while (!Serial) {}
    Serial.println("start");

    pinMode(dataPin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    digitalWriteFast(clockPin, LOW);
}

void loop()
{
    static uint32_t count = 0;

    //           cnt     BE    OF    OS,  BA    PS   CE
    mc.writeEnc(count++, LOW, HIGH, HIGH, LOW, HIGH, LOW);  // counter and some random flags
}
