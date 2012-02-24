/**	@file
* This is a wrapper header for the nvWidgets (from NVIDIA Widgets) and nvManipulators (from NVIDIA OpenGL SDK).
* NVIDIA Widgets is the immediate mode graphical user interface toolkit used by the NVIDIA SDK samples. 
* In order to compile this library, nvMath.h in NVIDIA GPU Computing SDK (or NVIDIA OpenGL SDK) and freeglut.h are required.
*/

#ifndef NVWIDGETS_INCLUDE_H
#define NVWIDGETS_INCLUDE_H

/// from NVIDIA OpenGL SDK 10.6 http://developer.nvidia.com/nvidia-graphics-sdk-11
#include "nvGlutManipulators.h"

/// from NVIDIA Widgets 1.0.0 http://code.google.com/p/nvidia-widgets/
#include "nvGlutWidgets.h"

#endif // NVWIDGETS_INCLUDE_H
