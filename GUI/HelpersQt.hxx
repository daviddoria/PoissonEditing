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

// ITK
#include "itkRegionOfInterestImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkVectorImage.h"
#include "itkVectorMagnitudeImageFilter.h"
#include "itkVectorIndexSelectionCastImageFilter.h"

// Qt
#include <QColor>

// Custom
#include "Helpers.h"

namespace HelpersQt
{

template <typename TImage>
QImage GetQImage(const TImage* const image)
{
  return GetQImage(image, image->GetLargestPossibleRegion());
}

template <typename TImage>
QImage GetQImage(const TImage* const image, const itk::ImageRegion<2>& region)
{
  if(image->GetNumberOfComponentsPerPixel() == 3)
  {
    std::cout << "GetQImage: Calling GetQImageRGB." << std::endl;
    return GetQImageRGB(image, region);
  }
  else if(image->GetNumberOfComponentsPerPixel() == 1)
  {
    typedef itk::Image<float, 2> FloatImageType;
//     FloatImageType::Pointer scalarImage = FloatImageType::New();
//     Helpers::DeepCopy(image, scalarImage.GetPointer());

    typedef itk::VectorIndexSelectionCastImageFilter<TImage, FloatImageType> IndexSelectionType;
    typename IndexSelectionType::Pointer indexSelectionFilter = IndexSelectionType::New();
    indexSelectionFilter->SetIndex(0);
    indexSelectionFilter->SetInput(image);
    indexSelectionFilter->Update();

    float minValue = Helpers::MinValue(indexSelectionFilter->GetOutput());
    float maxValue = Helpers::MaxValue(indexSelectionFilter->GetOutput());
    float valueRange = maxValue - minValue;
    if(valueRange < 20)
    {
      std::cout << "GetQImage: Calling GetQImageScaled because range too small." << std::endl;
      return GetQImageScaled(indexSelectionFilter->GetOutput(), region);
    }

    if(minValue < 0)
    {
      std::cout << "GetQImage: Calling GetQImageScaled because minValue < 0." << std::endl;
      return GetQImageScaled(indexSelectionFilter->GetOutput(), region);
    }

    if(maxValue > 255)
    {
      std::cout << "GetQImage: Calling GetQImageScaled because minValue < 0." << std::endl;
      return GetQImageScaled(indexSelectionFilter->GetOutput(), region);
    }

    std::cout << "GetQImage: Calling GetQImageScalar (value range = " << valueRange << "." << std::endl;
    return GetQImageScalar(indexSelectionFilter->GetOutput(), region);
  }
  else
  {
    std::cout << "GetQImage: Calling GetQImageMagnitude." << std::endl;
    return GetQImageMagnitude(image, region);
  }
}

template <typename TImage>
QImage GetQImageRGB(const TImage* const image)
{
  return GetQImageRGB<TImage>(image, image->GetLargestPossibleRegion());
}

template <typename TImage>
QImage GetQImageRGB(const TImage* const image, const itk::ImageRegion<2>& region)
{
  QImage qimage(region.GetSize()[0], region.GetSize()[1], QImage::Format_RGB888);

  typedef itk::RegionOfInterestImageFilter< TImage, TImage > RegionOfInterestImageFilterType;
  typename RegionOfInterestImageFilterType::Pointer regionOfInterestImageFilter = RegionOfInterestImageFilterType::New();
  regionOfInterestImageFilter->SetRegionOfInterest(region);
  regionOfInterestImageFilter->SetInput(image);
  regionOfInterestImageFilter->Update();
  
  itk::ImageRegionIterator<TImage> imageIterator(regionOfInterestImageFilter->GetOutput(), regionOfInterestImageFilter->GetOutput()->GetLargestPossibleRegion());
  
  while(!imageIterator.IsAtEnd())
    {
    typename TImage::PixelType pixel = imageIterator.Get();

    itk::Index<2> index = imageIterator.GetIndex();
    int r = static_cast<int>(pixel[0]);
    int g = static_cast<int>(pixel[1]);
    int b = static_cast<int>(pixel[2]);
    QColor pixelColor(r,g,b);
    if(r > 255 || r < 0 || g > 255 || g < 0 || b > 255 || b < 0)
      {
      std::cout << "Can't set r,g,b to " << r << " " << g << " " << b << std::endl;
      }
    qimage.setPixel(index[0], index[1], pixelColor.rgb());

    ++imageIterator;
    }

  QColor highlightColor(255, 0, 255);
  qimage.setPixel(region.GetSize()[0]/2, region.GetSize()[1]/2, highlightColor.rgb());
  
  //return qimage; // The actual image region
  return qimage.mirrored(false, true); // The flipped image region
}


template <typename TImage>
QImage GetQImageRGBA(const TImage* const image)
{
  return GetQImageRGBA<TImage>(image, image->GetLargestPossibleRegion());
}

template <typename TImage>
QImage GetQImageRGBA(const TImage* const image, const itk::ImageRegion<2>& region)
{
  QImage qimage(region.GetSize()[0], region.GetSize()[1], QImage::Format_ARGB32);

  typedef itk::RegionOfInterestImageFilter< TImage, TImage > RegionOfInterestImageFilterType;
  typename RegionOfInterestImageFilterType::Pointer regionOfInterestImageFilter = RegionOfInterestImageFilterType::New();
  regionOfInterestImageFilter->SetRegionOfInterest(region);
  regionOfInterestImageFilter->SetInput(image);
  regionOfInterestImageFilter->Update();
  
  itk::ImageRegionIterator<TImage> imageIterator(regionOfInterestImageFilter->GetOutput(), regionOfInterestImageFilter->GetOutput()->GetLargestPossibleRegion());
  
  while(!imageIterator.IsAtEnd())
    {
    typename TImage::PixelType pixel = imageIterator.Get();

    itk::Index<2> index = imageIterator.GetIndex();
    int r = static_cast<int>(pixel[0]);
    int g = static_cast<int>(pixel[1]);
    int b = static_cast<int>(pixel[2]);
    QColor pixelColor(r,g,b,255); // opaque
    if(r > 255 || r < 0 || g > 255 || g < 0 || b > 255 || b < 0)
      {
      std::cout << "Can't set r,g,b to " << r << " " << g << " " << b << std::endl;
      }
    qimage.setPixel(index[0], index[1], pixelColor.rgba());

    ++imageIterator;
    }
  
  //return qimage; // The actual image region
  return qimage.mirrored(false, true); // The flipped image region
}

template <typename TImage>
QImage GetQImageMagnitude(const TImage* const image)
{
  return GetQImageMagnitude<TImage>(image, image->GetLargestPossibleRegion());
}

template <typename TImage>
QImage GetQImageMagnitude(const TImage* const image, const itk::ImageRegion<2>& region)
{
  typedef itk::Image<typename TImage::InternalPixelType> ScalarImageType;
  
  QImage qimage(region.GetSize()[0], region.GetSize()[1], QImage::Format_RGB888);

  typedef itk::VectorMagnitudeImageFilter<TImage, ScalarImageType>  VectorMagnitudeFilterType;
  typename VectorMagnitudeFilterType::Pointer magnitudeFilter = VectorMagnitudeFilterType::New();
  magnitudeFilter->SetInput(image);
  magnitudeFilter->Update();

  typedef itk::VectorImage<unsigned char, 2> UnsignedCharVectorImageType;
  typedef itk::Image<unsigned char, 2> UnsignedCharScalarImageType;
  typedef itk::RescaleIntensityImageFilter<ScalarImageType, UnsignedCharScalarImageType> RescaleFilterType;
  typename RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
  rescaleFilter->SetOutputMinimum(0);
  rescaleFilter->SetOutputMaximum(255);
  rescaleFilter->SetInput( magnitudeFilter->GetOutput() );
  rescaleFilter->Update();
  
  typedef itk::RegionOfInterestImageFilter< UnsignedCharScalarImageType, UnsignedCharScalarImageType> RegionOfInterestImageFilterType;
  typename RegionOfInterestImageFilterType::Pointer regionOfInterestImageFilter = RegionOfInterestImageFilterType::New();
  regionOfInterestImageFilter->SetRegionOfInterest(region);
  regionOfInterestImageFilter->SetInput(rescaleFilter->GetOutput());
  regionOfInterestImageFilter->Update();

  itk::ImageRegionConstIteratorWithIndex<UnsignedCharScalarImageType> imageIterator(regionOfInterestImageFilter->GetOutput(),
                                                                                    regionOfInterestImageFilter->GetOutput()->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    unsigned char pixelValue = imageIterator.Get();

    QColor pixelColor(static_cast<int>(pixelValue), static_cast<int>(pixelValue), static_cast<int>(pixelValue));

    itk::Index<2> index = imageIterator.GetIndex();
    qimage.setPixel(index[0], index[1], pixelColor.rgb());

    ++imageIterator;
    }

  //return qimage; // The actual image region
  return qimage.mirrored(false, true); // The flipped image region
}

template <typename TImage>
QImage GetQImageScaled(const TImage* const image)
{
  return GetQImageScaled(image, image->GetLargestPossibleRegion());
}

template <typename TImage>
QImage GetQImageScaled(const TImage* const image, const itk::ImageRegion<2>& region)
{
  QImage qimage(region.GetSize()[0], region.GetSize()[1], QImage::Format_RGB888);

  typedef itk::RegionOfInterestImageFilter< TImage, TImage> RegionOfInterestImageFilterType;
  typename RegionOfInterestImageFilterType::Pointer regionOfInterestImageFilter = RegionOfInterestImageFilterType::New();
  regionOfInterestImageFilter->SetRegionOfInterest(region);
  regionOfInterestImageFilter->SetInput(image);
  regionOfInterestImageFilter->Update();

  typedef itk::RescaleIntensityImageFilter< TImage, TImage > RescaleFilterType;
  typename RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
  rescaleFilter->SetInput(regionOfInterestImageFilter->GetOutput());
  rescaleFilter->SetOutputMinimum(0);
  rescaleFilter->SetOutputMaximum(255);
  rescaleFilter->Update();
  
  itk::ImageRegionConstIteratorWithIndex<TImage> imageIterator(rescaleFilter->GetOutput(), rescaleFilter->GetOutput()->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    typename TImage::PixelType pixelValue = imageIterator.Get();

    QColor pixelColor(static_cast<int>(pixelValue), static_cast<int>(pixelValue), static_cast<int>(pixelValue));

    itk::Index<2> index = imageIterator.GetIndex();
    qimage.setPixel(index[0], index[1], pixelColor.rgb());

    ++imageIterator;
    }

  //return qimage; // The actual image region
  return qimage.mirrored(false, true); // The flipped image region
}

template <typename TImage>
QImage GetQImageScalar(const TImage* const image)
{
  return GetQImageScalar(image, image->GetLargestPossibleRegion());
}

template <typename TImage>
QImage GetQImageScalar(const TImage* const image, const itk::ImageRegion<2>& region)
{
  QImage qimage(region.GetSize()[0], region.GetSize()[1], QImage::Format_RGB888);

  typedef itk::RegionOfInterestImageFilter< TImage, TImage> RegionOfInterestImageFilterType;
  typename RegionOfInterestImageFilterType::Pointer regionOfInterestImageFilter = RegionOfInterestImageFilterType::New();
  regionOfInterestImageFilter->SetRegionOfInterest(region);
  regionOfInterestImageFilter->SetInput(image);
  regionOfInterestImageFilter->Update();

  itk::ImageRegionIterator<TImage> imageIterator(regionOfInterestImageFilter->GetOutput(), regionOfInterestImageFilter->GetOutput()->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    typename TImage::PixelType pixelValue = imageIterator.Get();

    QColor pixelColor(static_cast<int>(pixelValue), static_cast<int>(pixelValue), static_cast<int>(pixelValue));

    itk::Index<2> index = imageIterator.GetIndex();
    qimage.setPixel(index[0], index[1], pixelColor.rgb());

    ++imageIterator;
    }

  //return qimage; // The actual image region
  return qimage.mirrored(false, true); // The flipped image region
}

template <typename TImage>
QImage GetQImageMasked(const TImage* const image, const Mask::Pointer mask)
{
  if(image->GetLargestPossibleRegion() != mask->GetLargestPossibleRegion())
    {
    std::cerr << "Image and mask size do not match!" << std::endl;
    exit(-1);
    }
  return GetQImageMasked<TImage>(image, mask, image->GetLargestPossibleRegion());
}

template <typename TImage>
QImage GetQImageMasked(const TImage* const image, const Mask* const mask, const itk::ImageRegion<2>& region)
{
  QImage qimage(region.GetSize()[0], region.GetSize()[1], QImage::Format_RGB888);

  typedef itk::RegionOfInterestImageFilter< TImage, TImage > RegionOfInterestImageFilterType;
  typename RegionOfInterestImageFilterType::Pointer regionOfInterestImageFilter = RegionOfInterestImageFilterType::New();
  regionOfInterestImageFilter->SetRegionOfInterest(region);
  regionOfInterestImageFilter->SetInput(image);
  regionOfInterestImageFilter->Update();

  typedef itk::RegionOfInterestImageFilter< Mask, Mask> RegionOfInterestMaskFilterType;
  typename RegionOfInterestMaskFilterType::Pointer regionOfInterestMaskFilter = RegionOfInterestMaskFilterType::New();
  regionOfInterestMaskFilter->SetRegionOfInterest(region);
  regionOfInterestMaskFilter->SetInput(mask);
  regionOfInterestMaskFilter->Update();
  
  itk::ImageRegionIterator<TImage> imageIterator(regionOfInterestImageFilter->GetOutput(), regionOfInterestImageFilter->GetOutput()->GetLargestPossibleRegion());

  QColor holeColor(0, 255, 0);
  
  while(!imageIterator.IsAtEnd())
    {
    typename TImage::PixelType pixel = imageIterator.Get();

    itk::Index<2> index = imageIterator.GetIndex();

    if(regionOfInterestMaskFilter->GetOutput()->IsHole(index))
      {
      qimage.setPixel(index[0], index[1], holeColor.rgb());
      }
    else
      {
      QColor pixelColor(static_cast<int>(pixel[0]), static_cast<int>(pixel[1]), static_cast<int>(pixel[2]));
      qimage.setPixel(index[0], index[1], pixelColor.rgb());
      }

    ++imageIterator;
    }

  QColor highlightColor(255, 0, 255);
  qimage.setPixel(region.GetSize()[0]/2, region.GetSize()[1]/2, highlightColor.rgb());
  
  //return qimage; // The actual image region
  return qimage.mirrored(false, true); // The flipped image region
}

} // end namespace
