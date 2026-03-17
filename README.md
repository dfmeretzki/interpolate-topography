# AMGEM - Adaptive Mesh Generator for Geophysical Electromagnetic Modeling

> A companion preprocessing tool for [PETGEM](https://petgem.bsc.es/).  
> Automates the generation of simulation ready tetrahedral FE meshes for 3D CSEM surveys.

## Overview

**AMGEM** bridges the gap between raw geophysical survey data and a simulation ready mesh for [PETGEM](https://petgem.bsc.es/) (Parallel Exascale Toolkit for Geophysical Electromagnetic Modeling). Starting from a base Gmsh boundary mesh, it executes a fully automated pipeline:

1. **Multi-surface topography interpolation** — displaces mesh nodes on any number of
   geological faces (bathymetry, basement, etc.) to conform to real survey data in gridded or raw coordinate format.
2. **Laplacian mesh smoothing** — iteratively repositions interior nodes to the centroid of their neighbours, improving element quality after interpolation.
3. **Skin-depth computation** — reads a 3D resistivity model (SEG-Y) and computes the electromagnetic skin-depth across the domain to derive physically motivated element sizes.
4. **Background mesh generation** — produces a Gmsh `.pos` file that encodes spatially varying target element sizes: globally driven by the minimum skin-depth and locally refined near survey sources and receivers using a bin grid spatial index.

The output background mesh file is passed directly into a Gmsh `.geo` script to drive adaptive tetrahedral mesh generation, completing the data preparation workflow for PETGEM.

## Prerequisites

| Dependency | Version | Notes |
|---|---|---|
| CMake | ≥ 3.25 | |
| C compiler | C23 | GCC / Clang |
| Gmsh | any | For creating the base `.msh` mesh |
| segyio | 1.9.14 | Included as a Git submodule |

Getting started
--------

Clone the repository **with submodules**:
```bash
git clone --recurse-submodules https://github.com/dfmeretzki/interpolate-topography.git
```

If you cloned without submodules:
```bash
git submodule update --init --recursive
```

---

### Build

Build the program using cmake from the root directory of the project
```bash
cmake -S . -B build
cmake --build build
```

Or use one of the provided presets for debug and release builds
```bash
# Debug build
cmake --preset debug
cmake --build --preset debug

# Release build
cmake --preset release
cmake --build --preset release
```

---

### Usage

```bash
./<build_directory>/amgem <config_file>
```

example:
```bash
./out/build/release/amgem config.in
```

---

### Configuration file

All parameters are supplied through a plain text file using key = value syntax.
Lines beginning with # are comments. Multi-value parameters use comma separated lists.

```bash
# -- Topography ---------------------------------------------------------------
# Paths to topography data files, one per geological surface.
# Mapped to surfaceMeshFaces in order: 1st file → 1st face, 2nd → 2nd, etc.
topoFiles = data/bathymetry.dat, data/basement.dat

# Interpolation grid resolution (number of sample points along each axis)
nx = 150
ny = 180

# -- Mesh I/O -----------------------------------------------------------------
skinMeshFileIn  = meshes/skin.msh
skinMeshFileOut = meshes/skin_modified.msh

# -- Surface interpolation ----------------------------------------------------
# Face identifiers in the mesh corresponding to the geological surfaces
surfaceMeshFaces = 6, 11

# -- Smoothing ----------------------------------------------------------------
# Face indices where Laplacian smoothing is applied after interpolation
meshFacesToSmooth = 1, 2, 3, 4, 5
# Convergence parameters (defaults: 200 and 0.01)
iterMaxSmooth = 200
tolerSmooth   = 0.01

# -- Resistivity model --------------------------------------------------------
# SEG-Y file with 3D resistivity distribution (used for skin-depth calculation)
resistivityFile = data/resistivity.segy

# -- Sources / receivers ------------------------------------------------------
# Source and receiver positions as (x, y, z) triplets, one per line
sourcesFile = data/sources.dat

# -- Background mesh output ---------------------------------------------------
backgroundMeshFile = meshes/background.pos

# -- EM survey parameters -----------------------------------------------------
# Signal frequency in Hz (default: 1.0)
frequency = 0.25

# Global element size = min_skin_depth / rSkinDepth  (default: 2.0)
rSkinDepth = 2.0

# Emitter dipole length in metres (default: 1.0)
emitterLength = 1.0

# Local element size near sources = emitterLength / rsFactor,
# capped at the global skin-depth size (default: 10.0)
rsFactor = 10.0
```

### Parameter reference

| Parameter | Required | Default | Description |
|---|---|---|---|
| `topoFiles` | yes | — | Comma-separated paths to topography files |
| `nx`, `ny` | yes | — | Interpolation grid resolution |
| `skinMeshFileIn` | yes | — | Input boundary mesh (Gmsh `.msh` v1) |
| `skinMeshFileOut` | yes | — | Output mesh file path |
| `surfaceMeshFaces` | yes | — | Face IDs to interpolate topography onto |
| `meshFacesToSmooth` | no | — | Face IDs where Laplacian smoothing is applied |
| `iterMaxSmooth` | no | 200 | Maximum smoothing iterations |
| `tolerSmooth` | no | 0.01 | Smoothing convergence tolerance |
| `resistivityFile` | yes | — | SEG-Y resistivity model |
| `sourcesFile` | yes | — | Source/receiver positions (XYZ, one per line) |
| `backgroundMeshFile` | yes | — | Output Gmsh background mesh (`.pos`) |
| `frequency` | no | 1.0 | EM survey frequency in Hz |
| `rSkinDepth` | no | 2.0 | Global element size factor (relative to min skin-depth) |
| `emitterLength` | no | 1.0 | Emitter dipole length in metres |
| `rsFactor` | no | 10.0 | Source local refinement factor |

### Topography file format

Grid format  for regularly sampled datasets:
```
<nx> <ny>
<x_1> <x_2> ... <x_nx>
<y_1> <y_2> ... <y_ny>
<z_1,1> <z_2,1> ... <z_nx,1>
...
<z_1,ny> <z_2,ny> ... <z_nx,ny>
```

Raw XYZ format — for irregularly spaced survey points:
```
<x_1> <y_1> <z_1>
<x_2> <y_2> <z_2>
...
```
The same raw XYZ format is used for sourcesFile.

---

### Mesh refinement strategy

Element sizes are determined by two levels:

- Global size: derived from the minimum skin-depth across the resistivity model:  

  $\displaystyle h_\text{global} = \frac{503\sqrt{\rho_\text{min} / f}}{r_\text{skin}}$

- Local size: applied near each source/receiver within a radius of $3 \times h_\text{global}$, defined as:  

  $\displaystyle h_\text{local}(d) = \min\left(\frac{L_\text{emitter}}{r_s}, h_\text{global}\right)\cdot{1.25^{\lfloor{d}/{h_\text{global}}\rfloor}}$  

Size grows geometrically with distance d from the source with a factor 1.25 per layer.