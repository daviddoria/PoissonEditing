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
#include "itkSobelOperator.h"
#include "itkGaussianDerivativeOperator.h"

int main(int argc, char* argv[])
{
  if(argc < 2)
    {
    std::cout << "Usage: inputImage outputImage" << std::endl;
    exit(-1);
    }

  std::string sourceImageFilename = argv[1];
  std::string outputFilename = argv[2];

  std::cout << "Reconstructing image from derivative (input " << sourceImageFilename << " output " << outputFilename << ")" << std::endl;

  typedef itk::ImageFileReader<FloatScalarImageType> ImageReaderType;
  ImageReaderType::Pointer sourceImageReader = ImageReaderType::New();
  sourceImageReader->SetFileName(sourceImageFilename);
  sourceImageReader->Update();
  Helpers::CastAndWriteScalarImage<FloatScalarImageType>(sourceImageReader->GetOutput(), "input.png");

  //typedef itk::GaussianDerivativeOperator<float, 2> OperatorType;
  typedef itk::DerivativeOperator<float, 2> OperatorType;
  OperatorType operatorX;
  itk::Size<2> radius;
  radius.Fill(1); // a radius of 1x1 creates a 3x3 operator
  operatorX.SetDirection(0); // Create the operator for the X axis derivative
  operatorX.CreateToRadius(radius);

  typedef itk::NeighborhoodOperatorImageFilter<FloatScalarImageType, FloatScalarImageType> NeighborhoodOperatorImageFilterType;
  NeighborhoodOperatorImageFilterType::Pointer xDerivativeFilter = NeighborhoodOperatorImageFilterType::New();
  xDerivativeFilter->SetOperator(operatorX);
  xDerivativeFilter->SetInput(sourceImageReader->GetOutput());
  xDerivativeFilter->Update();

  Helpers::CastAndWriteScalarImage<FloatScalarImageType>(xDerivativeFilter->GetOutput(), "xDerivative.png");

  OperatorType operatorY;
  operatorY.SetDirection(1); // Create the operator for the Y axis derivative
  operatorY.CreateToRadius(radius);

  NeighborhoodOperatorImageFilterType::Pointer yDerivativeFilter = NeighborhoodOperatorImageFilterType::New();
  yDerivativeFilter->SetOperator(operatorY);
  yDerivativeFilter->SetInput(sourceImageReader->GetOutput());
  yDerivativeFilter->Update();
  Helpers::CastAndWriteScalarImage<FloatScalarImageType>(yDerivativeFilter->GetOutput(), "yDerivative.png");

  FloatScalarImageType::Pointer outputImage = FloatScalarImageType::New();

  DerivativeToImage<FloatScalarImageType> derivativeToImage;
  derivativeToImage.SetImage(sourceImageReader->GetOutput());
  derivativeToImage.SetXDerivative(xDerivativeFilter->GetOutput());
  derivativeToImage.SetYDerivative(yDerivativeFilter->GetOutput());
  derivativeToImage.SetXDerivativeOperator(&operatorX);
  derivativeToImage.SetYDerivativeOperator(&operatorY);
  derivativeToImage.ReconstructImage(outputImage);

  Helpers::WriteImage<FloatScalarImageType>(outputImage, "output.mhd");

//  Helpers::ClampImage<FloatScalarImageType>(outputImage);
  Helpers::CastAndWriteScalarImage<FloatScalarImageType>(outputImage, "output.png");

  return EXIT_SUCCESS;
}