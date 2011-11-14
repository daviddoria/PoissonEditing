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

//#include "PoissonCloning.h"
#include "PoissonEditing.h"
#include "Types.h"

#include <iostream>

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCastImageFilter.h"
#include "itkNeighborhoodOperatorImageFilter.h"
#include "itkSobelOperator.h"
//#include "itkGaussianDerivativeOperator.h"

// vector image support
#include "itkVectorIndexSelectionCastImageFilter.h"
#include "itkImageToVectorImageFilter.h"

/*
 * This program expects an N channel image and a Laplacian image with the corresponding number
 * of components
 */

int main(int argc, char* argv[])
{
  // Verify arguments
  if(argc < 4)
    {
    std::cout << "Usage: inputImage maskImage laplacianImage outputImage" << std::endl;
    exit(-1);
    }

  // Parse arguments
  std::string sourceImageFilename = argv[1];
  std::string maskFilename = argv[2];
  std::string laplacianImageFilename = argv[3];
  std::string outputFilename = argv[4];

  // Output arguments
  std::cout << "Input " << sourceImageFilename << std::endl
            << "Mask " << maskFilename << std::endl
            << "laplacianImage " << laplacianImageFilename << std::endl
            << "output " << outputFilename << std::endl;

  // Read files as vector images (even if they are only 1 channel, this is just a degenerate case which does not need special treatment)
  typedef itk::ImageFileReader<FloatVectorImageType> ImageReaderType;
  ImageReaderType::Pointer sourceImageReader = ImageReaderType::New();
  sourceImageReader->SetFileName(sourceImageFilename);
  sourceImageReader->Update();

  typedef itk::ImageFileReader<FloatVectorImageType> LaplacianImageReaderType;
  LaplacianImageReaderType::Pointer laplacianImageReader = LaplacianImageReaderType::New();
  laplacianImageReader->SetFileName(laplacianImageFilename);
  laplacianImageReader->Update();

  // Ensure the input image file has the same number of components as the input Laplacian file
  if(laplacianImageReader->GetOutput()->GetNumberOfComponentsPerPixel() != sourceImageReader->GetOutput()->GetNumberOfComponentsPerPixel())
    {
    std::cerr << "The number of components of the source image is " <<  sourceImageReader->GetOutput()->GetNumberOfComponentsPerPixel()
	      << " which must (but does not) match the number of components of the Laplacian image: " 
	      << laplacianImageReader->GetOutput()->GetNumberOfComponentsPerPixel() << std::endl;
    return EXIT_FAILURE;
    }
  
  // Read the mask
  typedef itk::ImageFileReader<UnsignedCharScalarImageType> MaskReaderType;
  MaskReaderType::Pointer maskReader = MaskReaderType::New();
  maskReader->SetFileName(maskFilename);
  maskReader->Update();

  typedef itk::VectorIndexSelectionCastImageFilter<FloatVectorImageType, FloatScalarImageType> DisassemblerType;
  typedef itk::ImageToVectorImageFilter<FloatScalarImageType> ReassemblerType;
  ReassemblerType::Pointer reassembler = ReassemblerType::New();
  // Perform the Poisson reconstruction on each channel (source/Laplacian pair) independently
  std::vector<PoissonEditing<FloatScalarImageType> > poissonFilters;
  
  for(unsigned int component = 0; component < laplacianImageReader->GetOutput()->GetNumberOfComponentsPerPixel(); component++)
    {
    // Disassemble the image into its components
    
    DisassemblerType::Pointer sourceDisassembler = DisassemblerType::New();
    sourceDisassembler->SetIndex(component);
    sourceDisassembler->SetInput(sourceImageReader->GetOutput());
    sourceDisassembler->Update();
  
    DisassemblerType::Pointer laplacianDisassembler = DisassemblerType::New();
    laplacianDisassembler->SetIndex(component);
    laplacianDisassembler->SetInput(laplacianImageReader->GetOutput());
    laplacianDisassembler->Update();
    
    PoissonEditing<FloatScalarImageType> poissonFilter;
    poissonFilters.push_back(poissonFilter);
    poissonFilters[component].SetImage(sourceDisassembler->GetOutput());
    poissonFilters[component].SetGuidanceField(laplacianDisassembler->GetOutput());
    poissonFilters[component].SetMask(maskReader->GetOutput());
    poissonFilters[component].FillMaskedRegion();

    // Reassemble the image
    reassembler->SetNthInput(component, poissonFilters[component].GetOutput());
    }
  
  reassembler->Update();
  
  // Get and write output
  Helpers::WriteImage<FloatVectorImageType>(reassembler->GetOutput(), outputFilename);

  return EXIT_SUCCESS;
}
