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

  // Read images
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

  // Output image properties
  std::cout << "Source image: " << sourceImageReader->GetOutput()->GetLargestPossibleRegion().GetSize() << std::endl
            << "Target image: " << targetImageReader->GetOutput()->GetLargestPossibleRegion().GetSize() << std::endl
            << "Mask image: " << maskReader->GetOutput()->GetLargestPossibleRegion().GetSize() << std::endl;

  typedef itk::VectorIndexSelectionCastImageFilter<FloatVectorImageType, FloatScalarImageType> DisassemblerType;
  //typedef itk::ImageToVectorImageFilter<FloatScalarImageType> ReassemblerType;
  typedef itk::ImageToVectorImageFilter<UnsignedCharScalarImageType> ReassemblerType;
  ReassemblerType::Pointer reassembler = ReassemblerType::New();
  // Perform the Poisson reconstruction on each channel (source/Laplacian pair) independently
  std::vector<PoissonCloning<FloatScalarImageType> > poissonFilters(sourceImageReader->GetOutput()->GetNumberOfComponentsPerPixel());
  
  for(unsigned int component = 0; component < sourceImageReader->GetOutput()->GetNumberOfComponentsPerPixel(); component++)
    {
    // Disassemble the image into its components
    
    DisassemblerType::Pointer sourceDisassembler = DisassemblerType::New();
    sourceDisassembler->SetIndex(component);
    sourceDisassembler->SetInput(sourceImageReader->GetOutput());
    sourceDisassembler->Update();
  
    DisassemblerType::Pointer targetDisassembler = DisassemblerType::New();
    targetDisassembler->SetIndex(component);
    targetDisassembler->SetInput(targetImageReader->GetOutput());
    targetDisassembler->Update();

    std::stringstream ss;
    ss << "target_" << component << ".mha";
    Helpers::WriteImage<FloatScalarImageType>(targetDisassembler->GetOutput(), ss.str());
    
    poissonFilters[component].SetImage(sourceDisassembler->GetOutput());
    poissonFilters[component].SetTargetImage(targetDisassembler->GetOutput());
    //poissonFilters[component].SetGuidanceFieldToZero();
    //poissonFilters[component].CreateGuidanceField(sourceDisassembler->GetOutput()); // this is done internally
    poissonFilters[component].SetMask(maskReader->GetOutput());
    poissonFilters[component].PasteMaskedRegionIntoTargetImage();

    typedef itk::RescaleIntensityImageFilter< FloatScalarImageType, UnsignedCharScalarImageType > RescaleFilterType;
    RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
    rescaleFilter->SetInput(poissonFilters[component].GetOutput());
    rescaleFilter->SetOutputMinimum(0);
    rescaleFilter->SetOutputMaximum(255);
    rescaleFilter->Update();

    // Reassemble the image
    reassembler->SetNthInput(component, rescaleFilter->GetOutput());
    }
  
  reassembler->Update();

  // Write output
  //Helpers::WriteImage<FloatVectorImageType>(reassembler->GetOutput(), "output.mhd");
  Helpers::WriteVectorImageAsPNG<UnsignedCharVectorImageType>(reassembler->GetOutput(), outputFilename);

  return EXIT_SUCCESS;
}
