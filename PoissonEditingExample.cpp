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

#include "PoissonEditing.h"
#include "Types.h"

#include <iostream>

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCastImageFilter.h"

int main(int argc, char* argv[])
{
  // Verify arguments
  if(argc < 4)
    {
    std::cout << "Usage: inputImage mask outputImage" << std::endl;
    exit(-1);
    }

  // Parse arguments
  std::string inputFilename = argv[1];
  std::string maskFilename = argv[2];
  std::string outputFilename = argv[3];

  // Display arguments
  std::cout << "Input file: " << inputFilename << std::endl
            << "Mask file: " << maskFilename << std::endl
            << "Output file: " << outputFilename << std::endl;

  typedef itk::ImageFileReader<FloatVectorImageType> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(inputFilename);
  imageReader->Update();

  typedef itk::ImageFileReader<UnsignedCharScalarImageType> MaskReaderType;
  MaskReaderType::Pointer maskReader = MaskReaderType::New();
  maskReader->SetFileName(maskFilename);
  maskReader->Update();

  PoissonEditing<FloatVectorImageType> poissonEditing;
  poissonEditing.SetImage(imageReader->GetOutput());
  poissonEditing.SetMask(maskReader->GetOutput());
  poissonEditing.SetGuidanceFieldToZero();
  poissonEditing.FillMaskedRegion();

  FloatVectorImageType::Pointer outputImage = poissonEditing.GetOutput();
/*
  typedef itk::CastImageFilter< FloatVectorImageType, UnsignedCharVectorImageType > CastFilterType;
  CastFilterType::Pointer castFilter = CastFilterType::New();
  castFilter->SetInput(outputImage);
  castFilter->Update();

  typedef  itk::ImageFileWriter< UnsignedCharVectorImageType > PNGWriterType;
  PNGWriterType::Pointer pngWriter = PNGWriterType::New();
  pngWriter->SetFileName(outputFilename);
  pngWriter->SetInput(castFilter->GetOutput());
  pngWriter->Update();
*/
  typedef  itk::ImageFileWriter< FloatVectorImageType > MHDWriterType;
  MHDWriterType::Pointer mhdWriter = MHDWriterType::New();
  mhdWriter->SetFileName(outputFilename);
  mhdWriter->SetInput(outputImage);
  mhdWriter->Update();

}