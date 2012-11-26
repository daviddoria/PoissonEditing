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

// STL
#include <iostream>

// ITK
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCastImageFilter.h"
#include "itkVectorIndexSelectionCastImageFilter.h"
#include "itkComposeImageFilter.h"

// Submodules
#include "ITKHelpers/ITKContainerInterface.h"

int main(int argc, char* argv[])
{
  // Verify arguments
  if(argc < 5)
  {
    std::cout << "Usage: ImageToFill sourceImage sourceImageMask outputImage" << std::endl;
    return EXIT_FAILURE;
  }

  // Parse arguments
  std::string targetImageFilename = argv[1];
  std::string sourceImageFilename = argv[2];
  std::string sourceImageMaskFilename = argv[3];
  std::string outputFilename = argv[4];

  // Output arguments
  std::cout << "Target image: " << targetImageFilename << std::endl
            << "Source image: " << sourceImageFilename << std::endl
            << "Source image mask: " << sourceImageMaskFilename << std::endl
            << "Output image: " << outputFilename << std::endl;

  typedef itk::VectorImage<float, 2> ImageType;

  // Read images
  typedef itk::ImageFileReader<ImageType> ImageReaderType;
  ImageReaderType::Pointer targetImageReader = ImageReaderType::New();
  targetImageReader->SetFileName(targetImageFilename);
  targetImageReader->Update();

  // Read mask
  Mask::Pointer mask = Mask::New();
//  mask->Read(maskFilename);
  mask->ReadFromImage(sourceImageMaskFilename);

  ImageReaderType::Pointer sourceImageReader = ImageReaderType::New();
  sourceImageReader->SetFileName(sourceImageFilename);
  sourceImageReader->Update();

  typedef PoissonEditing<float> PoissonEditingType;

  PoissonEditingType::GuidanceFieldType::Pointer guidanceField =
      PoissonEditingType::GuidanceFieldType::New();
  ITKHelpers::ComputeGradients(sourceImageReader->GetOutput(), guidanceField.GetPointer());

//   // Output image properties
//   std::cout << "Source image: " << sourceImageReader->GetOutput()->GetLargestPossibleRegion().GetSize() << std::endl
//             << "Target image: " << targetImageReader->GetOutput()->GetLargestPossibleRegion().GetSize() << std::endl
//             << "Mask image: " << mask->GetLargestPossibleRegion().GetSize() << std::endl;

  ImageType::Pointer output = ImageType::New();


  PoissonEditingType::FillImage(targetImageReader->GetOutput(), mask,
                                guidanceField.GetPointer(), output.GetPointer());

//  ITKHelpers::ScaleAllChannelsTo255(output.GetPointer());
  ITKHelpers::ClampAllChannelsTo255(output.GetPointer());

  // Write output
  if(Helpers::GetFileExtension(outputFilename) == "png")
  {
    ITKHelpers::WriteRGBImage(output.GetPointer(), outputFilename);
  }
  else
  {
    ITKHelpers::WriteImage(output.GetPointer(), outputFilename);
  }

  return EXIT_SUCCESS;
}
