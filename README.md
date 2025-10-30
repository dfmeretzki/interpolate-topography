Interpolate topography
--------

This program modifies an existing boundary mesh by moving the nodes of a given set of its faces in order to fit with a given topography

Getting started
--------

Create a configuration file with the following content
```
# name of the topography file:
topoFile = <path>/MyTopography

# name of the input and output mesh files:
skinMeshFileIn  = <path>/skin.msh
skinMeshFileOut = <path>/skin_modified.msh

# face #s corresponding to the surface to interpolate:
surfaceMeshFaces = 6

# optionals (this lines can be omitted)
# face #s corresponding to the surface which a mesh smoothing will be applied
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