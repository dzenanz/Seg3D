/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2009 Scientific Computing and Imaging Institute,
 University of Utah.


 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
 */

#ifndef UTILS_TEXTRENDERER_TEXTRENDERER_H
#define UTILS_TEXTRENDERER_TEXTRENDERER_H

// STL includes
#include <map>
#include <string>

// boost includes
#include <boost/smart_ptr.hpp>
#include <boost/utility.hpp>

#include <Utils/TextRenderer/FreeTypeLibrary.h>

namespace Utils
{

class TextRenderer;
typedef boost::shared_ptr< TextRenderer > TextRendererHandle;

class TextRenderer : public boost::noncopyable
{
public:
  TextRenderer();
  ~TextRenderer();

  void render( const std::string& text, unsigned char* buffer, int width, 
    int height, int x_offset, int y_offset, unsigned int font_size, 
    float red, float green, float blue, float alpha, bool blend = false );

private:
  FreeTypeLibraryHandle ft_library_;

  typedef std::map< unsigned int, FreeTypeFaceHandle > face_map_type;
  // The cache of faces indexed by their sizes
  face_map_type face_map_;

  std::string font_file_;
  bool valid_;
};

} // end namespace Utils

#endif