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

#include "PoissonEditing.h"

// STL
#include <iostream>

// ITK
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

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

  typedef itk::VectorImage<float, 2> FloatVectorImageType;

  typedef itk::ImageFileReader<FloatVectorImageType> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(inputFilename);
  imageReader->Update();

  typedef itk::ImageFileReader<Mask> MaskReaderType;
  MaskReaderType::Pointer maskReader = MaskReaderType::New();
  maskReader->SetFileName(maskFilename);
  maskReader->Update();
  
  FloatVectorImageType::Pointer result = FloatVectorImageType::New();
  FillAllChannels(imageReader->GetOutput(), maskReader->GetOutput(), result.GetPointer());
  
  // Get and write output
  //Helpers::WriteImage<FloatVectorImageType>(reassembler->GetOutput(), outputFilename);
  /*
  typedef  itk::ImageFileWriter< FloatVectorImageType > WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName(outputFilename);
  writer->SetInput(reassembler->GetOutput());
  writer->Update();
  */
  Helpers::WriteVectorImageAsPNG(result.GetPointer(), outputFilename);
  
  return EXIT_SUCCESS;
}
