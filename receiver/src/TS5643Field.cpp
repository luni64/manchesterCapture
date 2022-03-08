#include "TS5643Field.h"

namespace Manchester
{
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
    }

    size_t TS5643Field::printTo(Print& p) const
    {
        return p.printf("cnt: %d, BE+OS:%d OF:%d OS:%d BA:%d PS:%d CE:%d", count, BE_OS, OF, OS, BA, PS, CE);
    }
}