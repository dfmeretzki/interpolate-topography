/*
    Filename: utils.h
    Author: David F. Meretzki
    Date: 2025-10-21

    Description:
    This file contains a collection of declarations for common utility functions
    that are used throughout the Interpol Topography project 
*/

#ifndef UTILS_H
#define UTILS_H

void removeSpaces(char* s);

void combinePaths(char* dest, const char* path1, const char* path2);

void minMaxElement(const double* array, size_t n, double* min, double* max);

#endif