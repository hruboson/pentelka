#pragma once

#include <cstdint>

enum TOOLS {
	BRUSH,
	SPRAY,
	ERASER,
	TEXT,
	FILL,
};

/*******************
 *	BRUSH PATTERNS *
 ******************/

enum class BRUSHPATTERNS {
	NONE,
	CROSS,
	DIAGCROSS,
	DENSE,
};

struct BrushPattern {
    static const int W = 8;
    static const int H = 8;
    const uint8_t data[H][W];  // 0 = transparent, 1 = paint
};

static const BrushPattern PATTERN_CROSS = {
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

static const BrushPattern PATTERN_DIAGCROSS = {
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

static const BrushPattern PATTERN_DENSE = {
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

/*****************
 * FILL PATTERNS *
 ****************/

enum class FILLPATTERNS {
	NONE,
	CROSS,
	DIAGCROSS,
	DENSE,
};
