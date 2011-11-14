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
#include "itkVectorIndexSelectionCastImageFilter.h"
#include "itkImageToVectorImageFilter.h"

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
  
  FloatVectorImageType::Pointer result = FloatVectorImageType::New();
  FillAllChannels<FloatVectorImageType>(imageReader->GetOutput(), maskReader->GetOutput(), result);
  
  // Get and write output
  //Helpers::WriteImage<FloatVectorImageType>(reassembler->GetOutput(), outputFilename);
  /*
  typedef  itk::ImageFileWriter< FloatVectorImageType > WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName(outputFilename);
  writer->SetInput(reassembler->GetOutput());
  writer->Update();
  */
  Helpers::WriteVectorImageAsPNG<FloatVectorImageType>(result, outputFilename);
  
  return EXIT_SUCCESS;
}