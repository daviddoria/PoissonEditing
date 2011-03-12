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
#include "itkGaussianDerivativeOperator.h"

/*
 * This program expects a single channel image and its Laplacian (also single channel)
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

  // Read files
  typedef itk::ImageFileReader<FloatSingleChannelImageType> ImageReaderType;
  ImageReaderType::Pointer sourceImageReader = ImageReaderType::New();
  sourceImageReader->SetFileName(sourceImageFilename);
  sourceImageReader->Update();

  typedef itk::ImageFileReader<FloatScalarImageType> LaplacianImageReaderType;
  LaplacianImageReaderType::Pointer laplacianImageReader = LaplacianImageReaderType::New();
  laplacianImageReader->SetFileName(laplacianImageFilename);
  laplacianImageReader->Update();

  typedef itk::ImageFileReader<UnsignedCharScalarImageType> MaskReaderType;
  MaskReaderType::Pointer maskReader = MaskReaderType::New();
  maskReader->SetFileName(maskFilename);
  maskReader->Update();

  PoissonEditing<FloatSingleChannelImageType> poissonEditing;
  poissonEditing.SetImage(sourceImageReader->GetOutput());
  poissonEditing.SetGuidanceField(laplacianImageReader->GetOutput());
  poissonEditing.SetMask(maskReader->GetOutput());
  poissonEditing.FillMaskedRegion();

  // Get and write output
  FloatSingleChannelImageType::Pointer outputImage = poissonEditing.GetOutput();

  Helpers::WriteImage<FloatSingleChannelImageType>(outputImage, outputFilename);

  return EXIT_SUCCESS;
}