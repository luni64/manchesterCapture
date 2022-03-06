
#include "Arduino.h"
#include "decoder.h"

IntervalTimer t;

Manchester::Decoder decoder;

void setup()
{
    pinMode(10, OUTPUT); // monitor time in ISR
    pinMode(9, OUTPUT);  // monitor time in ISR
    pinMode(8, OUTPUT);  // monitor time in ISR

    while (!Serial) {}
    Serial.println("start --------------");
    decoder.begin(2E6);

    // for (int i = 0; i < 500; i++)
    // {
    //     Serial.printf("%d %d\n",i, NVIC_GET_PRIORITY((IRQ_NUMBER_t)i));
    // }

    // while(1)
    //     ;


    t.begin([] { decoder.tick(); }, 100);
    t.priority(255);
}

TS5643Field old;

void loop()
{
    digitalWriteFast(8, HIGH);
    for (int i = 0; i < 150; i++)
    {
        if (decoder.resultBuffer.isEmpty()) break;



        TS5643Field result;

        constexpr size_t s = sizeof(result);

        noInterrupts();
        decoder.resultBuffer.pop(result);
        interrupts();

        //Serial.printf("%0.3d %d\n", i, result.count);
        if (result.count != old.count + 1)
        {
            Serial.println("Error --------------------------------");
        }
        old = result;

    }
    

    digitalWriteFast(8, LOW); //
    delay(10);                 // do something else, make sure you don't spend too much time otherwise the result buffer might overflow and you loose data
}

void yield()
{
    //decoder.tick(); // tick the decoder from yield to get ticks even during delays and other time consuming tasks...
}