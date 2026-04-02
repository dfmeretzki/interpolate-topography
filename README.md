# AMGEM - Adaptive Mesh Generator for Geophysical Electromagnetic Modeling

> A companion preprocessing tool for [PETGEM](https://github.com/ocastilloreyes/petgem).  
> Automates the generation of simulation ready tetrahedral FE meshes for 3D CSEM surveys.

## Overview

**AMGEM** bridges the gap between raw geophysical survey data and a simulation ready mesh for [PETGEM](https://github.com/ocastilloreyes/petgem) (Parallel Exascale Toolkit for Geophysical Electromagnetic Modeling). Starting from a base Gmsh boundary mesh, it executes a fully automated pipeline:

1. **Multi-surface topography interpolation** — displaces mesh nodes on any number of
   geological faces (bathymetry, basement, etc.) to conform to real survey data in gridded or XYZ coordinate format.
2. **Laplacian mesh smoothing** — iteratively repositions interior nodes to the centroid of their neighbours, improving element quality after interpolation.
3. **Skin-depth computation** — reads a 3D resistivity model (SEG-Y) and computes the electromagnetic skin-depth across the domain to derive physically motivated element sizes.
4. **Background mesh generation** — produces a Gmsh `.pos` file that encodes spatially varying target element sizes: globally driven by the minimum skin-depth and locally refined near survey sources and receivers using a bin grid spatial index.

The output background mesh file is passed directly into a Gmsh `.geo` script to drive adaptive tetrahedral mesh generation, completing the data preparation workflow for PETGEM.

## Prerequisites

| Dependency | Version | Notes |
|---|---|---|
| CMake | ≥ 3.25 | |
| C compiler | C23 | GCC / Clang |
| segyio | 1.9.14 | Included as a Git submodule |
| GNU Scientific Library (gsl) | ≥ 2.0 | |


Install GSL on Debian/Ubuntu:

```bash
sudo apt install libgsl-dev
```

Install GSL on Fedora/RHEL/CentOS Stream:

```bash
sudo dnf install gsl-devel
```

Install GSL on Arch Linux:

```bash
sudo pacman -S gsl
```

Install GSL on macOS (Homebrew):

```bash
brew install gsl
```

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

Or use one of the provided presets for debug and release builds. For debug builds use

```bash
cmake --preset debug
cmake --build --preset debug
```
and for release builds use
```bash
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
# AMGEM configuration file

# -- Mode of operation --------------------------------------------------------
# Valid values: all, interpolate, background_mesh
# interpolate = only perform topography interpolation and smoothing
# background_mesh = only perform background mesh generation using the input mesh and resistivity model
mode = all

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
# Minimum resistivity in ohm-m, if not specified, it will be extracted from the SEG-Y file
minResistivity = 0.1
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

# Geometric growth factor for element size away from sources (default: 1.25)
# Element size grows by this factor each layer away from sources center, up to the global size limit with a maximum distance of 3.0 * global element size
growthFactor = 1.25
```

### Parameter reference

| Parameter | Required | Default | Description |
|---|---|---|---|
| `mode` | no | all | Operation mode: all, interpolate, background_mesh |
| `topoFiles` | yes | — | Comma-separated paths to topography files |
| `nx`, `ny` | yes | — | Interpolation grid resolution |
| `skinMeshFileIn` | yes | — | Input boundary mesh (Gmsh `.msh` v1) |
| `skinMeshFileOut` | yes | — | Output mesh file path |
| `surfaceMeshFaces` | yes | — | Face IDs to interpolate topography onto |
| `meshFacesToSmooth` | no | — | Face IDs where Laplacian smoothing is applied |
| `iterMaxSmooth` | no | 200 | Maximum smoothing iterations |
| `tolerSmooth` | no | 0.01 | Smoothing convergence tolerance |
| `minResistivity` | yes/no* | — | Minimum resistivity in ohm-m (overrides SEG-Y extraction) |
| `resistivityFile` | yes/no* | — | SEG-Y resistivity model |
| `sourcesFile` | yes | — | Source/receiver positions (XYZ, one per line) |
| `backgroundMeshFile` | yes | — | Output Gmsh background mesh (`.pos`) |
| `frequency` | no | 1.0 | EM survey frequency in Hz |
| `rSkinDepth` | no | 2.0 | Global element size factor (relative to min skin-depth) |
| `emitterLength` | no | 1.0 | Emitter dipole length in metres |
| `rsFactor` | no | 10.0 | Source local refinement factor |
| `growthFactor` | no | 1.25 | Geometric growth factor for element size away from sources |

*either `minResistivity` or `resistivityFile` must be provided to compute skin-depths.If both are provided, `minResistivity` will be used.

### Topography file format

Grid format for regularly sampled datasets:
```
<nx> <ny>
<x_1> <x_2> ... <x_nx>
<y_1> <y_2> ... <y_ny>
<z_1,1> <z_2,1> ... <z_nx,1>
...
<z_1,ny> <z_2,ny> ... <z_nx,ny>
```

XYZ format for irregularly spaced survey points:
```
<x_1> <y_1> <z_1>
<x_2> <y_2> <z_2>
...
```
The same XYZ format is used for sourcesFile.

---

### Mesh refinement strategy

Element sizes are determined by two levels:

- Global size: derived from the minimum skin-depth across the resistivity model:  

  $\displaystyle h_\text{global} = \frac{503\sqrt{\rho_\text{min} / f}}{r_\text{skin}}$

- Local size: applied near each source/receiver within a radius of $3 \times h_\text{global}$, defined as:  

  $\displaystyle h_\text{local}(d) = \min\left(\frac{L_\text{emitter}}{r_s}, h_\text{global}\right)\cdot{growthFactor^{\lfloor{d}/{h_\text{global}}\rfloor}}$  

Size grows geometrically with distance d from the source with a factor `growthFactor` per layer.