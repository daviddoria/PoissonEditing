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

#ifndef PoissonEditingWrappers_HPP
#define PoissonEditingWrappers_HPP

#include "PoissonEditingWrappers.h" // Appease syntax parser

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

template <typename TImage>
void FillVectorImage(const TImage* const image, const Mask* const mask,
                     const std::vector<PoissonEditingParent::GuidanceFieldType*>& guidanceFields,
                     TImage* const output, const itk::ImageRegion<2>& regionToProcess)
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

  itk::ImageRegion<2> holeBoundingBox =
      ITKHelpers::ComputeBoundingBox(mask, HoleMaskPixelTypeEnum::HOLE);

  // Adjust the hole bounding box to be in the target position
  itk::ImageRegion<2> holeBoundingBoxPositioned = holeBoundingBox;
  holeBoundingBoxPositioned.SetIndex(regionToProcess.GetIndex() +
                                     (holeBoundingBox.GetIndex() - mask->GetLargestPossibleRegion().GetIndex()));

  if(!image->GetLargestPossibleRegion().IsInside(holeBoundingBoxPositioned))
  {
    std::cerr << "Cannot clone at this position! Source image holes are outside of the target image!" << std::endl;
    return;
  }

  // Crop the mask
  Mask::Pointer croppedMask = Mask::New();
  croppedMask->Allocate();
  ITKHelpers::ExtractRegion(mask, holeBoundingBox, croppedMask.GetPointer());
  std::cout << "croppedMask region: " << croppedMask->GetLargestPossibleRegion() << std::endl;

  // Setup components of the channel-wise processing
  typedef itk::Image<typename TypeTraits<typename TImage::PixelType>::ComponentType, 2> ScalarImageType;

  typedef itk::ComposeImageFilter<ScalarImageType, TImage> ReassemblerType;
  typename ReassemblerType::Pointer reassembler = ReassemblerType::New();

  // Perform the Poisson reconstruction on each channel independently
  typedef typename TypeTraits<typename TImage::PixelType>::ComponentType ComponentType;
  typedef PoissonEditing<ComponentType> PoissonEditingFilterType;

  std::vector<PoissonEditingFilterType> poissonFilters;

  for(unsigned int component = 0;
      component < image->GetNumberOfComponentsPerPixel(); ++component)
  {
    std::cout << "Filling component " << component << std::endl;

    // Disassemble the target image into its components
    typedef itk::VectorIndexSelectionCastImageFilter<TImage, ScalarImageType>
        TargetDisassemblerType;
    typename TargetDisassemblerType::Pointer targetDisassembler =
        TargetDisassemblerType::New();
    targetDisassembler->SetIndex(component);
    targetDisassembler->SetInput(image);
    targetDisassembler->Update();

    PoissonEditingParent::GuidanceFieldType::Pointer croppedGuidanceField =
        PoissonEditingParent::GuidanceFieldType::New();
    croppedGuidanceField->Allocate();
    ITKHelpers::ExtractRegion(guidanceFields[component], holeBoundingBox,
                              croppedGuidanceField.GetPointer());

    // Perform the actual filling
    PoissonEditingFilterType poissonFilter;
    poissonFilter.SetTargetImage(targetDisassembler->GetOutput());
    poissonFilter.SetRegionToProcess(holeBoundingBoxPositioned);
    poissonFilter.SetGuidanceField(croppedGuidanceField.GetPointer());
    poissonFilter.SetMask(croppedMask.GetPointer());
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
template <typename TScalarPixel>
void FillScalarImage(const itk::Image<TScalarPixel, 2>* const image,
                     const Mask* const mask,
                     const PoissonEditingParent::GuidanceFieldType* const guidanceField,
                     itk::Image<TScalarPixel, 2>* const output,
                     const itk::ImageRegion<2>& regionToProcess)
{
  typedef PoissonEditing<TScalarPixel> PoissonEditingFilterType;
  PoissonEditingFilterType poissonFilter;

  poissonFilter.SetTargetImage(image);
  poissonFilter.SetRegionToProcess(regionToProcess);
  poissonFilter.SetGuidanceField(guidanceField);
  poissonFilter.SetMask(mask);

  // Perform the actual filling
  poissonFilter.FillMaskedRegion();

  ITKHelpers::DeepCopy(poissonFilter.GetOutput(), output);
}

/** For vector images with smart pointers to different guidance fields.*/
template <typename TImage>
void FillImage(const TImage* const image, const Mask* const mask,
               const std::vector<PoissonEditingParent::GuidanceFieldType::Pointer>& guidanceFields, TImage* const output,
               const itk::ImageRegion<2>& regionToProcess)
{
  std::vector<PoissonEditingParent::GuidanceFieldType*> guidanceFieldsRaw;
  for(unsigned int i = 0; i < guidanceFields.size(); ++i)
  {
    guidanceFieldsRaw.push_back(guidanceFields[i]);
  }

  FillVectorImage(image, mask, guidanceFieldsRaw, output, regionToProcess);
}

/** For scalar images. */
template <typename TScalarPixel>
void FillImage(const itk::Image<TScalarPixel, 2>* const image, const Mask* const mask,
               const PoissonEditingParent::GuidanceFieldType* const guidanceField,
               itk::Image<TScalarPixel, 2>* const output, const itk::ImageRegion<2>& regionToProcess)
{
  FillScalarImage(image, mask, guidanceField, output, regionToProcess);
}

/** For vector images with the same guidance field for each channel. */
template <typename TImage>
void
FillImage(const TImage* const image, const Mask* const mask,
          const PoissonEditingParent::GuidanceFieldType* const guidanceField,
          TImage* const output, const itk::ImageRegion<2>& regionToProcess)
{
  std::vector<PoissonEditingParent::GuidanceFieldType*>
      guidanceFields(image->GetNumberOfComponentsPerPixel(),
                     const_cast<PoissonEditingParent::GuidanceFieldType*>(guidanceField));
  FillVectorImage(image, mask, guidanceFields, output, regionToProcess);
}

/** For vector images with different guidance fields for each channel. */
template <typename TImage>
void
FillImage(const TImage* const image, const Mask* const mask,
          const std::vector<PoissonEditingParent::GuidanceFieldType*>& guidanceFields,
          TImage* const output, const itk::ImageRegion<2>& regionToProcess)
{
  // Always call the vector version, as it is the only one that makes sense
  // to have passed a collection of guidance fields.
  FillVectorImage(image, mask, guidanceFields, output, regionToProcess);
}

/** For vector images with different guidance fields for each channel. */
template <typename TPixel>
void
FillImage(const itk::VectorImage<TPixel>* const image,
          const Mask* const mask,
          const std::vector<PoissonEditingParent::GuidanceFieldType*>& guidanceFields,
          itk::VectorImage<TPixel>* const output,
          const itk::ImageRegion<2>& regionToProcess)
{
  FillVectorImage(image, mask, guidanceFields, output, regionToProcess);
}

/** For Image<CovariantVector> images. */
template <typename TComponent, unsigned int NumberOfComponents>
void FillImage(const itk::Image<itk::CovariantVector<TComponent,
                                             NumberOfComponents>, 2>* const image,
               const Mask* const mask,
               const PoissonEditingParent::GuidanceFieldType* const guidanceField,
               itk::Image<itk::CovariantVector<TComponent,
                     NumberOfComponents>, 2>* const output,
               const itk::ImageRegion<2>& regionToProcess)
{
  std::vector<PoissonEditingParent::GuidanceFieldType*>
      guidanceFields(image->GetNumberOfComponentsPerPixel(),
                     const_cast<PoissonEditingParent::GuidanceFieldType*>(guidanceField));
  FillVectorImage(image, mask, guidanceFields, output, regionToProcess);
}

#endif
