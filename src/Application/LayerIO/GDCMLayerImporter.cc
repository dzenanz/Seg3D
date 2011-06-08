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

#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable: 4267 )
#endif

// STL includes
#include <limits>

// GDCM Includes
#include <gdcmImageReader.h>
#include <gdcmImageHelper.h>
#include <gdcmRescaler.h>
#include <gdcmAttribute.h>
#include <gdcmUnpacker12Bits.h>

#ifdef _WIN32
#pragma warning( pop )
#endif

// Core includes
#include <Core/DataBlock/StdDataBlock.h>

// Application includes
#include <Application/LayerIO/GDCMLayerImporter.h>
#include <Application/LayerIO/LayerIO.h>

SEG3D_REGISTER_IMPORTER( Seg3D, GDCMLayerImporter );

namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// Class GDCMLayerImporterPrivate
//////////////////////////////////////////////////////////////////////////

class GDCMLayerImporterPrivate
{
public:
  GDCMLayerImporterPrivate() : 
    rescale_slope_( 0.0 ),
    rescale_intercept_( 0.0 ),
    buffer_length_( 0 ),
    slice_data_size_( 0 ),
    pixel_type_( Core::DataType::UCHAR_E ),
    origin_( 0.0, 0.0, 0.0 ),
    row_direction_( 1.0, 0.0, 0.0 ),
    col_direction_( 0.0, 1.0, 0.0 ),
    slice_direction_( 0.0, 0.0, 1.0 ),
    x_spacing_( 1.0 ),
    y_spacing_( 1.0 ),
    z_spacing_( 1.0 ),
    read_header_( false ),
    swap_xy_spacing_( false )
  {
  }

  // READ_HEADER
  // Read the header of the dicom file and fill out its information into this private class
  bool read_header();

  // READ_DATA
  // Read the dicom data into this private class
  bool read_data();
  
  // READ_IMAGE
  bool read_image( const std::string& filename, char* buffer );
  

public:
  // Pointer to interface class
  GDCMLayerImporter* importer_;

  double rescale_slope_;
  double rescale_intercept_;
  unsigned long buffer_length_;
  unsigned long slice_data_size_;

  Core::GridTransform grid_transform_;
  Core::DataType pixel_type_;

  Core::Point origin_;
  Core::Vector row_direction_;
  Core::Vector col_direction_;
  Core::Vector slice_direction_;
  
  double x_spacing_;
  double y_spacing_;
  double z_spacing_;
  
  // Data block with actual data (generated by read_data)
  Core::DataBlockHandle data_block_;
  
  // Meta data (generated by read_data)
  LayerMetaData meta_data_;
  
  // Whether the header of the files has been parsed
  bool read_header_;
  
  // Whether to swap xy spacing
  bool swap_xy_spacing_;
};

bool GDCMLayerImporterPrivate::read_header()
{ 
  if ( this->read_header_ ) return true;

  // Get the filenames
  std::vector<std::string> filenames = this->importer_->get_filenames();
  if ( filenames.size() == 0 )
  {
    this->importer_->set_error( "No files were specified." );
    return false;
  }

  // Setup some compatibility rules for GDCM
  gdcm::ImageHelper::SetForcePixelSpacing( true );
  gdcm::ImageHelper::SetForceRescaleInterceptSlope( true );

  // Read the first file and extract information out of it;
  
  gdcm::ImageReader reader;
  reader.SetFileName( filenames[ 0 ].c_str() );
  if ( !reader.Read() )
  {
    this->importer_->set_error( std::string( "Cannot read file '" ) + filenames[ 0 ]+ "'." );
    return false;
  }
  
  const gdcm::Image& image = reader.GetImage();
  const gdcm::File& file = reader.GetFile();
  const gdcm::DataSet& ds = file.GetDataSet();
  const unsigned int* dims = image.GetDimensions();
  const gdcm::PixelFormat& pixeltype = image.GetPixelFormat();
  this->buffer_length_ = image.GetBufferLength();

  if ( pixeltype.GetSamplesPerPixel() != 1 )
  {
    this->importer_->set_error( "Unsupported pixel format." );
    return false;
  }

  unsigned int num_of_dimensions = image.GetNumberOfDimensions(); 
  if ( num_of_dimensions != 2 && num_of_dimensions != 3 )
  {
    this->importer_->set_error( "Unsupported number of dimensions." );
    return false;
  }
  
  this->grid_transform_.set_nx( dims[ 0 ] );
  this->grid_transform_.set_ny( dims[ 1 ] );
  
  if ( num_of_dimensions == 2 )
  {
    this->grid_transform_.set_nz( filenames.size() );
  }
  else
  {
    this->grid_transform_.set_nz( dims[ 2 ] );
  }

  this->rescale_intercept_ = image.GetIntercept();
  this->rescale_slope_ = image.GetSlope();

  gdcm::Rescaler rescaler;
  rescaler.SetIntercept( this->rescale_intercept_ );
  rescaler.SetSlope( this->rescale_slope_ );
  rescaler.SetPixelFormat( pixeltype );
  gdcm::PixelFormat::ScalarType output_pixel_type = rescaler.ComputeInterceptSlopePixelType();
  
  switch( output_pixel_type )
  {
    case gdcm::PixelFormat::INT8:
      this->pixel_type_ = Core::DataType::CHAR_E;
      break;
    case gdcm::PixelFormat::UINT8:
      this->pixel_type_ = Core::DataType::UCHAR_E;
      break;
    case gdcm::PixelFormat::INT12:
      this->pixel_type_ = Core::DataType::SHORT_E;
      break;
    case gdcm::PixelFormat::UINT12:
      this->pixel_type_ = Core::DataType::USHORT_E;
      break;
    case gdcm::PixelFormat::INT16:
      this->pixel_type_ = Core::DataType::SHORT_E;
      break;
    case gdcm::PixelFormat::UINT16:
      this->pixel_type_ = Core::DataType::USHORT_E;
      break;
    case gdcm::PixelFormat::INT32:
      this->pixel_type_ = Core::DataType::INT_E;
      break;
    case gdcm::PixelFormat::UINT32:
      this->pixel_type_ = Core::DataType::UINT_E;
      break;
    case gdcm::PixelFormat::FLOAT32:
      this->pixel_type_ = Core::DataType::FLOAT_E;
      break;
    case gdcm::PixelFormat::FLOAT64:
      this->pixel_type_ = Core::DataType::DOUBLE_E;
      break;
    default:
      this->importer_->set_error( "Encountered an unknown data type." );
      return false; 
  }

  double epsilon = std::numeric_limits<double>::epsilon() * 10.0;

  this->slice_data_size_ = GetSizeDataType( this->pixel_type_ ) * dims[ 0 ] * dims[ 1 ];

  // Compute the grid transform
  const double* spacing = image.GetSpacing();
  const double* origin = image.GetOrigin();
  const double* dircos = image.GetDirectionCosines();

  this->row_direction_ = Core::Vector( dircos[ 0 ], dircos[ 1 ], dircos[ 2 ] );
  this->col_direction_ = Core::Vector( dircos[ 3 ], dircos[ 4 ], dircos[ 5 ] );
  this->slice_direction_ = Core::Cross( this->row_direction_, this->col_direction_ );
  this->slice_direction_.normalize();
  this->row_direction_.normalize();
  this->col_direction_.normalize();

  this->origin_[ 0 ] = origin[ 0 ];
  this->origin_[ 1 ] = origin[ 1 ];
  this->origin_[ 2 ] = origin[ 2 ];

  this->x_spacing_ = spacing[ 0 ];
  this->y_spacing_ = spacing[ 1 ];
  
  gdcm::Tag slice_thickness_tag( 0x0018,0x0050 );
  gdcm::Tag patient_position_tag( 0x0020, 0x0032 );

  bool found_thickness = false;
  
  if( ds.FindDataElement( slice_thickness_tag ) ) // Slice Thickness
  {
    const gdcm::DataElement& de = ds.GetDataElement( slice_thickness_tag );
    if ( ! de.IsEmpty() )
    {
      gdcm::Attribute< 0x0018, 0x0050 > slice_thickness;
      slice_thickness.SetFromDataElement( de );
      double thickness = slice_thickness.GetValue( 0 );
      if ( thickness > epsilon )
      {
        this->z_spacing_ = thickness;
        found_thickness = true;
      }
    }
  }
  
  if ( filenames.size() > 1 && ds.FindDataElement( patient_position_tag ) )
  {
    gdcm::ImageReader reader2;
    reader2.SetFileName( filenames[ 1 ].c_str() );
    if ( !reader2.Read() )
    {
      this->importer_->set_error( "Can't read file " + filenames[ 1 ] );
      return false;
    }
    
    const gdcm::Image &image2 = reader2.GetImage();
    
    const double* origin2 = image2.GetOrigin();
    Core::Vector origin_vec( origin[ 0 ], origin[ 1 ], origin[ 2 ] );
    Core::Vector origin_vec2( origin2[ 0 ], origin2[ 1 ], origin2[ 2 ] );
    Core::Vector dir = origin_vec2 - origin_vec;
    
    double spacing = dir.length();
    if ( spacing < -epsilon || spacing > epsilon ) 
    {
      this->z_spacing_ = spacing; 
      this->slice_direction_ = dir; 
      this->slice_direction_.normalize();
    }
    else 
    {
      this->z_spacing_ = 1.0;
    }
  }
  else if ( found_thickness == false )
  {
    this->z_spacing_ = 1.0;
  }
  
  this->grid_transform_.load_basis( this->origin_, this->row_direction_ * this->x_spacing_,
    this->col_direction_ * this->y_spacing_, 
    this->slice_direction_ * this->z_spacing_ );
  this->grid_transform_.set_originally_node_centered( false );

  this->read_header_ = true;

  return true;
}

bool GDCMLayerImporterPrivate::read_data()
{
  if ( this->swap_xy_spacing_ )
  {
    std::swap( this->x_spacing_, this->y_spacing_ );
  }
  
  this->grid_transform_.load_basis( this->origin_, this->row_direction_ * this->x_spacing_,
    this->col_direction_ * this->y_spacing_, 
    this->slice_direction_ * this->z_spacing_ );

  this->grid_transform_.set_originally_node_centered( false );

  this->data_block_ = Core::StdDataBlock::New( this->grid_transform_, this->pixel_type_ );

  char* data = reinterpret_cast< char* >( this->data_block_->get_data() );
  std::vector<std::string> filenames = this->importer_->get_filenames();

  for ( size_t i = 0; i < filenames.size(); i++ )
  {
    if ( !this->read_image( filenames[ i ], data + this->slice_data_size_ * i ) )
    {
      this->data_block_.reset();
      return false;
    }
  }

  if ( filenames.size() )
  {
    InputFilesID inputfile_id = this->importer_->get_inputfiles_id();
    this->meta_data_.meta_data_ = Core::ExportToString( filenames ) + "|" +
      Core::ExportToString( inputfile_id );
    this->meta_data_.meta_data_info_ = "dicom_filename"; 
  }
  
  return true;
}

bool GDCMLayerImporterPrivate::read_image( const std::string& filename, char* buffer )
{
  gdcm::ImageReader reader;
  reader.SetFileName( filename.c_str() );
  
  if ( !reader.Read() )
  {
    this->importer_->set_error( "Failed to read file '" + filename + "'" );
    return false;
  }
  
  gdcm::Image& image = reader.GetImage();
  if ( this->buffer_length_ != image.GetBufferLength() )
  {
    this->importer_->set_error( "Images in the series have different sizes" );
    return false;
  }
  
  image.GetBuffer( buffer );
  const gdcm::PixelFormat& pixeltype = image.GetPixelFormat();
  if ( pixeltype == gdcm::PixelFormat::UINT12 )
  {
    if ( this->rescale_slope_ != 1.0 || this->rescale_intercept_ != 0.0 )
    {
      this->importer_->set_error( "Unsupported data format" );
      return false;
    }
    
    std::vector< char > copy( this->buffer_length_ );
    memcpy( &copy[ 0 ], buffer, this->buffer_length_ );
    if ( !gdcm::Unpacker12Bits::Unpack( buffer, &copy[ 0 ], this->buffer_length_ ) )
    {
      this->importer_->set_error( "Failed to unpack 12bit data" );
      return false;
    }
  }
  
  if ( this->rescale_slope_ != 1.0 || this->rescale_intercept_ != 0.0 )
  {
    gdcm::Rescaler rescaler;
    rescaler.SetIntercept( this->rescale_intercept_ );
    rescaler.SetSlope( this->rescale_slope_ );
    rescaler.SetPixelFormat( pixeltype );
    std::vector< char > copy( this->buffer_length_ );
    memcpy( &copy[ 0 ], buffer, this->buffer_length_ );
    rescaler.Rescale( buffer, &copy[ 0 ], this->buffer_length_ );
  }
  
  return true;
}

//////////////////////////////////////////////////////////////////////////
// Class GDCMLayerImporter
//////////////////////////////////////////////////////////////////////////

GDCMLayerImporter::GDCMLayerImporter() :
  private_( new GDCMLayerImporterPrivate )
{
  this->private_->importer_ = this;
}

GDCMLayerImporter::~GDCMLayerImporter()
{
}

void GDCMLayerImporter::set_dicom_swap_xyspacing_hint( bool swap_xy_spacing )
{
  this->private_->swap_xy_spacing_ = swap_xy_spacing;
}

bool GDCMLayerImporter::get_file_info( LayerImporterFileInfoHandle& info )
{
  try
  {
    // Read the header of the file
    if ( ! this->private_->read_header() ) return false;
    
    // Create an information structure with the properties of this file
    info = LayerImporterFileInfoHandle( new LayerImporterFileInfo );
    info->set_grid_transform( this->private_->grid_transform_ );
    info->set_data_type( this->private_->pixel_type_ );
    info->set_file_type( "dicom" );
    info->set_mask_compatible( false );
  }
  catch ( ... )
  {
    this->set_error( "Importer crashed when reading file." );
    return false;
  }

  return true;
}

bool GDCMLayerImporter::get_file_data( LayerImporterFileDataHandle& data )
{
  try
  { 
    // Read the data from the file
    if ( !this->private_->read_data() ) return false;
    
    // Create a data structure with handles to the actual data in this file     
    data = LayerImporterFileDataHandle( new LayerImporterFileData );
    data->set_data_block( this->private_->data_block_ );
    data->set_grid_transform( this->private_->grid_transform_ );
    data->set_meta_data( this->private_->meta_data_ );
    data->set_name( this->get_file_tag() );
  }
  catch ( ... )
  {
    this->set_error( "Importer crashed when reading file." );
    return false;
  }

  return true;
}

} // end namespace seg3D
