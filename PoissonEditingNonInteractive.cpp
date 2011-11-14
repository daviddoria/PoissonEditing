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
  
  typedef itk::VectorIndexSelectionCastImageFilter<FloatVectorImageType, FloatScalarImageType> DisassemblerType;
  typedef itk::ImageToVectorImageFilter<FloatScalarImageType> ReassemblerType;
  ReassemblerType::Pointer reassembler = ReassemblerType::New();
  
  // Perform the Poisson reconstruction on each channel (source/Laplacian pair) independently
  std::vector<PoissonEditing<FloatScalarImageType> > poissonFilters;//(imageReader->GetOutput()->GetNumberOfComponentsPerPixel());
    
  for(unsigned int component = 0; component < imageReader->GetOutput()->GetNumberOfComponentsPerPixel(); component++)
    {
    std::cout << "Component " << component << std::endl;
    // Disassemble the image into its components
    DisassemblerType::Pointer sourceDisassembler = DisassemblerType::New();
    sourceDisassembler->SetIndex(component);
    sourceDisassembler->SetInput(imageReader->GetOutput());
    sourceDisassembler->Update();
      
    PoissonEditing<FloatScalarImageType> poissonFilter;
    poissonFilters.push_back(poissonFilter);
  
    poissonFilters[component].SetImage(sourceDisassembler->GetOutput());
    poissonFilters[component].SetGuidanceFieldToZero();
    poissonFilters[component].SetMask(maskReader->GetOutput());
    poissonFilters[component].FillMaskedRegion();

    reassembler->SetNthInput(component, poissonFilters[component].GetOutput());
    
    // Write this channel just for testing:
    std::cout << "Writing test image..." << std::endl;
    typedef  itk::ImageFileWriter< FloatScalarImageType > WriterType;
    WriterType::Pointer writer = WriterType::New();
    std::stringstream ss;
    ss << "test_" << component << ".mhd";
    writer->SetFileName(ss.str());
    writer->SetInput(poissonFilters[component].GetOutput());
    writer->Update();
    }
  
  reassembler->Update();
  std::cout << "Output components per pixel: " << reassembler->GetOutput()->GetNumberOfComponentsPerPixel() << std::endl;
  std::cout << "Output size: " << reassembler->GetOutput()->GetLargestPossibleRegion().GetSize() << std::endl;
  // Get and write output
  //Helpers::WriteImage<FloatVectorImageType>(reassembler->GetOutput(), outputFilename);
  /*
  typedef  itk::ImageFileWriter< FloatVectorImageType > WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName(outputFilename);
  writer->SetInput(reassembler->GetOutput());
  writer->Update();
  */
  Helpers::WriteVectorImageAsPNG<FloatVectorImageType>(reassembler->GetOutput(), outputFilename);
  
  return EXIT_SUCCESS;
}