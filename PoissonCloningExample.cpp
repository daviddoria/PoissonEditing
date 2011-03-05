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
#include "Types.h"

#include <iostream>

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCastImageFilter.h"

int main(int argc, char* argv[])
{
  if(argc < 4)
    {
    std::cout << "Usage: inputImage mask outputImage" << std::endl;
    exit(-1);
    }

  std::string sourceImageFilename = argv[1];
  std::string sourceMaskFilename = argv[2];
  std::string targetImageFilename = argv[3];
  std::string outputFilename = argv[4];

  typedef itk::ImageFileReader<FloatVectorImageType> ImageReaderType;
  ImageReaderType::Pointer sourceImageReader = ImageReaderType::New();
  sourceImageReader->SetFileName(sourceImageFilename);
  sourceImageReader->Update();

  typedef itk::ImageFileReader<UnsignedCharScalarImageType> MaskReaderType;
  MaskReaderType::Pointer maskReader = MaskReaderType::New();
  maskReader->SetFileName(sourceMaskFilename);
  maskReader->Update();

  ImageReaderType::Pointer targetImageReader = ImageReaderType::New();
  targetImageReader->SetFileName(targetImageFilename);
  targetImageReader->Update();

  FloatVectorImageType::Pointer outputImage = FloatVectorImageType::New();

  PoissonCloning<FloatVectorImageType> poissonCloning;
  poissonCloning.SetSourceImage(sourceImageReader->GetOutput());
  poissonCloning.SetTargetImage(targetImageReader->GetOutput());
  poissonCloning.SetMask(maskReader->GetOutput());
  poissonCloning.PasteMaskedRegionIntoTargetImage(outputImage);


  Helpers::WriteImage<FloatVectorImageType>(outputImage, "output.mhd");
  //Helpers::WriteScaledImage<FloatVectorImageType>(outputImage, "output.png");
  Helpers::ClampImage<FloatVectorImageType>(outputImage);
  Helpers::CastAndWriteImage<FloatVectorImageType>(outputImage, "output.png");

  return EXIT_SUCCESS;
}