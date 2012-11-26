/*=========================================================================
 *
 *  Copyright David Doria 2012 daviddoria@gmail.com
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

#include "PoissonEditing.h"

// Submodules
#include "Mask/ITKHelpers/ITKHelpers.h"

// STL
#include <iostream>

// ITK
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCastImageFilter.h"
#include "itkVectorIndexSelectionCastImageFilter.h"
#include "itkComposeImageFilter.h"

int main(int argc, char* argv[])
{
  // Verify arguments
  if(argc < 5)
    {
    std::cout << "Usage: ImageToFill mask guidanceField outputImage" << std::endl;
    return EXIT_FAILURE;
    }

  // Parse arguments
  std::string targetImageFilename = argv[1];
  std::string maskFilename = argv[2];
  std::string guidanceFieldFilename = argv[3];
  std::string outputFilename = argv[4];

  // Output arguments
  std::cout << "Target image: " << targetImageFilename << std::endl
            << "Mask image: " << maskFilename << std::endl
            << "Guidance field: " << guidanceFieldFilename << std::endl
            << "Output image: " << outputFilename << std::endl;

  itk::ImageIOBase::Pointer imageIO =
  itk::ImageIOFactory::CreateImageIO(
      targetImageFilename.c_str(), itk::ImageIOFactory::ReadMode);
  imageIO->ReadImageInformation();
  typedef itk::ImageIOBase::IOComponentType ScalarPixelType;
  const ScalarPixelType pixelType = imageIO->GetComponentType();
  std::cout << "Pixel Type is " << imageIO->GetComponentTypeAsString(pixelType)
            << std::endl;

  typedef itk::VectorImage<float, 2> ImageType;
  //typedef itk::Image<float, 2> ImageType;

  // Read images
  typedef itk::ImageFileReader<ImageType> ImageReaderType;
  ImageReaderType::Pointer targetImageReader = ImageReaderType::New();
  targetImageReader->SetFileName(targetImageFilename);
  targetImageReader->Update();

  std::cout << "Read target image." << std::endl;

  // Read mask
  typedef itk::ImageFileReader<Mask> MaskReaderType;
  MaskReaderType::Pointer maskReader = MaskReaderType::New();
  maskReader->SetFileName(maskFilename);
  maskReader->Update();

  std::cout << "Read mask." << std::endl;

  typedef itk::CovariantVector<float, 2> Vector2Type;
  typedef itk::Image<Vector2Type, 2> Vector2ImageType;
  typedef itk::ImageFileReader<Vector2ImageType> GuidanceFieldReaderType;

  GuidanceFieldReaderType::Pointer guidanceFieldReader = GuidanceFieldReaderType::New();
  guidanceFieldReader->SetFileName(guidanceFieldFilename);
  guidanceFieldReader->Update();

  std::cout << "Read guidance field." << std::endl;

//   typedef PoissonEditing<float> PoissonEditingFilterType;
//   PoissonEditingFilterType poissonFilter;
//   poissonFilter.SetTargetImage(targetImageReader->GetOutput());
//   poissonFilter.SetGuidanceField(guidanceFieldReader->GetOutput());
//   poissonFilter.SetMask(maskReader->GetOutput());
//   poissonFilter.FillMaskedRegionPoisson();

  std::vector<Vector2ImageType*> guidanceFields(
         targetImageReader->GetOutput()->GetNumberOfComponentsPerPixel(),
         guidanceFieldReader->GetOutput());

  ImageType::Pointer output = ImageType::New();

  PoissonEditing<float>::FillImage(targetImageReader->GetOutput(), maskReader->GetOutput(),
                                   guidanceFields, output.GetPointer(),
                                   targetImageReader->GetOutput()->GetLargestPossibleRegion());

  // Write output
  if(pixelType == itk::ImageIOBase::UCHAR)
  {
    ITKHelpers::WriteRGBImage(output.GetPointer(), outputFilename);
  }
  else
  {
    ITKHelpers::WriteImage(output.GetPointer(), outputFilename);
  }

  return EXIT_SUCCESS;
}
