/*
Copyright (C) 2011 David Doria, daviddoria@gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
