#include "Arduino.h"
#include "decoder.h"

Manchester::Decoder decoder;
Manchester::TS5643Field old, result;

void setup()
{
    decoder.begin(2E6); // 500ns clock
}

void loop()
{
    while (!decoder.resultBuffer.isEmpty())
    {
        decoder.resultBuffer.pop(result);
        Serial.println(result);
        if (result.count != old.count + 1)
        {
            Serial.printf("Error %d, %d --------------------------------\n", old.count, result.count);
        }
        old = result;
    }
    delay(1); // do something else, make sure you don't spend too much time otherwise the edge buffer might overflow and you'll loose data
}

void yield()
{
    decoder.tick(); // tick the decoder from yield to get ticks even during delays and other time consuming tasks...
}
