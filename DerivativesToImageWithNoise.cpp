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

#include "DerivativeToImage.h"
#include "Types.h"

#include <iostream>

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCastImageFilter.h"
#include "itkNeighborhoodOperatorImageFilter.h"
#include "itkRandomImageSource.h"
#include "itkAddImageFilter.h"
#include "itkLaplacianOperator.h"

// Note: adding noise to the input image doesn't change anything! You must add noise to the derivative images!
int main(int argc, char* argv[])
{
  // Verify argument
  if(argc < 3)
    {
    std::cout << "Usage: inputImage maskImage outputImage" << std::endl;
    exit(-1);
    }

  // Parse arguments
  std::string sourceImageFilename = argv[1];
  std::string maskFilename = argv[2];
  std::string outputFilename = argv[3];

  std::cout << "Input " << sourceImageFilename << std::endl
            << "Mask " << maskFilename << std::endl
            << "Output " << outputFilename << std::endl;

  // Read image
  typedef itk::ImageFileReader<FloatScalarImageType> ImageReaderType;
  ImageReaderType::Pointer sourceImageReader = ImageReaderType::New();
  sourceImageReader->SetFileName(sourceImageFilename);
  sourceImageReader->Update();

  FloatScalarImageType::Pointer image = FloatScalarImageType::New();
  image->Graft(sourceImageReader->GetOutput());
  Helpers::CastAndWriteScalarImage<FloatScalarImageType>(image, "input.png");
  
  // Read the mask
  typedef itk::ImageFileReader<UnsignedCharScalarImageType> MaskReaderType;
  MaskReaderType::Pointer maskReader = MaskReaderType::New();
  maskReader->SetFileName(maskFilename);
  maskReader->Update();

  // Setup operator
  typedef itk::LaplacianOperator<float, 2> OperatorType;
  itk::Size<2> radius;
  radius.Fill(1); // a radius of 1x1 creates a 3x3 operator

  OperatorType laplacianOperator;
  //laplacianOperator.SetDirection(0); // Create the operator for the X axis derivative
  laplacianOperator.CreateToRadius(radius);

  typedef itk::NeighborhoodOperatorImageFilter<FloatScalarImageType, FloatScalarImageType> NeighborhoodOperatorImageFilterType;
  NeighborhoodOperatorImageFilterType::Pointer laplacianFilter = NeighborhoodOperatorImageFilterType::New();
  laplacianFilter->SetOperator(laplacianOperator);
  laplacianFilter->SetInput(image);
  laplacianFilter->Update();
  Helpers::WriteImage<FloatScalarImageType>(laplacianFilter->GetOutput(), "laplacian.mhd");
  
  const float noiseMagnitude = 5;
  // Add noise to Laplacian derivative
  typedef itk::RandomImageSource<FloatScalarImageType> RandomSourceType;
  RandomSourceType::Pointer noise = RandomSourceType::New();
  noise->SetSize(laplacianFilter->GetOutput()->GetLargestPossibleRegion().GetSize());
  noise->SetMin(-noiseMagnitude);
  noise->SetMax(noiseMagnitude);
  noise->Update();
  //Helpers::WriteImage<FloatScalarImageType>(randomImageSource->GetOutput(), "noise.mhd");

  typedef itk::AddImageFilter <FloatScalarImageType, FloatScalarImageType> AddImageFilterType;
  AddImageFilterType::Pointer addNoiseFilter = AddImageFilterType::New ();
  addNoiseFilter->SetInput1(laplacianFilter->GetOutput());
  addNoiseFilter->SetInput2(noise->GetOutput());
  addNoiseFilter->Update();
  //Helpers::WriteImage<FloatScalarImageType>(addFilter->GetOutput(), "noisyImage.mhd");
  
  FloatScalarImageType::Pointer outputImage = FloatScalarImageType::New();

  DerivativeToImage<FloatScalarImageType> derivativeToImage;
  derivativeToImage.SetImage(image);
  derivativeToImage.SetMask(maskReader->GetOutput());
  derivativeToImage.SetLaplacianImage(addNoiseFilter->GetOutput());
  derivativeToImage.SetLaplacianOperator(&laplacianOperator);
  derivativeToImage.ReconstructImage(outputImage);

  //Helpers::WriteImage<FloatScalarImageType>(outputImage, "output.mhd");

//  Helpers::ClampImage<FloatScalarImageType>(outputImage);
  //Helpers::CastAndWriteScalarImage<FloatScalarImageType>(outputImage, "output.png");
  Helpers::ScaleAndWriteScalarImage<FloatScalarImageType>(outputImage, "output.png");
  Helpers::WriteImage<FloatScalarImageType>(outputImage, "output.mhd");

  return EXIT_SUCCESS;
}