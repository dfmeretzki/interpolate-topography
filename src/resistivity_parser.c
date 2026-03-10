/*
    Filename: resistivity_parser.c
    Author: David F. Meretzki
    Date: 2026-03-05

    Description:
    This file contains the implementation of the functions to parse resistivity files
*/

#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <segyio/segy.h>

#include "resistivity_parser.h"

int readSEGYFile(const char* filename, Resistivity* res)
{
    int result = 1;
    segy_file* sf = segy_open(filename, "r");
    if (sf == NULL)
    {
        fprintf(stderr, "Could not open SEG-Y file: %s\n", filename);
        return 0;
    }

    int binHeaderSize = segy_binheader_size();
    char binHeader[binHeaderSize];
    if (segy_binheader(sf, binHeader) != SEGY_OK)
    {
        fprintf(stderr, "Error reading binary header from SEG-Y file: %s\n", filename);
        result = 0;
        goto out_close_file;
    }

    int samples = segy_samples(binHeader);
    int format = segy_format(binHeader);
    int traceBSize = segy_trace_bsize(samples);
    long traceOffset = segy_trace0(binHeader);
    int totalTraces;
    if (segy_traces(sf, &totalTraces, traceOffset, traceBSize) != SEGY_OK)
    {
        fprintf(stderr, "Error counting traces in SEG-Y file: %s\n", filename);
        result = 0;
        goto out_close_file;
    }

    res->minX = DBL_MAX;
    res->minY = DBL_MAX;
    res->minZ = DBL_MAX;
    res->maxX = -DBL_MAX;
    res->maxY = -DBL_MAX;
    res->maxZ = -DBL_MAX;
    res->minResistivity = DBL_MAX;

    char trHeader[SEGY_TRACE_HEADER_SIZE];
    float* trBuf = malloc(traceBSize);
    if (trBuf == NULL)
    {
        fprintf(stderr, "Memory allocation failed for trace buffer\n");
        result = 0;
        goto out_close_file;
    }

    for (int i = 0; i < totalTraces; ++i)
    {
        if (segy_traceheader(sf, i, trHeader, traceOffset, traceBSize) != SEGY_OK)
        {
            fprintf(stderr, "Error reading trace header for trace %d\n", i);
            result = 0;
            goto out_free_trbuf;
        }

        int32_t sx, sy, scalar;
        if (segy_get_field(trHeader, SEGY_TR_CDP_X, &sx) != SEGY_OK ||
            segy_get_field(trHeader, SEGY_TR_CDP_Y, &sy) != SEGY_OK ||
            segy_get_field(trHeader, SEGY_TR_SOURCE_GROUP_SCALAR, &scalar) != SEGY_OK)
        {
            fprintf(stderr, "Error reading trace header fields for trace %d\n", i);
            result = 0;
            goto out_free_trbuf;
        }

        float scale = (float)scalar;
        if (scalar == 0) scale = 1.0;
        if (scalar < 0)  scale = -1.0f / scale;
        float x = sx * scale;
        float y = sy * scale;

        if (x < res->minX) res->minX = x;
        if (y < res->minY) res->minY = y;
        if (x > res->maxX) res->maxX = x;
        if (y > res->maxY) res->maxY = y;

        if (segy_readtrace(sf, i, trBuf, traceOffset, traceBSize) != 0)
        {
            fprintf(stderr, "Error reading trace data for trace %d\n", i);
            result = 0;
            goto out_free_trbuf;
        }

        if (segy_to_native(format, samples, trBuf) != 0)
        {
            fprintf(stderr, "Error converting trace data to native format for trace %d\n", i);
            result = 0;
            goto out_free_trbuf;
        }

        for (int j = 0; j < samples; ++j)
        {
            if (trBuf[j] < res->minResistivity) res->minResistivity = trBuf[j];
        }
    }

     /* Z bounds come from the sample indices (depth/time axis) */
    float dt;
    if (segy_sample_interval(sf, 4000.0f, &dt) != SEGY_OK)
    {
        fprintf(stderr, "Error reading sample interval from SEG-Y file: %s\n", filename);
        result = 0;
        goto out_free_trbuf;
    }

    if (segy_traceheader(sf, 0, trHeader, traceOffset, traceBSize) != SEGY_OK)
    {
        fprintf(stderr, "Error reading trace header for first trace\n");
        result = 0;
        goto out_free_trbuf;
    }

    int32_t delrt;
    if (segy_get_field(trHeader, SEGY_TR_DELAY_REC_TIME, &delrt) != SEGY_OK)
    {
        fprintf(stderr, "Error reading delay recording time from trace header\n");
        result = 0;
        goto out_free_trbuf;
    }
    res->nz = samples;
    res->dz = dt / 1000.0f;
    res->minZ = (float)delrt;
    res->maxZ = (float)delrt + (samples - 1) * (dt / 1000.0f);

    int32_t x0_raw, y0_raw, scalar0;
    if (segy_get_field(trHeader, SEGY_TR_CDP_X, &x0_raw) != SEGY_OK ||
        segy_get_field(trHeader, SEGY_TR_CDP_Y, &y0_raw) != SEGY_OK ||
        segy_get_field(trHeader, SEGY_TR_SOURCE_GROUP_SCALAR, &scalar0) != SEGY_OK)
    {
        fprintf(stderr, "Error reading initial trace header fields\n");
        result = 0;
        goto out_free_trbuf;
    }

    int sorting;
    if (segy_sorting(sf, SEGY_TR_INLINE, SEGY_TR_CROSSLINE, SEGY_TR_OFFSET,
        &sorting, traceOffset, traceBSize) != SEGY_OK)
    {
        fprintf(stderr, "Error determining sorting of SEG-Y file: %s\n", filename);
        result = 0;
        goto out_free_trbuf;
    }

    int offsets;
    if (segy_offsets(sf, SEGY_TR_INLINE, SEGY_TR_CROSSLINE, totalTraces,
        &offsets, traceOffset, traceBSize) != SEGY_OK)
    {
        fprintf(stderr, "Error determining offsets in SEG-Y file: %s\n", filename);
        result = 0;
        goto out_free_trbuf;
    }

    int nx, ny;
    if (segy_lines_count(sf, SEGY_TR_INLINE, SEGY_TR_CROSSLINE, sorting, offsets,
        &nx, &ny, traceOffset, traceBSize) != SEGY_OK)
    {
        fprintf(stderr, "Error counting lines in SEG-Y file: %s\n", filename);
        result = 0;
        goto out_free_trbuf;
    }
    res->nx = nx;
    res->ny = ny;

    float scale = (float)scalar0;
    if (scalar0 == 0)  scale = 1.0f;
    if (scalar0 < 0)   scale = -1.0f / scale;

    int32_t x1, y1, xn, yn;
    if (sorting == SEGY_INLINE_SORTING)
    {
        // trace 1 advances the crossline (y), trace ny advances the inline (x)
        if (segy_traceheader(sf, 1, trHeader, traceOffset, traceBSize) != SEGY_OK ||
            segy_get_field(trHeader, SEGY_TR_CDP_X, &x1) != SEGY_OK ||
            segy_get_field(trHeader, SEGY_TR_CDP_Y, &y1) != SEGY_OK ||
            segy_traceheader(sf, ny, trHeader, traceOffset, traceBSize) != SEGY_OK ||
            segy_get_field(trHeader, SEGY_TR_CDP_X, &xn) != SEGY_OK ||
            segy_get_field(trHeader, SEGY_TR_CDP_Y, &yn) != SEGY_OK)
        {
            fprintf(stderr, "Error reading trace headers for inline sorting\n");
            result = 0;
            goto out_free_trbuf;
        }
    }
    else if (sorting == SEGY_CROSSLINE_SORTING)
    {
        // trace 1 advances the inline (x), trace nx advances the crossline (y)
        if (segy_traceheader(sf, 1, trHeader, traceOffset, traceBSize) != SEGY_OK ||
            segy_get_field(trHeader, SEGY_TR_CDP_X, &x1) != SEGY_OK ||
            segy_get_field(trHeader, SEGY_TR_CDP_Y, &y1) != SEGY_OK ||
            segy_traceheader(sf, nx, trHeader, traceOffset, traceBSize) != SEGY_OK ||
            segy_get_field(trHeader, SEGY_TR_CDP_X, &xn) != SEGY_OK ||
            segy_get_field(trHeader, SEGY_TR_CDP_Y, &yn) != SEGY_OK)
        {
            fprintf(stderr, "Error reading trace headers for crossline sorting\n");
            result = 0;
            goto out_free_trbuf;
        }
    }

    if (sorting == SEGY_UNKNOWN_SORTING)
    {
        res->dx = (nx > 1) ? (float)((res->maxX - res->minX) / (nx - 1)) : 0.0f;
        res->dy = (ny > 1) ? (float)((res->maxY - res->minY) / (ny - 1)) : 0.0f;
    }
    else
    {
        float il_dx = fabsf((xn - x0_raw) * scale);
        float il_dy = fabsf((yn - y0_raw) * scale);
        float cr_dx = fabsf((x1 - x0_raw) * scale);
        float cr_dy = fabsf((y1 - y0_raw) * scale);

        bool inline_is_x = fabsf(il_dx) >= fabsf(il_dy);    // inlines run along X
        bool crossline_is_y = fabsf(cr_dy) >= fabsf(cr_dx); // crosslines run along Y

        if (!inline_is_x && !crossline_is_y)
        {
            // Inlines and crosslines are not aligned with the coordinate axes,
            // we need to swap the dimensions to match the file's orientation
            res->nx = ny;
            res->ny = nx;
            res->dx = cr_dx;
            res->dy = il_dy;
        }
        else
        {
            res->dx = il_dx;
            res->dy = cr_dy;
        }
    }

out_free_trbuf:
    free(trBuf);
out_close_file:
    segy_close(sf);
    return result;
}
