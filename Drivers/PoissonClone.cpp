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
    std::cout << "Usage: ./PoissonClone TargetImage SourceImage SourceImageMask OutputImage" << std::endl;
    std::cout << "argc = " << argc << std::endl;
    std::cout << "Provided arguments were: ";
    for(int i = 1; i < argc; ++i)
    {
      std::cout << argv[i] << " ";
    }
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

//  typedef itk::VectorImage<float, 2> ImageType;
  typedef itk::Image<itk::CovariantVector<float, 3>, 2> ImageType;

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

  if(sourceImageReader->GetOutput()->GetNumberOfComponentsPerPixel() !=
     targetImageReader->GetOutput()->GetNumberOfComponentsPerPixel())
  {
    throw std::runtime_error("The source and target images must have the same number of channels!");
  }

  if(!targetImageReader->GetOutput()->GetLargestPossibleRegion().IsInside(sourceImageReader->GetOutput()->GetLargestPossibleRegion()))
  {
    throw std::runtime_error("The target image must be larger than the source image!");
  }

  // Setup where the source image should appear in the target image
  itk::ImageRegion<2> regionToProcess = sourceImageReader->GetOutput()->GetLargestPossibleRegion();
//  itk::Offset<2> offset = {{50,0}}; // This moves the image that gets composited into the target image 50 pixels to the right (the default is in the top left corner of the target image)
  itk::Offset<2> offset = {{0,0}};
  regionToProcess.SetIndex(regionToProcess.GetIndex() + offset);
  std::cout << "Processing region " << regionToProcess << std::endl;

  typedef PoissonEditing<float> PoissonEditingType;

  // Compute the gradient of each channel to use as the guidance fields for each channel
  std::vector<PoissonEditingType::GuidanceFieldType::Pointer> guidanceFields;

  for(unsigned int channel = 0; channel < sourceImageReader->GetOutput()->GetNumberOfComponentsPerPixel(); ++channel)
  {
    PoissonEditingType::GuidanceFieldType::Pointer guidanceField =
        PoissonEditingType::GuidanceFieldType::New();
    guidanceField->SetRegions(sourceImageReader->GetOutput()->GetLargestPossibleRegion());
    guidanceField->Allocate();
    guidanceFields.push_back(guidanceField);

    typedef itk::Image<ImageType::PixelType::ComponentType, 2> ScalarImageType;
    typename ScalarImageType::Pointer imageChannel = ScalarImageType::New();
    imageChannel->SetRegions(sourceImageReader->GetOutput()->GetLargestPossibleRegion());
    imageChannel->Allocate();
    ITKHelpers::ExtractChannel(sourceImageReader->GetOutput(), channel, imageChannel.GetPointer());

    ITKHelpers::ComputeGradients(imageChannel.GetPointer(), guidanceField.GetPointer());
  }

  ImageType::Pointer output = ImageType::New();

  PoissonEditingType::FillImage(targetImageReader->GetOutput(), mask,
                                guidanceFields, output.GetPointer(), regionToProcess);

  // Make sure the output is in the valid pixel value range
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
