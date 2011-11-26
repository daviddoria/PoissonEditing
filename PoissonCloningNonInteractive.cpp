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

#include "PoissonCloning.h"

// STL
#include <iostream>

// ITK
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCastImageFilter.h"
#include "itkVectorIndexSelectionCastImageFilter.h"
#include "itkImageToVectorImageFilter.h"

int main(int argc, char* argv[])
{
  // Verify arguments
  if(argc < 4)
    {
    std::cout << "Usage: inputImage mask targetImage outputImage" << std::endl;
    exit(-1);
    }

  // Parse arguments
  std::string sourceImageFilename = argv[1];
  std::string sourceMaskFilename = argv[2];
  std::string targetImageFilename = argv[3];
  std::string outputFilename = argv[4];

  // Output arguments
  std::cout << "Source image: " << sourceImageFilename << std::endl
            << "Target image: " << targetImageFilename << std::endl
            << "Mask image: " << sourceMaskFilename << std::endl
            << "Output image: " << outputFilename << std::endl;

  typedef itk::VectorImage<float, 2> FloatVectorImageType;

  // Read images
  typedef itk::ImageFileReader<FloatVectorImageType> ImageReaderType;
  ImageReaderType::Pointer sourceImageReader = ImageReaderType::New();
  sourceImageReader->SetFileName(sourceImageFilename);
  sourceImageReader->Update();

  ImageReaderType::Pointer targetImageReader = ImageReaderType::New();
  targetImageReader->SetFileName(targetImageFilename);
  targetImageReader->Update();

  // Read mask
  typedef itk::ImageFileReader<Mask> MaskReaderType;
  MaskReaderType::Pointer maskReader = MaskReaderType::New();
  maskReader->SetFileName(sourceMaskFilename);
  maskReader->Update();

  // Output image properties
  std::cout << "Source image: " << sourceImageReader->GetOutput()->GetLargestPossibleRegion().GetSize() << std::endl
            << "Target image: " << targetImageReader->GetOutput()->GetLargestPossibleRegion().GetSize() << std::endl
            << "Mask image: " << maskReader->GetOutput()->GetLargestPossibleRegion().GetSize() << std::endl;

  FloatVectorImageType::Pointer output = FloatVectorImageType::New();
  CloneAllChannels<FloatVectorImageType>(sourceImageReader->GetOutput(), targetImageReader->GetOutput(), maskReader->GetOutput(), output);
  
  // Write output
  //Helpers::WriteImage<FloatVectorImageType>(reassembler->GetOutput(), "output.mhd");
  Helpers::WriteVectorImageAsPNG<FloatVectorImageType>(output, outputFilename);

  return EXIT_SUCCESS;
}
