# Qt Filament hello world example
This example program builds a Qt window and displays a simple PBR scene, using Imaged based lighting.
The goal was to create a bare minimum example of Physically based rendering using Filament and Qt.
Please feel free to correct areas and build upon this example.

## Build Instruction
We need to first compile our material to access it from the program. 
Next we need to compile our test mesh to allow us to load it using the filameshio lib.
Following that, we simply run qmake, and then make.
```
> matc -o assets/materials/aiDefaultMat.inc -f header assets/materials/aiDefaultMat.mat
> filamesh -c assets/meshes/suzanne.obj assets/meshes/suzanne.filamesh
> qmake
> make -j
> ./build/bin/QtFilamentPBR
```

## Notes
The `filament_raii.h` header contains some simple wrapper classes around filament entities and engine registered objects, to ensure they are correctly destroyed in a modern C++ manor.
If you would rather not use them, you should simply define a destructor in the FilamentWindow class, that destroys all of the resources manually.

There are a couple of other helper classes, namely `EnvironmentMap` which is a basic abstraction of an imaged based lighting setup + a sky box background, 
and `TrackballCamera` which implements orbiting and zooming around the mesh in response to mouse movement.
The `EnvironmentMap` class depends on the two ktx files in assets/env/pillars however these can be exchanged with any other pair from the filament samples.

## Result
<p align="center">
<img src="https://i.imgur.com/BedXVmS.gif" width="800">
</p>


