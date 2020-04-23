#pragma once

#include <stdlib.h>
#include <time.h>

class RNG
{
private:
    RNG() {}
    RNG(const RNG&) {}
    ~RNG() {}

public:
    static int Rand(const int max)
    {
        static bool seeded = false;

        if (!seeded)
        {
            srand(time(NULL));
            seeded = true;
        }

        return rand() % max;
    }
};

