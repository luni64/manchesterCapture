#include "TS5643Field.h"

TS5643Field::TS5643Field()
{
    clear();
}

void TS5643Field::clear()
{
    count = 0;
    BE_OS = LOW;
    OF    = LOW;
    OS    = LOW;
    BA    = LOW;
    PS    = LOW;
    CE    = LOW;
    valid = false;
}

size_t TS5643Field::printTo(Print& p) const
{
    if (valid)
        p.printf("cnt: %d, BE+OS:%d OF:%d OS:%d\n", count, BE_OS, OF, OS);
    else
        p.println("not valid");
}
