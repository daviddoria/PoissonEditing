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
#include "PoissonEditingWrappers.h"

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
  if(argc < 4)
  {
    std::cout << "Usage: ImageToFill mask outputImage" << std::endl;
    return EXIT_FAILURE;
  }

  // Parse arguments
  std::string targetImageFilename = argv[1];
  std::string maskFilename = argv[2];
  std::string outputFilename = argv[3];

  // Output arguments
  std::cout << "Target image: " << targetImageFilename << std::endl
            << "Mask image: " << maskFilename << std::endl
            << "Output image: " << outputFilename << std::endl;

  typedef itk::VectorImage<float, 2> ImageType;

  // Read images
  typedef itk::ImageFileReader<ImageType> ImageReaderType;
  ImageReaderType::Pointer targetImageReader = ImageReaderType::New();
  targetImageReader->SetFileName(targetImageFilename);
  targetImageReader->Update();

  std::cout << "Finished reading target image." << std::endl;

  // Read mask
  Mask::Pointer mask = Mask::New();
//  mask->Read(maskFilename);
  mask->ReadFromImage(maskFilename);

  std::cout << "Read mask." << std::endl;

  typedef PoissonEditingParent::GuidanceFieldType GuidanceFieldType;
  GuidanceFieldType::Pointer zeroGuidanceField = GuidanceFieldType::New();
  zeroGuidanceField->SetRegions(targetImageReader->GetOutput()->GetLargestPossibleRegion());
  zeroGuidanceField->Allocate();
  GuidanceFieldType::PixelType zeroVector;
  zeroVector.Fill(0);
  zeroGuidanceField->FillBuffer(zeroVector);

  ImageType::Pointer output = ImageType::New();

  FillImage(targetImageReader->GetOutput(), mask,
            zeroGuidanceField.GetPointer(), output.GetPointer(),
            targetImageReader->GetOutput()->GetLargestPossibleRegion());

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
