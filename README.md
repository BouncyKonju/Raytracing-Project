## Building instructions

This project can be build using cmake. What you will need for that:

- CMake and a valid build tool for that (e.g. ninja)

- A valid installation of OpenCV somewhere on your computer (tested with OpenCV 4.6.0, although any 4.x should work)

- GCC (MSVC or CLang will likely need adjustments)

> :exclamation: Your device will need support for OpenCL 2.0. If it doesn't have that, you can still compile this program, but execution is not guaranteed, although it can work (you just have to try!). If your card only supports OpenCL 1.2 (as the 1070 Ti in my main PC does) it could work, but might run into problems during runtime.

Step-by-step:

1. Create a folder named `build`

2. From the terminal, call `cmake .\CMakeLists.txt -Bbuild`

3. If no error occured, execute `cmake --build build -- -j3`

4. Copy the resulting file in `build` into the main project folder and execute it


## Usage instructions

#### Syntax

Following the syntax given in "design/Implementation.pdf", paragraph "File Formats", you should adjust variables and objects according to how you want the scene to look.

> :bell: Color values should be on a scale of 0-1 (in floats). Putting a color outside that range results in undefined behaviour.

#### The object tree

The objects follow a specific format:

- So called "Base objects" are what you create with `!obj = sphere` for example. These are the basic building blocks of your scene.

- So called "Transformed objects" are what you create through any transformation, e.g. rotation or scaling. These Are second in the scene tree and transformations can only be applied to base objects or other transformed objects.

- So called "Complex objects" are what you create through other interactions, e.g. unions or intersections. Complex modifications cannot be further transformed and represent the top of the scene tree. They can be applied to all object types.

> :bell: Material assignment is an exception to these and can be applied to anything. Behavior of that however is merely defined for assigning to a base object.

> :exclamation: Complex interactions don't matter due to the algorithm not working properly. You will have to, however, combine base objects using `Complex`es anyways since a tree is required. Submit the top object in that tree. It does not matter which interactions you choose, all will lead to the same: effectively a union.

#### Your own RTI file

If no arguments for execution are provided, the program assumes that you use the default `input.rti` file. Otherwise, the first argument specifies the path to the file that should be interpreted, i.e. `raytracing.exe micky.rti` will interpret whatever is in `micky.rti` and raytrace it.

> :bell: Only one file can be interpreted for raytracing, so you have to put the entire script in there! No includes or similar things.

> Remember your graphics card RAM: The memory usage of the raytracer is dependent on your input parameters, more specifically width, height, amount of objects and materials and max_reflections. The exact amount will be shown by the program on execution. If that value exceeds your cards VRAM, the program will run slower, but still be able to generate an image. Remember however that the raytracing code itself uses some VRAM, although the exact amount is very much dependent on your hardware's compiler and resource needs.

> ~~If the program calculates a low value but OpenCL returns an error saying that some ridiculously high amount of VRAM is not available, simply restart. I haven't found the issue where it comes to this conclusion, all I can say is that it should work on the second try.~~ This should be fixed now, but might still occur if I've missed a case.

## Documentation generation

Due to the size of the generated files, I have opted to not include the finished Doxygen docs. If you, however, wish to read through them, here are some things to look out for:

1. You need to enable recursive search

2. All entities need to be extracted

3. Optimize for C++ output (although C++/CLI could work as well)

4. Add lib to EXCLUDE, otherwise you will get a lot of documentation for things related to the OpenCL wrapper and not the project

5. I recommend an output with a navigation panel, although this is optional and up to you.

6. Diagrams.

### Last notes

The 'timeline' folder contains some funny in-between results that certainly should not make it into a final build.

## Sources for the libraries:

- OpenCL: https://github.com/ProjectPhysX/OpenCL-Wrapper (17.06.2022, 16:00)
