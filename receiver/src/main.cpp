#include "Arduino.h"
#include "decoder.h"

IntervalTimer t;

Manchester::Decoder decoder;

void setup()
{
    while (!Serial) {}
    Serial.println("start --------------");

    pinMode(10, OUTPUT); // error indicator
    pinMode(9, OUTPUT);  // monitor time in ISR
    pinMode(8, OUTPUT);  // monitor print time

    decoder.begin(2E6); //

    // t.begin([] { decoder.tick(); }, 100);
    // t.priority(255);
}

TS5643Field old;
TS5643Field result;

void loop()
{
    digitalWriteFast(8, HIGH);
    while(!decoder.resultBuffer.isEmpty())
    {
        decoder.resultBuffer.pop(result);

        Serial.println(result.count);
        if (result.count != old.count + 1)
        {
            Serial.printf("Error %d, %d --------------------------------\n", old.count, result.count);
        }
        old = result;
    }
    digitalWriteFast(8, LOW); //

    delay(10);                 // do something else, make sure you don't spend too much time otherwise the result buffer might overflow and you loose data
}

void yield()
{
   decoder.tick(); // tick the decoder from yield to get ticks even during delays and other time consuming tasks...
}
