
#include "Arduino.h"
#include "decoder.h"

Manchester::Decoder decoder;

void setup()
{
    pinMode(9, OUTPUT); // monitor time in ISR
    pinMode(8, OUTPUT); // monitor time in ISR

    while (!Serial) {}
    Serial.println("start --------------");

    decoder.begin();
}

void loop()
{
    char buf[1000];
    digitalWriteFast(8, HIGH);
    size_t i = decoder.read(buf, 1000); // copy the decoded chars from the decoder to some buffer
    Serial.write(buf, i);               // write this buffer to Serial (or do whatever you need to do with it)
                                        //
    digitalWriteFast(8, LOW);           //
    delay(100);                         // do something else, make sure you don't spend too much time otherwise the result buffer might overflow and you loose data
}

void yield()
{
    decoder.tick(); // tick the decoder from yield to get ticks even during delays and other time consuming tasks...
}