#  For more information, please see: http://software.sci.utah.edu
#
#  The MIT License
#
#  Copyright (c) 2017 Scientific Computing and Imaging Institute,
#  University of Utah.
#
#
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  the rights to use, copy, modify, merge, publish, distribute, sublicense,
#  and/or sell copies of the Software, and to permit persons to whom the
#  Software is furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included
#  in all copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
#  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#  DEALINGS IN THE SOFTWARE.

set_property(DIRECTORY PROPERTY "EP_BASE" ${ep_base})

set(TETGEN_LIBRARY "tet")

# if(TRAVIS_BUILD OR ${CMAKE_VERSION} VERSION_GREATER 3.7.2)
  # # allowed since CMake 3.7.2
  # # only supporting in CI builds for now, since CMake version is so new
  # set(DOWNLOAD_URL "http://tetgen.org/files/tetgen1.4.3.tar.gz http://www.sci.utah.edu/devbuilds/seg3d/tetgen1.4.3.tar.gz")
# else()
  # set(DOWNLOAD_URL "http://tetgen.org/files/tetgen1.4.3.tar.gz")
# endif()

# ExternalProject_Add(Tetgen_external
  # URL ${DOWNLOAD_URL}
  # PATCH_COMMAND ""
  # INSTALL_COMMAND ""
  # CMAKE_CACHE_ARGS
    # -DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}
    # -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    # -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
    # -DTETGEN_LIBRARY:STRING=${TETGEN_LIBRARY}
# )


set(tetgen_SVN_URL "https://gforge.sci.utah.edu/svn/tetgen")
#set(sci_data_DIR "${CMAKE_BINARY_DIR}/SCIRunData")

ExternalProject_Add(Tetgen_external
  SVN_REPOSITORY "${tetgen_SVN_URL}"
  SVN_TRUST_CERT 1
  SVN_USERNAME "anonymous"
  SVN_PASSWORD ""
  PATCH_COMMAND ""
  INSTALL_COMMAND ""
  CMAKE_CACHE_ARGS
    -DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
    -DTETGEN_LIBRARY:STRING=${TETGEN_LIBRARY}
)

ExternalProject_Add_Step(Tetgen_external add_cmakelists
  COMMAND "${CMAKE_COMMAND}" -E copy ${SUPERBUILD_DIR}/TetgenCMakeLists.txt CMakeLists.txt
  DEPENDEES download
  DEPENDERS configure
  ALWAYS 1
  WORKING_DIRECTORY <SOURCE_DIR>
)

ExternalProject_Get_Property(Tetgen_external SOURCE_DIR)
ExternalProject_Get_Property(Tetgen_external BINARY_DIR)
ExternalProject_Get_Property(Tetgen_external INSTALL_DIR)
set(TETGEN_INCLUDE ${SOURCE_DIR})
set(TETGEN_LIBRARY_DIR ${BINARY_DIR})
set(TETGEN_USE_FILE ${INSTALL_DIR}/UseTetgen.cmake)
# see Tetgen CMakeLists.txt file
set(Tetgen_DIR ${INSTALL_DIR} CACHE PATH "")

# Normally this should be handled in external library repo
configure_file(${SUPERBUILD_DIR}/TetgenConfig.cmake.in ${INSTALL_DIR}/TetgenConfig.cmake @ONLY)
configure_file(${SUPERBUILD_DIR}/UseTetgen.cmake ${TETGEN_USE_FILE} COPYONLY)

message(STATUS "Tetgen_DIR: ${Tetgen_DIR}")
