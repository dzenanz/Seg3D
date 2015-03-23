# Seg3D
Seg3D is a free volume segmentation and processing tool developed by the NIH Center for Integrative Biomedical Computing at the University of Utah Scientific Computing and Imaging (SCI) Institute.


## Installing Seg3D from source

### Compiler Requirements

**C++11 compiler support is required.**

#### Windows Vista, 7, 8

The current source code was optimized for use with Visual Studio (VS2013 ).
The code should compile fine using the Visual Studio Express versions as well. The code
has not been tested under Cygwin or MinGW compilers.

#### OS X

The source code base was using under Xcode as well as using make/gcc and works for both 
environments.  

#### Linux

The code base has been tested for use with GCC and this is the recommended compiler for
linux.


### Dependencies

#### Qt

Before building Seg3D, please make sure that Qt 4.7 or higher has been installed on your system.

##### Windows

A Visual Studio binary build is available.
To our knowledge the Windows Visual Studio development libraries are only available in a 32-bit version.
A 64-bit version can be built from the source code download, configuring it as described on the Qt webpage.

##### Mac OS X

Qt binaries are available on the Qt website or can be built from source code.

##### Linux

Qt is available in most package managers. Look for Qt 4.7-4.8.


### Compiling Seg3D

Once you have obtained a compatible compiler and installed Qt 4.7 on your system, you need to 
download and install CMake ( www.cmake.org ) to actually build the software. CMake is a platform
independent configuring system that is used for generating Makefiles, Visual Studio project files,
or Xcode project files. Once CMake has been installed, run CMake from your build (bin) directory and
give a path to the source directory (src) containing the master CMakeLists.txt file.
For example, on the command line:

```
cd bin
cmake ../src
```

The console version ccmake or GUI version can also be used.
You may be prompted to specify your location of the Qt installation.
If you installed Qt in the default location, it should find Qt automatically.
After configuration is done, generate the make files or project files for your favorite
development environment and build.

For questions and issues regarding building the software from source, 
please email our support list: seg3d@sci.utah.edu
