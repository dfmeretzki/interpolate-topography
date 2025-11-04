Interpolate topography
--------

This program modifies an existing boundary mesh by moving the nodes of a given set of its faces in order to fit with a given topography

Getting started
--------

Create a configuration file with the following content
```
# name of the topography files. If more than one file is provided,
# the number of files must match the number of surfaceMeshFaces
# the topography files will be interpolated onto the surfaceMeshFaces in the
# order they are listed ie: 1st file -> 1st surface, 2nd file -> 2nd surface, etc.
topoFiles = <path>/MyTopography, <path>/OtherTopography

# number of x and y values to use on the grid:
nx = 150    
ny = 180

# name of the input and output mesh files:
skinMeshFileIn  = <path>/skin.msh
skinMeshFileOut = <path>/skin_modified.msh

# face numbers corresponding to the surfaces to interpolate:
surfaceMeshFaces = 6, 11

# optionals (this lines can be omitted)
# face numbers corresponding to the surfaces which a mesh smoothing will be applied
meshFacesToSmooth = 1, 2, 3, 4, 5
# maximum number of smoothing iterations and tolerance
# if not specified, default values will be used (200 and 0.01 respectively)
iterMaxSmooth = 200
tolerSmooth   = 0.01
```

Build the program using cmake from the root directory of the project
```
cmake -S . -B build
cmake --build build
```

Or use one of the provided presets for debug and release builds. For a debug build, run
```
cmake --preset debug
cmake --build --preset debug
```
For a release build, run
```
cmake --preset release
cmake --build --preset release
```

Then run the program as follows
```
./<build_directory>/interpol_topography config.in