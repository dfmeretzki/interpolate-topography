/*
    Filename: resistivity.h
    Author: David F. Meretzki
    Date: 2026-03-10

    Description:
    This file contains the definitions of the resistivity functions
*/

#include "resistivity.h"

#include <math.h>

#include "config_file.h"

float skinDepth(float frequency, float resistivity)
{
    return 503.0f * sqrtf(resistivity / frequency);
}
