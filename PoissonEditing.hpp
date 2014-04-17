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

#ifndef PoissonEditing_HPP
#define PoissonEditing_HPP

#include "PoissonEditing.h" // Appease syntax parser

// Submodules
#include "Helpers/Helpers.h"
#include "ITKHelpers/ITKHelpers.h"

// ITK
#include "itkAddImageFilter.h"
#include "itkImageRegionConstIterator.h"
#include "itkComposeImageFilter.h"
#include "itkLaplacianOperator.h"
#include "itkLaplacianImageFilter.h"
#include "itkVectorIndexSelectionCastImageFilter.h"

// Eigen
#include <Eigen/Sparse>

template <typename TPixel>
PoissonEditing<TPixel>::PoissonEditing()
{
  this->TargetImage = ImageType::New();
  this->SourceImage = ImageType::New();
  this->Output = ImageType::New();

  this->GuidanceField = GuidanceFieldType::New();
  this->MaskImage = Mask::New();
}

template <typename TPixel>
void PoissonEditing<TPixel>::SetLaplacian(FloatScalarImageType* const laplacian)
{
  this->Laplacian = laplacian;
}

template <typename TPixel>
void PoissonEditing<TPixel>::SetFillMethod(FillMethodEnum fillMethod)
{
  this->FillMethod = fillMethod;
}

template <typename TPixel>
void PoissonEditing<TPixel>::SetTargetImage(const ImageType* const targetImage)
{
  ITKHelpers::DeepCopy(targetImage, this->TargetImage.GetPointer());
}

template <typename TPixel>
void PoissonEditing<TPixel>::SetSourceImage(const ImageType* const sourceImage)
{
//  ITKHelpers::DeepCopy(sourceImage, this->SourceImage.GetPointer());

  if(this->RegionToProcess.GetSize()[0] == 0 || this->RegionToProcess.GetSize()[1] == 0)
  {
    throw std::runtime_error("RegionToProcess must be set before calling SetSourceImage!");
  }

  if(!this->TargetImage->GetLargestPossibleRegion().IsInside(sourceImage->GetLargestPossibleRegion()))
  {
    throw std::runtime_error("Guidance field must be smaller than the target image!");
  }

  // Make the guidance field the same size as the target image, and copy the data to the requested location
  this->SourceImage->SetRegions(this->TargetImage->GetLargestPossibleRegion());
  this->SourceImage->Allocate();
  ITKHelpers::SetImageToConstant(this->SourceImage.GetPointer(), 0);

  ITKHelpers::CopyRegion(sourceImage, this->SourceImage.GetPointer(), sourceImage->GetLargestPossibleRegion(),
                         this->RegionToProcess);
}

template <typename TPixel>
void PoissonEditing<TPixel>::SetGuidanceField(const GuidanceFieldType* const field)
{
  if(this->RegionToProcess.GetSize()[0] == 0 || this->RegionToProcess.GetSize()[1] == 0)
  {
    throw std::runtime_error("RegionToProcess must be set before calling SetGuidanceField!");
  }

  if(!this->TargetImage->GetLargestPossibleRegion().IsInside(field->GetLargestPossibleRegion()))
  {
    throw std::runtime_error("Guidance field must be smaller than the target image!");
  }

  // Make the guidance field the same size as the target image, and copy the data to the requested location
  this->GuidanceField->SetRegions(this->TargetImage->GetLargestPossibleRegion());
  this->GuidanceField->Allocate();

  SetGuidanceFieldToZero();

  ITKHelpers::CopyRegion(field, this->GuidanceField.GetPointer(), field->GetLargestPossibleRegion(),
                         this->RegionToProcess);

//  ITKHelpers::WriteImage(field, "Field.mha");
//  ITKHelpers::WriteImage(this->GuidanceField.GetPointer(), "GuidanceFieldSet.mha");
}

template <typename TPixel>
void PoissonEditing<TPixel>::SetMask(const Mask* const mask)
{
  if(this->RegionToProcess.GetSize()[0] == 0 || this->RegionToProcess.GetSize()[1] == 0)
  {
    throw std::runtime_error("RegionToProcess must be set before calling SetMask!");
  }

  if(!this->TargetImage->GetLargestPossibleRegion().IsInside(mask->GetLargestPossibleRegion()))
  {
    throw std::runtime_error("Guidance field must be smaller than the target image!");
  }

  // Make the mask field the same size as the target image, and copy the data to the requested location
  this->MaskImage->SetRegions(this->TargetImage->GetLargestPossibleRegion());
  this->MaskImage->Allocate();
  ITKHelpers::SetImageToConstant(this->MaskImage.GetPointer(), HoleMaskPixelTypeEnum::VALID);

  ITKHelpers::CopyRegion(mask, this->MaskImage.GetPointer(), mask->GetLargestPossibleRegion(),
                         this->RegionToProcess);
}

template <typename TPixel>
void PoissonEditing<TPixel>::SetGuidanceFieldToZero()
{
  // In the hole filling problem, we want the guidance field fo be zero
  this->GuidanceField->SetRegions(this->TargetImage->GetLargestPossibleRegion());
  this->GuidanceField->Allocate();
  this->GuidanceField->FillBuffer(itk::NumericTraits<typename GuidanceFieldType::PixelType>::Zero);
}

template <typename TPixel>
void PoissonEditing<TPixel>::FillMaskedRegion()
{
  // Create a map that stores the ID of each hole pixel
  typedef std::map<itk::Index<2>, unsigned int, itk::Index<2>::LexicographicCompare> VariableIdMapType;
  VariableIdMapType variableIdMap;

  itk::ImageRegionIterator<Mask> maskIterator(this->MaskImage, this->MaskImage->GetLargestPossibleRegion());

  while(!maskIterator.IsAtEnd())
  {
    itk::Index<2> pixelIndex = maskIterator.GetIndex();

    if(this->MaskImage->IsHole(pixelIndex))
    {
      //std::cout << "Adding variable " << variableIdMap.size() << std::endl;
      // Add the pixel to the map, with ID equal to the next sequential integer.
      variableIdMap.insert(VariableIdMapType::value_type(pixelIndex, variableIdMap.size()));
    }
    ++maskIterator;
  }

  if(variableIdMap.size() == 0)
  {
    std::cerr << "PoissonEditing::FillMaskedRegion(): No masked pixels found!" << std::endl;
    return;
  }

  // Create a 3x3 Laplacian kernel
  typedef itk::LaplacianOperator<float, 2> LaplacianOperatorType;
  LaplacianOperatorType laplacianOperator;
  itk::Size<2> radius;
  radius.Fill(1);
  laplacianOperator.CreateToRadius(radius);
  unsigned int numberOfPixelsInKernel = laplacianOperator.GetSize()[0] * laplacianOperator.GetSize()[1];

  // Create the sparse matrix
  typedef Eigen::SparseMatrix<double> SparseMatrixType;
  SparseMatrixType A(variableIdMap.size(), variableIdMap.size());
  A.reserve(Eigen::VectorXi::Constant(variableIdMap.size(),5));

  // Create the right-hand-side vector
  Eigen::VectorXd b(variableIdMap.size());

  // Create a laplacian image from the provided gradient field if it is not provided directly
  FloatImageType::Pointer laplacian = FloatImageType::New();
  laplacian->SetRegions(this->MaskImage->GetLargestPossibleRegion());
  laplacian->Allocate();
  if(!this->Laplacian)
  {
    std::cout << "Computing Laplacian from provided GuidanceField..." << std::endl;
    //ITKHelpers::WriteImage(this->GuidanceField.GetPointer(), "guidance.mha");
    LaplacianFromGradient(this->GuidanceField, laplacian);
  }
  else
  {
    std::cout << "Using provided Laplacian..." << std::endl;
    ITKHelpers::DeepCopy(this->Laplacian, laplacian.GetPointer());
  }

  //ITKHelpers::WriteImage(laplacian.GetPointer(), "laplacian.mha");

  if(this->SourceImage->GetLargestPossibleRegion().GetSize()[0] != 0)
  {
    if(this->SourceImage->GetLargestPossibleRegion().GetSize() !=
       laplacian->GetLargestPossibleRegion().GetSize())
    {
      std::cerr << "SourceImage and laplacian are not the same size!"
                << "SourceImage size: " << this->SourceImage->GetLargestPossibleRegion().GetSize() << std::endl
                << "laplacian size: " << laplacian->GetLargestPossibleRegion().GetSize() << std::endl
                << "GuidanceField size: " << this->GuidanceField->GetLargestPossibleRegion().GetSize() << std::endl
                << std::endl;
      return;
    }
  }

  // Create the row of the matrix for each pixel
  for(VariableIdMapType::const_iterator iter = variableIdMap.begin(); iter != variableIdMap.end(); ++iter)
  {
    //std::cout << "Creating equation for variable " << iter->second << std::endl;
    itk::Index<2> originalPixel = iter->first;
    unsigned int variableId = iter->second;

    // The right hand side of the equation starts equal to the value of the guidance field
    double bvalue = laplacian->GetPixel(originalPixel);

    // Loop over the kernel around the current pixel
    for(unsigned int offset = 0; offset < numberOfPixelsInKernel; ++offset)
    {
      if(laplacianOperator.GetElement(offset) == 0)
      {
        continue; // this pixel isn't going to contribute anyway
      }

      itk::Index<2> currentPixel = originalPixel + laplacianOperator.GetOffset(offset);

      if(!this->MaskImage->GetLargestPossibleRegion().IsInside(currentPixel))
      {
        continue; // this pixel is on the border, just ignore it.
      }

      if(this->MaskImage->IsHole(currentPixel))
      {
        // If the pixel is masked, add it as part of the unknown matrix
        double value = laplacianOperator.GetElement(offset);
        A.coeffRef(variableId, variableIdMap[currentPixel]) += value;
      }
      else
      {
        // If the pixel is known, move its contribution to the known (right) side of the equation
        bvalue -= this->TargetImage->GetPixel(currentPixel) * laplacianOperator.GetElement(offset);
      }
    }
    b[variableId] = bvalue;
  }// end for variables

  // Solve the (symmetric) system
  Eigen::SimplicialLDLT<SparseMatrixType> sparseSolver(A);
  Eigen::VectorXd x = sparseSolver.solve(b);
  if(sparseSolver.info() != Eigen::Success)
  {
    throw std::runtime_error("Decomposition failed!");
  }

  // Convert solution vector back to image
  // Initialize the output by copying the target image into the output.
  // Pixels that are not filled will remain the same in the output.
  ITKHelpers::DeepCopy(this->TargetImage.GetPointer(), this->Output.GetPointer());

  for(VariableIdMapType::const_iterator iter = variableIdMap.begin(); iter != variableIdMap.end(); ++iter)
  {
    this->Output->SetPixel(iter->first, x(iter->second));
  }
} // end FillMaskedRegion


template <typename TPixel>
void PoissonEditing<TPixel>::FillMaskedRegionNoColorCorrection()
{
  //ITKHelpers::WriteImage(this->TargetImage, "FillMaskedRegion_TargetImage.mha");
  //ITKHelpers::WriteImage(this->Output, "InitializedOutput.mha");

  // Create a map that stores the ID of each hole pixel
  typedef std::map<itk::Index<2>, unsigned int, itk::Index<2>::LexicographicCompare> VariableIdMapType;
  VariableIdMapType variableIdMap;

  itk::ImageRegionIterator<Mask> maskIterator(this->MaskImage, this->MaskImage->GetLargestPossibleRegion());

  while(!maskIterator.IsAtEnd())
  {
    itk::Index<2> pixelIndex = maskIterator.GetIndex();

    if(this->MaskImage->IsHole(pixelIndex))
    {
      //std::cout << "Adding variable " << variableIdMap.size() << std::endl;
      // Add the pixel to the map, with ID equal to the next sequential integer.
      variableIdMap.insert(VariableIdMapType::value_type(pixelIndex, variableIdMap.size()));
    }
    ++maskIterator;
  }

  if(variableIdMap.size() == 0)
  {
    std::cerr << "PoissonEditing::FillMaskedRegion(): No masked pixels found!" << std::endl;
    return;
  }

  // Create a 3x3 Laplacian kernel
  typedef itk::LaplacianOperator<float, 2> LaplacianOperatorType;
  LaplacianOperatorType laplacianOperator;
  itk::Size<2> radius;
  radius.Fill(1);
  laplacianOperator.CreateToRadius(radius);
  unsigned int numberOfPixelsInKernel = laplacianOperator.GetSize()[0] * laplacianOperator.GetSize()[1];

  // Create the sparse matrix
  typedef Eigen::SparseMatrix<double> SparseMatrixType;
  SparseMatrixType A(variableIdMap.size(), variableIdMap.size());
  A.reserve(Eigen::VectorXi::Constant(variableIdMap.size(),5));

  // Create the right-hand-side vector
  Eigen::VectorXd b(variableIdMap.size());

  // Create a laplacian image from the provided gradient field if it is not provided directly
  FloatImageType::Pointer laplacian = FloatImageType::New();
  laplacian->SetRegions(this->MaskImage->GetLargestPossibleRegion());
  laplacian->Allocate();
  if(!this->Laplacian)
  {
    LaplacianFromGradient(this->GuidanceField, laplacian);
  }
  else
  {
    ITKHelpers::DeepCopy(this->Laplacian, laplacian.GetPointer());
  }

  //ITKHelpers::WriteImage(laplacian.GetPointer(), "laplacian.mha");

  // Create the row of the matrix for each pixel
  for(VariableIdMapType::const_iterator iter = variableIdMap.begin(); iter != variableIdMap.end(); ++iter)
  {
    //std::cout << "Creating equation for variable " << iter->second << std::endl;
    itk::Index<2> originalPixel = iter->first;
    unsigned int variableId = iter->second;
    //std::cout << "originalPixel " << originalPixel << std::endl;

    // The right hand side of the equation starts equal to the value of the guidance field
    double bvalue = laplacian->GetPixel(originalPixel);

    // Loop over the kernel around the current pixel
    for(unsigned int offset = 0; offset < numberOfPixelsInKernel; ++offset)
    {
      if(laplacianOperator.GetElement(offset) == 0)
      {
        continue; // this pixel isn't going to contribute anyway
      }

      itk::Index<2> currentPixel = originalPixel + laplacianOperator.GetOffset(offset);

      if(!this->MaskImage->GetLargestPossibleRegion().IsInside(currentPixel))
      {
        continue; // this pixel is on the border, just ignore it.
      }

      if(this->MaskImage->IsHole(currentPixel))
      {
        // If the pixel is masked, add it as part of the unknown matrix
        double value = laplacianOperator.GetElement(offset);
//        A.insert(variableId, variableIdMap[currentPixel]) += laplacianOperator.GetElement(offset);
        A.coeffRef(variableId, variableIdMap[currentPixel]) += value;
      }
      else
      {
        // If the pixel is known, move its contribution to the known (right) side of the equation
        bvalue -= this->TargetImage->GetPixel(currentPixel) * laplacianOperator.GetElement(offset);
      }
    }
    b[variableId] = bvalue;
  }// end for variables

  // Solve the (symmetric) system
  Eigen::SimplicialLDLT<SparseMatrixType> sparseSolver(A);
  Eigen::VectorXd x = sparseSolver.solve(b);
  if(sparseSolver.info() != Eigen::Success)
  {
    throw std::runtime_error("Decomposition failed!");
  }

  // Convert solution vector back to image
  // Initialize the output by copying the target image into the output.
  // Pixels that are not filled will remain the same in the output.
  ITKHelpers::DeepCopy(this->TargetImage.GetPointer(), this->Output.GetPointer());

  for(VariableIdMapType::const_iterator iter = variableIdMap.begin(); iter != variableIdMap.end(); ++iter)
  {
    this->Output->SetPixel(iter->first, x(iter->second));
  }
} // end FillMaskedRegion


template <typename TPixel>
typename PoissonEditing<TPixel>::ImageType* PoissonEditing<TPixel>::GetOutput()
{
  return this->Output;
}

template <typename TPixel>
bool PoissonEditing<TPixel>::VerifyMask() const
{
  // This function checks that the mask is the same size as the image and that
  // there is no mask on the boundary.

  // Verify that the image and the mask are the same size
  if(this->SourceImage->GetLargestPossibleRegion().GetSize() != this->MaskImage->GetLargestPossibleRegion().GetSize())
  {
    std::cout << "Image size: " << this->SourceImage->GetLargestPossibleRegion().GetSize() << std::endl;
    std::cout << "Mask size: " << this->MaskImage->GetLargestPossibleRegion().GetSize() << std::endl;
    return false;
  }

  // Verify that no border pixels are masked
  itk::ImageRegionConstIterator<Mask> maskIterator(this->MaskImage, this->MaskImage->GetLargestPossibleRegion());

  while(!maskIterator.IsAtEnd())
  {
    if(maskIterator.GetIndex()[0] == 0 ||
       static_cast<unsigned int>(maskIterator.GetIndex()[0]) == this->MaskImage->GetLargestPossibleRegion().GetSize()[0]-1 ||
       maskIterator.GetIndex()[1] == 0 ||
       static_cast<unsigned int>(maskIterator.GetIndex()[1]) == this->MaskImage->GetLargestPossibleRegion().GetSize()[1]-1)
      if(maskIterator.Get())
      {
        std::cout << "Mask is invalid! Pixel " << maskIterator.GetIndex() << " is masked!" << std::endl;
        return false;
      }
    ++maskIterator;
  }

  std::cout << "Mask is valid!" << std::endl;
  return true;
}


template <typename TPixel>
void PoissonEditing<TPixel>::
CreateGuidanceFieldFromImage(const FloatScalarImageType* const sourceImage)
{
  typedef itk::LaplacianImageFilter<FloatScalarImageType, FloatScalarImageType>  LaplacianFilterType;

  typename LaplacianFilterType::Pointer laplacianFilter = LaplacianFilterType::New();
  laplacianFilter->SetInput(sourceImage);
  laplacianFilter->Update();
  this->SetGuidanceField(laplacianFilter->GetOutput());
}

template <typename TPixel>
void
PoissonEditing<TPixel>::LaplacianFromGradient(const PoissonEditing<TPixel>::GradientImageType* const gradientImage,
                                              FloatImageType* const outputLaplacian)
{
  typedef itk::VectorIndexSelectionCastImageFilter<GradientImageType, FloatImageType> IndexSelectionType;
  
  IndexSelectionType::Pointer xIndexSelectionFilter = IndexSelectionType::New();
  xIndexSelectionFilter->SetIndex(0);
  xIndexSelectionFilter->SetInput(gradientImage);
  xIndexSelectionFilter->Update();

  // The the x derivative of the x derivative (second x partial)
  FloatImageType::Pointer xSecondDerivative = FloatImageType::New();
  ITKHelpers::CentralDifferenceDerivative(xIndexSelectionFilter->GetOutput(), 0, xSecondDerivative.GetPointer());
  
  IndexSelectionType::Pointer yIndexSelectionFilter = IndexSelectionType::New();
  yIndexSelectionFilter->SetIndex(1);
  yIndexSelectionFilter->SetInput(gradientImage);
  yIndexSelectionFilter->Update();

  // The the y derivative of the y derivative (second y partial)
  FloatImageType::Pointer ySecondDerivative = FloatImageType::New();
  ITKHelpers::CentralDifferenceDerivative(yIndexSelectionFilter->GetOutput(), 1, ySecondDerivative.GetPointer());
  
  typedef itk::AddImageFilter<FloatImageType, FloatImageType> AddImageFilterType;
  AddImageFilterType::Pointer addFilter = AddImageFilterType::New ();
  addFilter->SetInput1(xSecondDerivative);
  addFilter->SetInput2(ySecondDerivative);
  addFilter->Update();

  ITKHelpers::DeepCopy(addFilter->GetOutput(), outputLaplacian);
}

template <typename TPixel>
void PoissonEditing<TPixel>::SetRegionToProcess(const itk::ImageRegion<2>& regionToProcess)
{
  this->RegionToProcess = regionToProcess;
}

#endif
