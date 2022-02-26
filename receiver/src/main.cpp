
#include "Arduino.h"
#include "decoder.h"

Manchester::Decoder decoder;

void setup()
{
    pinMode(9, OUTPUT);
    pinMode(8, OUTPUT);
    while (!Serial) {}
    Serial.println("start --------------");

    decoder.begin();
}

void loop()
{
    char buf[1000];
    size_t i = decoder.read(buf, 1000);
    Serial.write(buf, i);

    delay(100); // do something else
}

void yield()
{
    decoder.tick();
}