/*
    Filename: resistivity.h
    Author: David F. Meretzki
    Date: 2026-03-10

    Description:
    This file contains the declarations of the resistivity structure and its functions
*/

#ifndef RESISTIVITY_H
#define RESISTIVITY_H

typedef struct
{
    int nx;                 // number of inlines
    int ny;                 // number of crosslines
    int nz;                 // number of depth samples
    float dx;               // inline spacing
    float dy;               // crossline spacing
    float dz;               // depth sampling interval
    float minX;             // minimum X coordinate
    float maxX;             // maximum X coordinate
    float minY;             // minimum Y coordinate
    float maxY;             // maximum Y coordinate
    float minZ;             // minimum Z coordinate (t0)
    float maxZ;             // maximum Z coordinate (t0 + (nz-1)*dz)
    float minResistivity;   // minimum resistivity value in the dataset
} Resistivity;

float skinDepth(float frequency, float resistivity);

#endif // RESISTIVITY_H
