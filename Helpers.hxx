/*=========================================================================
 *
 *  Copyright David Doria 2011 daviddoria@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include "itkCastImageFilter.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIterator.h"
#include "itkImageFileWriter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkVectorCastImageFilter.h"
#include "itkMinimumMaximumImageCalculator.h"
#include "itkDerivativeImageFilter.h"
#include "itkNeighborhoodOperatorImageFilter.h"
#include "itkForwardDifferenceOperator.h"

namespace Helpers
{

template<typename TImage>
void DeepCopy(const TImage* const input, TImage* const output)
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
void DeepCopyVectorImage(const TImage* const input, TImage* const output)
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
void WriteImage(const TImage* const input, const std::string& filename)
{
  typedef  itk::ImageFileWriter< TImage > WriterType;
  typename WriterType::Pointer writer = WriterType::New();
  writer->SetFileName(filename);
  writer->SetInput(input);
  writer->Update();
}

template<typename TImage>
void WriteVectorImageAsPNG(const TImage* const input, const std::string& filename)
{
  /*
  typedef itk::VectorCastImageFilter< FloatVectorImageType, UnsignedCharVectorImageType > VectorCastImageFilterType;
  VectorCastImageFilterType::Pointer vectorCastImageFilter = VectorCastImageFilterType::New();
  vectorCastImageFilter->SetInput(input);
  vectorCastImageFilter->Update();
  */
  
  typedef itk::VectorImage<unsigned char, 2> UnsignedCharVectorImageType;
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
void ClampImage(TImage* const image)
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
void ClampVectorImage(TImage* const image)
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

template <class TImage>
float MinValue(const TImage* const image)
{
  typedef typename itk::MinimumMaximumImageCalculator<TImage>
          ImageCalculatorFilterType;

  typename ImageCalculatorFilterType::Pointer imageCalculatorFilter
          = ImageCalculatorFilterType::New ();
  imageCalculatorFilter->SetImage(image);
  imageCalculatorFilter->Compute();

  return imageCalculatorFilter->GetMinimum();
}

template <class TImage>
float MaxValue(const TImage* const image)
{
  typedef typename itk::MinimumMaximumImageCalculator<TImage>
          ImageCalculatorFilterType;

  typename ImageCalculatorFilterType::Pointer imageCalculatorFilter
          = ImageCalculatorFilterType::New ();
  imageCalculatorFilter->SetImage(image);
  imageCalculatorFilter->Compute();

  return imageCalculatorFilter->GetMaximum();
}

template <class TImage>
void CentralDifferenceDerivative(const TImage* const image, const unsigned int direction, TImage* const output)
{
  typedef itk::DerivativeImageFilter<TImage, TImage> DerivativeImageFilterType;
  typename DerivativeImageFilterType::Pointer derivativeFilter = DerivativeImageFilterType::New();
  derivativeFilter->SetDirection(direction);
  derivativeFilter->SetOrder(1);
  derivativeFilter->SetInput(image);
  derivativeFilter->Update();

  DeepCopy(derivativeFilter->GetOutput(), output);
}

template <class TImage>
void ForwardDifferenceDerivative(const TImage* const image, const unsigned int direction, TImage* const output)
{
  typedef itk::ForwardDifferenceOperator<float, 2> OperatorType;
  itk::Size<2> radius;
  radius.Fill(1); // a radius of 1x1 creates a 3x3 operator
  
  OperatorType derivativeOperator;
  derivativeOperator.SetDirection(direction);
  derivativeOperator.CreateToRadius(radius);

  typedef itk::NeighborhoodOperatorImageFilter<TImage, TImage> NeighborhoodOperatorImageFilterType;
  typename NeighborhoodOperatorImageFilterType::Pointer derivativeFilter = NeighborhoodOperatorImageFilterType::New();
  derivativeFilter->SetOperator(derivativeOperator);
  derivativeFilter->SetInput(image);
  derivativeFilter->Update();

  DeepCopy(derivativeFilter->GetOutput(), output);
}

}// end namespace
