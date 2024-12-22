# floodcontrol

floodcontrol is a port in C++ 20 and OpenGL of the game with the same
name written for the XNA framework.

The code has been heavily reworked but the game logic stayed the same.

## Dependencies

To build the game you need the meson build system and a C++ 20
compiler.

The following libraries are required:

  * GLFW3 - the graphics library
  * glew - for the OpenGL extensions
  * glm - for vectors and matrices
  * freetype2 - to load the fonts

If any of these dependencies are not available, the meson build system
will download and compile them automatically. Please note however that
they may not be the latest version available.

## Quick start

1. Configure the project and the build directory:

   ```
   $ meson setup build
   ```

2. Compile the project:

    ```
	$ meson compile -C build
	```

3. Run the game:

   ```
   $ build/src/floodcontrol
   ```

