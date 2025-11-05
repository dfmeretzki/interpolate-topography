/*
    Filename: topography.c
    Author: David F. Meretzki
    Date: 2025-11-02

    Description:
    This file contains the declaration of the functions to handle topography data
*/

#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline2d.h>
#include <stdio.h>

#include "topography_parser.h"
#include "topography.h"
#include "utils.h"

static int compareNodes(const void* a, const void* b)
{
    const Node* nodeA = (const Node*)a;
    const Node* nodeB = (const Node*)b;

    if (nodeA->y < nodeB->y) return -1;
    else if (nodeA->y > nodeB->y) return 1;
    else if (nodeA->x < nodeB->x) return -1;
    else if (nodeA->x > nodeB->x) return 1;
    return 0;
}

static int findStep(const Node* nodes, size_t nNodes, size_t* step)
{
    double x = nodes[0].x;
    for (size_t i = 1; i < nNodes; ++i)
    {
        if (nodes[i].x == x)
        {
            *step = i;
            return 1;
        }
    }
    return 0;
}

static int buildOriginalTopography(Node* nodes, size_t nNodes, Topography* topo)
{
    // Sort nodes by y, then by x
    qsort(nodes, nNodes, sizeof(Node), compareNodes);

    size_t nx = 0;
    if (!findStep(nodes, nNodes, &nx))
    {
        fprintf(stderr, "Could not determine step size in topography data\n");
        return 0;
    }

    size_t ny = nNodes / nx;
    topo->nx = nx;
    topo->ny = ny;
    topo->xGrid = (double*)malloc(nx * sizeof(double));
    topo->yGrid = (double*)malloc(ny * sizeof(double));
    topo->values = (double*)malloc(nx * ny * sizeof(double));
    if (topo->xGrid == NULL || topo->yGrid == NULL || topo->values == NULL)
    {
        fprintf(stderr, "Could not allocate memory for original topography\n");
        freeTopography(topo);
        return 0;
    }

    for (size_t i = 0; i < nx; ++i)
    {
        topo->xGrid[i] = nodes[i].x;
    }
    for (size_t i = 0; i < ny; ++i)
    {
        topo->yGrid[i] = nodes[i * nx].y;
    }
    for (size_t j = 0; j < ny; ++j)
    {
        for (size_t i = 0; i < nx; ++i)
        {
            topo->values[j * nx + i] = nodes[j * nx + i].z;
        }
    }

    return 1;
}

static int buildHiResTopography(const Topography* orig, size_t nx, size_t ny, Topography* topo)
{
    topo->nx = nx;
    topo->ny = ny;
    topo->xGrid = (double*)malloc(nx * sizeof(double));
    topo->yGrid = (double*)malloc(ny * sizeof(double));
    topo->values = (double*)malloc(nx * ny * sizeof(double));
    if (topo->xGrid == NULL || topo->yGrid == NULL || topo->values == NULL)
    {
        fprintf(stderr, "Could not allocate memory for high-resolution topography\n");
        freeTopography(topo);
        return 0;
    }

    double xMin, xMax, yMin, yMax;
    minMaxElement(orig->xGrid, orig->nx, &xMin, &xMax);
    minMaxElement(orig->yGrid, orig->ny, &yMin, &yMax);

    for (size_t i = 0; i < nx; ++i)
    {
        topo->xGrid[i] = xMin + i * (xMax - xMin) / (nx - 1);
    }
    for (size_t i = 0; i < ny; ++i)
    {
        topo->yGrid[i] = yMin + i * (yMax - yMin) / (ny - 1);
    }

    return 1;
}

static int interpolate2dSpline(const Topography* orig, Topography* topo)
{
    gsl_spline2d* spline = gsl_spline2d_alloc(gsl_interp2d_bicubic, orig->nx, orig->ny);
    int status = gsl_spline2d_init(spline, orig->xGrid, orig->yGrid,
        orig->values, orig->nx, orig->ny);
    if (status != GSL_SUCCESS)
    {
        fprintf(stderr, "Error initializing 2D spline interpolation\n");
        gsl_spline2d_free(spline);
        return 0;
    }

    gsl_interp_accel* xAccel = gsl_interp_accel_alloc();
    gsl_interp_accel* yAccel = gsl_interp_accel_alloc();
    size_t nx = topo->nx;
    size_t ny = topo->ny;
    for (size_t j = 0; j < ny; ++j)
    {
        for (size_t i = 0; i < nx; ++i)
        {
            double x = topo->xGrid[i];
            double y = topo->yGrid[j];
            double z = gsl_spline2d_eval(spline, x, y, xAccel, yAccel);
            topo->values[j * nx + i] = z;
        }
    }

    gsl_interp_accel_free(xAccel);
    gsl_interp_accel_free(yAccel);
    gsl_spline2d_free(spline);

    return 1;
}



void freeTopography(Topography* topo)
{
    free(topo->xGrid);
    topo->xGrid = NULL;
    free(topo->yGrid);
    topo->yGrid = NULL;
    free(topo->values);
    topo->values = NULL;
}

int increaseTopographyResolution(const ConfigFile* config,
    const char* filename, Topography* topo)
{
    int result = 1;
    Node* nodes = NULL;
    size_t nNodes = 0;
    if (!readRawTopographyFile(filename, &nodes, &nNodes))
    {
        fprintf(stderr, "Error reading raw topography file: %s\n", filename);
        return 0;
    }

    Topography origTopo = { 0 };
    if (!buildOriginalTopography(nodes, nNodes, &origTopo))
    {
        fprintf(stderr, "Error building original topography from file: %s\n", filename);
        result = 0;
        goto out_free_nodes;
    }

    if (!buildHiResTopography(&origTopo, config->nx, config->ny, topo))
    {
        fprintf(stderr, "Error building high-resolution topography for file: %s\n",
            filename);
        result = 0;
        goto out_free_Topography;
    }

    if (!interpolate2dSpline(&origTopo, topo))
    {
        fprintf(stderr, "Error interpolating topography for file: %s\n", filename);
        freeTopography(topo);
        result = 0;
        goto out_free_Topography;
    }

out_free_Topography:
    freeTopography(&origTopo);
out_free_nodes:
    free(nodes);

    return result;
}
