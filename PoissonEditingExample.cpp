// =============================================================================
// PoissonEditing - Poisson Image Editing for cloning and image estimation
//
// The following code implements:
// Exercise 1, Advanced Computer Graphics Course (Spring 2005)
// Tel-Aviv University, Israel
// http://www.leyvand.com/research/adv-graphics/ex1.htm
//
// * Based on "Poisson Image Editing" paper, Pe'rez et. al. [SIGGRAPH/2003].
// * The code uses TAUCS, A sparse linear solver library by Sivan Toledo
//   (see http://www.tau.ac.il/~stoledo/taucs)
// =============================================================================
//
// COPYRIGHT NOTICE, DISCLAIMER, and LICENSE:
//
// PoissonEditing : Copyright (C) 2005, Tommer Leyvand (tommerl@gmail.com)
//
// Covered code is provided under this license on an "as is" basis, without
// warranty of any kind, either expressed or implied, including, without
// limitation, warranties that the covered code is free of defects,
// merchantable, fit for a particular purpose or non-infringing. The entire risk
// as to the quality and performance of the covered code is with you. Should any
// covered code prove defective in any respect, you (not the initial developer
// or any other contributor) assume the cost of any necessary servicing, repair
// or correction. This disclaimer of warranty constitutes an essential part of
// this license. No use of any covered code is authorized hereunder except under
// this disclaimer.
//
// Permission is hereby granted to use, copy, modify, and distribute this
// source code, or portions hereof, for any purpose, including commercial
// applications, freely and without fee, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//

#include "PoissonEditing.h"
#include "Types.h"

#include <iostream>

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCastImageFilter.h"

int main(int argc, char* argv[])
{
  if(argc < 4)
    {
    std::cout << "Usage: inputImage mask outputImage" << std::endl;
    exit(-1);
    }

  std::string inputFilename = argv[1];
  std::string maskFilename = argv[2];
  std::string outputFilename = argv[3];

  typedef itk::ImageFileReader<FloatVectorImageType> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(inputFilename);
  imageReader->Update();

  typedef itk::ImageFileReader<UnsignedCharScalarImageType> MaskReaderType;
  MaskReaderType::Pointer maskReader = MaskReaderType::New();
  maskReader->SetFileName(maskFilename);
  maskReader->Update();

  FloatVectorImageType::Pointer outputImage = FloatVectorImageType::New();

  PoissonEditing poissonEditing;
  poissonEditing.FillRegion(imageReader->GetOutput(), maskReader->GetOutput(), outputImage);

  typedef itk::CastImageFilter< FloatVectorImageType, UnsignedCharVectorImageType > CastFilterType;
  CastFilterType::Pointer castFilter = CastFilterType::New();
  castFilter->SetInput(outputImage);
  castFilter->Update();
  
  typedef  itk::ImageFileWriter< UnsignedCharVectorImageType > WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName(outputFilename);
  writer->SetInput(castFilter->GetOutput());
  writer->Update();

}