#pragma once

#define OFFWHITE (Color){252, 252, 252, 255}
#define TRANSPARENT (Color){252, 252, 252, 0}
#define WHITE (Color){255, 255, 255, 255}
#define DARKGREY (Color){215, 215, 215, 255}
#define DARKERGREY (Color){120, 120, 120, 255}
#define LIGHTGREY (Color){221, 221, 221, 100}
#define WHITEBLUE (Color){227, 243, 248, 255}
#define LIGHTBLUE (Color){227, 243, 248, 255}
#define LIGHTERBLUE (Color){227, 243, 248, 255}
#define DARKBLUE (Color){17,125,187,255}

static float TextToFloat(const char *text)
{
    float value = 0.0f;
    float sign = 1.0f;

    if ((text[0] == '+') || (text[0] == '-'))
    {
        if (text[0] == '-') sign = -1.0f;
        text++;
    }

    int i = 0;
    for (; ((text[i] >= '0') && (text[i] <= '9')); i++) value = value*10.0f + (float)(text[i] - '0');

    if (text[i++] != '.') value *= sign;
    else
    {
        float divisor = 10.0f;
        for (; ((text[i] >= '0') && (text[i] <= '9')); i++)
        {
            value += ((float)(text[i] - '0'))/divisor;
            divisor = divisor*10.0f;
        }
    }

    return value;
}
