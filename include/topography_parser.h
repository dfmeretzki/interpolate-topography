/*
    Filename: topography_parser.h
    Author: David F. Meretzki
    Date: 2025-10-25

    Description:
    This file contains the declaration of the functions to parse topography files
*/

#ifndef TOPOGRAPHY_PARSER_H
#define TOPOGRAPHY_PARSER_H

#include "mesh.h"

/**
 * Reads a topography file and fills the Topography structure
 *
 * @param filename The path to the topography file to read
 * @param topo Pointer to a Topography structure that will be filled with data
 * @return 1 on success, 0 on failure
 */
int readTopographyFile(const char* filename, Topography* topo);

/**
 * Reads a topography file containing only coordinate (x, y, z) data for nodes
 *
 * @param filename The path to the topography file to read
 * @param nodes Pointer to a Node pointer that will be allocated and filled with node data
 * @param nNodes Pointer to a size_t that will be set to the number of nodes read
 * @return 1 on success, 0 on failure
 */
int readRawTopographyFile(const char* filename, Node** nodes, size_t* nNodes);

#endif // TOPOGRAPHY_PARSER_H
