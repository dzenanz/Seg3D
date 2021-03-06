
#
#  For more information, please see: http://software.sci.utah.edu
#
#  The MIT License
#
#  Copyright (c) 2016 Scientific Computing and Imaging Institute,
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
#

##################################################
# Set sources
##################################################

set(CORE_PYTHON_SRCS
  Util.h
  Util.cc
  PythonInterpreter.h
  PythonInterpreter.cc
  PythonActionContext.h
  PythonActionContext.cc
  ToPythonConverters.h
  ToPythonConverters.cc
  PythonCLI.h
  PythonCLI.cc
)

if(WIN32)
  add_definitions(-DPYTHONPATH=L"${SCI_PYTHON_MODULE_PARENT_PATH}"
                  -DPYTHONNAME=L"${SCI_PYTHON_NAME}")
elseif(APPLE)
  if(BUILD_TESTING)
    add_definitions(-DPYTHONPATH=L"${PYTHON_MODULE_SEARCH_PATH}:${APPLICATION_NAME}.app/Contents/Frameworks/${PYTHON_MODULE_SEARCH_ROOT}")
  else()
    add_definitions(-DPYTHONPATH=L"${PYTHON_MODULE_SEARCH_PATH}")
  endif()
  add_definitions(-DPYTHONLIBDIR=L"${SCI_PYTHON_LIBRARY_DIR}"
                  -DPYTHONLIB=L"${SCI_PYTHON_LIBRARY}")
else()
  add_definitions(-DPYTHONPATH=L"${PYTHON_MODULE_SEARCH_PATH}")
endif()

CORE_ADD_LIBRARY(Core_Python ${CORE_PYTHON_SRCS})

target_link_libraries(Core_Python
  Core_Utils
  Core_Action
  ${SCI_PYTHON_LIBRARY}
  ${SCI_BOOST_LIBRARY}
)

set_target_properties(Core_Python
  PROPERTIES
  COMPILE_DEFINITIONS_DEBUG "BOOST_DEBUG_PYTHON"
)

ADD_TEST_DIR(Tests)
