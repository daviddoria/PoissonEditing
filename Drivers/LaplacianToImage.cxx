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
#include "itkComposeImageFilter.h"

/**
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

  typedef itk::VectorImage<float, 2> FloatVectorImageType;
  typedef itk::Image<float, 2> FloatScalarImageType;
  typedef itk::Image<unsigned char, 2> UnsignedCharScalarImageType;
  
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
  Mask::Pointer mask = Mask::New();
  mask->Read(maskFilename);

  typedef itk::VectorIndexSelectionCastImageFilter<FloatVectorImageType, FloatScalarImageType> DisassemblerType;
  typedef itk::ComposeImageFilter<FloatScalarImageType> ReassemblerType;
  ReassemblerType::Pointer reassembler = ReassemblerType::New();
  // Perform the Poisson reconstruction on each channel (source/Laplacian pair) independently
  std::vector<PoissonEditing<float> > poissonFilters;

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
    
    PoissonEditing<float> poissonFilter;
    poissonFilters.push_back(poissonFilter);
    poissonFilters[component].SetTargetImage(sourceDisassembler->GetOutput());
    poissonFilters[component].SetLaplacian(laplacianDisassembler->GetOutput());
    poissonFilters[component].SetMask(mask);
    poissonFilters[component].FillMaskedRegion();

    // Reassemble the image
    reassembler->SetInput(component, poissonFilters[component].GetOutput());
  }

  reassembler->Update();

  // Get and write output
  ITKHelpers::WriteImage(reassembler->GetOutput(), outputFilename);

  return EXIT_SUCCESS;
}
