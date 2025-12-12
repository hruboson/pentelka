#pragma once

#include <cstdint>

//TODO namespace Pentelka

enum TOOLS {
	BRUSH,
	SPRAY,
	ERASER,
	TEXT,
	FILL,
};

/*************
 *	PATTERNS *
 ************/

enum class TOOLPATTERN {
	NONE,
	CROSS,
	DIAGCROSS,
	DENSE,
};

struct Pattern {
    static const int W = 8;
    static const int H = 8;
    const uint8_t data[H][W];  // 0 = transparent, 1 = paint
};

static const Pattern PATTERN_CROSS = {
    {
        {0,0,1,0,0,1,0,0},
        {0,0,1,0,0,1,0,0},
        {1,1,1,1,1,1,1,1},
        {0,0,1,0,0,1,0,0},
        {0,0,1,0,0,1,0,0},
        {1,1,1,1,1,1,1,1},
        {0,0,1,0,0,1,0,0},
        {0,0,1,0,0,1,0,0}
    }
};

static const Pattern PATTERN_DIAGCROSS = {
    {
        {1,0,0,0,0,0,0,1},
        {0,1,0,0,0,0,1,0},
        {0,0,1,0,0,1,0,0},
        {0,0,0,1,1,0,0,0},
        {0,0,0,1,1,0,0,0},
        {0,0,1,0,0,1,0,0},
        {0,1,0,0,0,0,1,0},
        {1,0,0,0,0,0,0,1}
    }
};

static const Pattern PATTERN_DENSE = {
    {
        {1,0,1,0,1,0,1,0},
        {0,1,0,1,0,1,0,1},
        {1,0,1,0,1,0,1,0},
        {0,1,0,1,0,1,0,1},
        {1,0,1,0,1,0,1,0},
        {0,1,0,1,0,1,0,1},
        {1,0,1,0,1,0,1,0},
        {0,1,0,1,0,1,0,1}
    }
};
