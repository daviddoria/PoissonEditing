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
  this->Output = ImageType::New();

  this->GuidanceField = GuidanceFieldType::New();
  this->MaskImage = Mask::New();

  this->Laplacian = nullptr;
  this->FillMethod = FillMethodEnum::POISSON;
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
void PoissonEditing<TPixel>::SetTargetImage(const ImageType* const image)
{
  ITKHelpers::DeepCopy(image, this->TargetImage.GetPointer());
}

template <typename TPixel>
void PoissonEditing<TPixel>::SetGuidanceField(const GuidanceFieldType* const field)
{
  ITKHelpers::DeepCopy(field, this->GuidanceField.GetPointer());
}

template <typename TPixel>
void PoissonEditing<TPixel>::SetSourceImage(const ImageType* const image)
{
  CreateGuidanceFieldFromImage(image);
}

template <typename TPixel>
void PoissonEditing<TPixel>::SetMask(const Mask* const mask)
{
  this->MaskImage->DeepCopyFrom(mask);
}

template <typename TPixel>
void PoissonEditing<TPixel>::SetGuidanceFieldToZero()
{
  // In the hole filling problem, we want the guidance field fo be zero
  GuidanceField->SetRegions(this->TargetImage->GetLargestPossibleRegion());
  GuidanceField->Allocate();
  GuidanceField->FillBuffer(0);
}


template <typename TPixel>
void PoissonEditing<TPixel>::FillMaskedRegion()
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
  laplacian->SetRegions(MaskImage->GetLargestPossibleRegion());
  laplacian->Allocate();
  if(!this->Laplacian)
  {
    LaplacianFromGradient(GuidanceField, laplacian);
  }
  else
  {
    ITKHelpers::DeepCopy(this->Laplacian, laplacian.GetPointer());
  }

  ITKHelpers::WriteImage(laplacian.GetPointer(), "laplacian.mha");

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
        A.insert(variableId, variableIdMap[currentPixel]) = laplacianOperator.GetElement(offset);
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
template <typename TImage>
void PoissonEditing<TPixel>::FillImage(const TImage* const image, const Mask* const mask,
                                       const std::vector<GuidanceFieldType::Pointer>& guidanceFields, TImage* const output)
{
  std::vector<GuidanceFieldType*> guidanceFieldsRaw;
  for(unsigned int i = 0; i < guidanceFields.size(); ++i)
  {
    guidanceFieldsRaw.push_back(guidanceFields[i]);
  }

  FillVectorImage(image, mask, guidanceFieldsRaw, output);
}

template <typename TPixel>
template <typename TImage>
void PoissonEditing<TPixel>::FillVectorImage(const TImage* const image, const Mask* const mask,
                                             const std::vector<GuidanceFieldType*>& guidanceFields,
                                             TImage* const output)
{
  if(!mask)
  {
    throw std::runtime_error("You must specify a mask!");
  }

  if(guidanceFields.size() != image->GetNumberOfComponentsPerPixel())
  {
    std::stringstream ss;
    ss << "There are " << image->GetNumberOfComponentsPerPixel() << " channels but "
       << guidanceFields.size() << " guidance fields were specified (these must match).";
    throw std::runtime_error(ss.str());
  }

  typedef itk::Image<typename TypeTraits<typename TImage::PixelType>::ComponentType, 2> ScalarImageType;

  typedef itk::ComposeImageFilter<ScalarImageType, TImage> ReassemblerType;
  typename ReassemblerType::Pointer reassembler = ReassemblerType::New();

  // Perform the Poisson reconstruction on each channel independently
  typedef typename TypeTraits<typename TImage::PixelType>::ComponentType ComponentType;
  typedef PoissonEditing<ComponentType> PoissonEditingFilterType;

  std::vector<PoissonEditingFilterType> poissonFilters;

  for(unsigned int component = 0; component < image->GetNumberOfComponentsPerPixel(); component++)
  {
    std::cout << "Filling component " << component << std::endl;

    // Disassemble the target image into its components
    typedef itk::VectorIndexSelectionCastImageFilter<TImage, ScalarImageType> TargetDisassemblerType;
    typename TargetDisassemblerType::Pointer targetDisassembler = TargetDisassemblerType::New();
    targetDisassembler->SetIndex(component);
    targetDisassembler->SetInput(image);
    targetDisassembler->Update();

    // Perform the actual filling
    PoissonEditingFilterType poissonFilter;
    poissonFilter.SetTargetImage(targetDisassembler->GetOutput());
    poissonFilter.SetGuidanceField(guidanceFields[component]);
    poissonFilter.SetMask(mask);
    poissonFilter.FillMaskedRegion();

    poissonFilters.push_back(poissonFilter);

    reassembler->SetInput(component, poissonFilters[component].GetOutput());
  }

  reassembler->Update();
//   std::cout << "Output components per pixel: " << reassembler->GetOutput()->GetNumberOfComponentsPerPixel()
//             << std::endl;
//   std::cout << "Output size: " << reassembler->GetOutput()->GetLargestPossibleRegion().GetSize() << std::endl;

  ITKHelpers::DeepCopy(reassembler->GetOutput(), output);
}

/** Specialization for scalar images */
template <typename TPixel>
template <typename TScalarPixel>
void PoissonEditing<TPixel>::FillScalarImage(const itk::Image<TScalarPixel, 2>* const image,
                                             const Mask* const mask,
                                             const GuidanceFieldType* const guidanceField,
                                             itk::Image<TScalarPixel, 2>* const output)
{
  typedef PoissonEditing<TScalarPixel> PoissonEditingFilterType;
  PoissonEditingFilterType poissonFilter;

  poissonFilter.SetTargetImage(image);

  poissonFilter.SetGuidanceField(guidanceField);

  poissonFilter.SetMask(mask);

  // Perform the actual filling
  poissonFilter.FillMaskedRegion();

  ITKHelpers::DeepCopy(poissonFilter.GetOutput(), output);
}

/** For scalar images. */
template <typename TPixel>
template <typename TScalarPixel>
void PoissonEditing<TPixel>::FillImage(const itk::Image<TScalarPixel, 2>* const image, const Mask* const mask,
                                       const PoissonEditing<TPixel>::GuidanceFieldType* const guidanceField,
                                       itk::Image<TScalarPixel, 2>* const output)
{
  FillScalarImage(image, mask, guidanceField, output);
}

/** For vector images with the same guidance field for each channel. */
template <typename TPixel>
template <typename TImage>
void PoissonEditing<TPixel>::
FillImage(const TImage* const image, const Mask* const mask,
          const PoissonEditing<TPixel>::GuidanceFieldType* const guidanceField,
          TImage* const output)
{
  std::vector<GuidanceFieldType*> guidanceFields(image->GetNumberOfComponentsPerPixel(), const_cast<GuidanceFieldType*>(guidanceField));
  FillVectorImage(image, mask, guidanceFields, output);
}

/** For vector images with different guidance fields for each channel. */
template <typename TPixel>
template <typename TImage>
void PoissonEditing<TPixel>::
FillImage(const TImage* const image, const Mask* const mask,
          const std::vector<PoissonEditing<TPixel>::GuidanceFieldType*>& guidanceFields,
          TImage* const output)
{
  // Always call the vector version, as it is the only one that makes sense
  // to have passed a collection of guidance fields.
  FillVectorImage(image, mask, guidanceFields, output);
}

/** For Image<CovariantVector> images. */
template <typename TPixel>
template <typename TComponent, unsigned int NumberOfComponents>
void PoissonEditing<TPixel>::FillImage(const itk::Image<itk::CovariantVector<TComponent,
                                             NumberOfComponents>, 2>* const image,
                                       const Mask* const mask,
                                       const PoissonEditing<TPixel>::GuidanceFieldType* const guidanceField,
                                       itk::Image<itk::CovariantVector<TComponent,
                                             NumberOfComponents>, 2>* const output)
{
  std::vector<GuidanceFieldType*> guidanceFields(image->GetNumberOfComponentsPerPixel(),
                                                 const_cast<GuidanceFieldType*>(guidanceField));
  FillVectorImage(image, mask, guidanceFields, output);
}

#endif
