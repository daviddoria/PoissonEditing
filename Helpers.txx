#include "itkCastImageFilter.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIterator.h"
#include "itkImageFileWriter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkVectorCastImageFilter.h"

namespace Helpers
{

template<typename TImage>
void DeepCopy(typename TImage::Pointer input, typename TImage::Pointer output)
{
  output->SetRegions(input->GetLargestPossibleRegion());
  output->Allocate();

  itk::ImageRegionConstIterator<TImage> inputIterator(input, input->GetLargestPossibleRegion());
  itk::ImageRegionIterator<TImage> outputIterator(output, output->GetLargestPossibleRegion());

  while(!inputIterator.IsAtEnd())
    {
    outputIterator.Set(inputIterator.Get());
    ++inputIterator;
    ++outputIterator;
    }
}

template<typename TImage>
void DeepCopyVectorImage(typename TImage::Pointer input, typename TImage::Pointer output)
{
  output->SetRegions(input->GetLargestPossibleRegion());
  output->SetNumberOfComponentsPerPixel(input->GetNumberOfComponentsPerPixel());
  output->Allocate();

  itk::ImageRegionConstIterator<TImage> inputIterator(input, input->GetLargestPossibleRegion());
  itk::ImageRegionIterator<TImage> outputIterator(output, output->GetLargestPossibleRegion());

  while(!inputIterator.IsAtEnd())
    {
    outputIterator.Set(inputIterator.Get());
    ++inputIterator;
    ++outputIterator;
    }
}

template<typename TImage>
void ExtractComponent(typename TImage::Pointer input, unsigned int component, FloatScalarImageType::Pointer output)
//void ExtractComponent(typename TImage::Pointer input, unsigned int component, typename itk::Image<typename TImage::PixelType, 2>::Pointer output)
{
  /*
  // non vector images
  if(component >= TImage::PixelType::Dimension)
    {
    std::cerr << "Component is " << component << " but must be less than " << TImage::PixelType::Dimension << std::endl;
    exit(-1);
    }
  */

  if(component >= input->GetNumberOfComponentsPerPixel())
    {
    std::cerr << "Component is " << component << " but must be less than " << input->GetNumberOfComponentsPerPixel() << std::endl;
    exit(-1);
    }
  output->SetRegions(input->GetLargestPossibleRegion());
  output->Allocate();

  itk::ImageRegionConstIterator<TImage> inputIterator(input, input->GetLargestPossibleRegion());
  //itk::ImageRegionIterator<itk::Image<typename TImage::PixelType, 2> > outputIterator(output, output->GetLargestPossibleRegion());
  itk::ImageRegionIterator<FloatScalarImageType> outputIterator(output, output->GetLargestPossibleRegion());

  while(!inputIterator.IsAtEnd())
    {
    outputIterator.Set(inputIterator.Get()[component]);
    ++inputIterator;
    ++outputIterator;
    }
}


template<typename TImage>
void SetComponent(typename TImage::Pointer input, unsigned int component, FloatScalarImageType::Pointer componentImage)
{
  /*
  if(component >= TImage::PixelType::Dimension)
    {
    std::cerr << "Component is " << component << " but must be less than " << TImage::PixelType::Dimension << std::endl;
    exit(-1);
    }
  */

  if(component >= input->GetNumberOfComponentsPerPixel())
    {
    std::cerr << "Component is " << component << " but must be less than " << input->GetNumberOfComponentsPerPixel() << std::endl;
    exit(-1);
    }

  itk::ImageRegionIterator<TImage> inputIterator(input, input->GetLargestPossibleRegion());
  itk::ImageRegionConstIterator<FloatScalarImageType> componentIterator(componentImage, componentImage->GetLargestPossibleRegion());

  while(!inputIterator.IsAtEnd())
    {
    typename TImage::PixelType pixel = inputIterator.Get();
    pixel[component] = componentIterator.Get();
    inputIterator.Set(pixel);
    ++inputIterator;
    ++componentIterator;
    }
}



template<typename TImage>
void CastAndWriteScalarImage(typename TImage::Pointer input, std::string filename)
{
  typedef itk::CastImageFilter< TImage, UnsignedCharScalarImageType > CastFilterType;
  typename CastFilterType::Pointer castFilter = CastFilterType::New();
  castFilter->SetInput(input);
  castFilter->Update();

  typedef  itk::ImageFileWriter< UnsignedCharScalarImageType > PNGWriterType;
  PNGWriterType::Pointer pngWriter = PNGWriterType::New();
  pngWriter->SetFileName(filename);
  pngWriter->SetInput(castFilter->GetOutput());
  pngWriter->Update();
}


template<typename TImage>
void ScaleAndWriteScalarImage(typename TImage::Pointer input, std::string filename)
{
  typedef itk::RescaleIntensityImageFilter< TImage, UnsignedCharScalarImageType > RescaleFilterType;
  typename RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
  rescaleFilter->SetInput(input);
  rescaleFilter->SetOutputMinimum(0);
  rescaleFilter->SetOutputMaximum(255);
  rescaleFilter->Update();

  typedef  itk::ImageFileWriter< UnsignedCharScalarImageType > PNGWriterType;
  PNGWriterType::Pointer pngWriter = PNGWriterType::New();
  pngWriter->SetFileName(filename);
  pngWriter->SetInput(rescaleFilter->GetOutput());
  pngWriter->Update();
}


template<typename TImage>
void WriteImage(typename TImage::Pointer input, std::string filename)
{
  typedef  itk::ImageFileWriter< TImage > WriterType;
  typename WriterType::Pointer writer = WriterType::New();
  writer->SetFileName(filename);
  writer->SetInput(input);
  writer->Update();
}

template<typename TImage>
void WriteVectorImageAsPNG(typename TImage::Pointer input, std::string filename)
{
  /*
  typedef itk::VectorCastImageFilter< FloatVectorImageType, UnsignedCharVectorImageType > VectorCastImageFilterType;
  VectorCastImageFilterType::Pointer vectorCastImageFilter = VectorCastImageFilterType::New();
  vectorCastImageFilter->SetInput(input);
  vectorCastImageFilter->Update();
  */
  typedef itk::CastImageFilter< TImage, UnsignedCharVectorImageType > CastImageFilterType;
  typename CastImageFilterType::Pointer castImageFilter = CastImageFilterType::New();
  castImageFilter->SetInput(input);
  castImageFilter->Update();
  
  typedef  itk::ImageFileWriter< UnsignedCharVectorImageType > WriterType;
  typename WriterType::Pointer writer = WriterType::New();
  writer->SetFileName(filename);
  writer->SetInput(castImageFilter->GetOutput());
  writer->Update();
}

template<typename TImage>
void ClampImage(typename TImage::Pointer image)
{
  itk::ImageRegionIterator<TImage> imageIterator(image, image->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    typename TImage::PixelType pixel = imageIterator.Get();

    for(unsigned int i = 0; i < TImage::PixelType::Dimension; i++)
      {
      if(pixel[i] < 0)
        {
        pixel[i] = 0;
        }
      if(pixel[i] > 255)
        {
        pixel[i] = 255;
        }
      }// end for components

    imageIterator.Set(pixel);
    ++imageIterator;
    }// end iterator while
}


template<typename TImage>
void ClampVectorImage(typename TImage::Pointer image)
{
  itk::ImageRegionIterator<TImage> imageIterator(image, image->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    typename TImage::PixelType pixel = imageIterator.Get();

    for(unsigned int i = 0; i < image->GetNumberOfComponentsPerPixel(); i++)
      {
      if(pixel[i] < 0)
        {
        pixel[i] = 0;
        }
      if(pixel[i] > 255)
        {
        pixel[i] = 255;
        }
      }// end for components

    imageIterator.Set(pixel);
    ++imageIterator;
    }// end iterator while
}


}// end namespace