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

/** The terminology "targetImage" and "sourceImage" come from Poisson Cloning.
 * To interpret these arguments in a Poisson Filling context, there is no source image
 * (sourceImage must be nullptr), and the targetImage is the image to be filled.
 */
template <typename TImage>
void FillVectorImage(const TImage* const targetImage, const Mask* const mask,
                     const std::vector<PoissonEditingParent::GuidanceFieldType::Pointer>& guidanceFields,
                     TImage* const output, const itk::ImageRegion<2>& regionToProcess,
                     const TImage* const sourceImage)
{
  std::cout << "FillVectorImage()" << std::endl;
  if(!mask)
  {
    throw std::runtime_error("You must specify a mask!");
  }

  if(guidanceFields.size() != targetImage->GetNumberOfComponentsPerPixel())
  {
    std::stringstream ss;
    ss << "There are " << targetImage->GetNumberOfComponentsPerPixel() << " channels but "
       << guidanceFields.size() << " guidance fields were specified (these must match).";
    throw std::runtime_error(ss.str());
  }

  itk::ImageRegion<2> holeBoundingBox =
      ITKHelpers::ComputeBoundingBox(mask, HoleMaskPixelTypeEnum::HOLE);

  // Adjust the hole bounding box to be in the target position
  itk::ImageRegion<2> holeBoundingBoxPositioned = holeBoundingBox;
  holeBoundingBoxPositioned.SetIndex(regionToProcess.GetIndex() +
                                     (holeBoundingBox.GetIndex() - mask->GetLargestPossibleRegion().GetIndex()));

  if(!targetImage->GetLargestPossibleRegion().IsInside(holeBoundingBoxPositioned))
  {
    std::cerr << "Cannot clone at this position! Source image holes are outside of the target image!" << std::endl;
    return;
  }

  // Crop the mask
  Mask::Pointer croppedMask = Mask::New();
  croppedMask->Allocate();
  ITKHelpers::ExtractRegion(mask, holeBoundingBox, croppedMask.GetPointer());
//  std::cout << "croppedMask region: " << croppedMask->GetLargestPossibleRegion() << std::endl;

  // Setup components of the channel-wise processing
  typedef itk::Image<typename TypeTraits<typename TImage::PixelType>::ComponentType, 2> ScalarImageType;

  typedef itk::ComposeImageFilter<ScalarImageType, TImage> ReassemblerType;
  typename ReassemblerType::Pointer reassembler = ReassemblerType::New();

  // Perform the Poisson reconstruction on each channel independently
  typedef typename TypeTraits<typename TImage::PixelType>::ComponentType ComponentType;
  typedef PoissonEditing<ComponentType> PoissonEditingFilterType;

  std::vector<typename ScalarImageType::Pointer> outputChannels(targetImage->GetNumberOfComponentsPerPixel());

  //std::cout << "There are " << targetImage->GetNumberOfComponentsPerPixel() << " components in the output image." << std::endl;
  for(unsigned int component = 0;
      component < targetImage->GetNumberOfComponentsPerPixel(); ++component)
  {
    std::cout << "Filling component " << component << std::endl;

    // Disassemble the target image into its components
    typedef itk::VectorIndexSelectionCastImageFilter<TImage, ScalarImageType>
        TargetDisassemblerType;
    typename TargetDisassemblerType::Pointer targetDisassembler =
        TargetDisassemblerType::New();
    targetDisassembler->SetIndex(component);
    targetDisassembler->SetInput(targetImage);
    targetDisassembler->Update();

    PoissonEditingParent::GuidanceFieldType::Pointer croppedGuidanceField =
        PoissonEditingParent::GuidanceFieldType::New();
    croppedGuidanceField->Allocate();
    ITKHelpers::ExtractRegion(guidanceFields[component].GetPointer(), holeBoundingBox,
                              croppedGuidanceField.GetPointer());
//    ITKHelpers::WriteImage(croppedGuidanceField.GetPointer(), "CroppedGuidanceField_" + std::to_string(component) + ".mha");

    // Perform the actual filling
    PoissonEditingFilterType poissonFilter;
    poissonFilter.SetTargetImage(targetDisassembler->GetOutput());
    poissonFilter.SetRegionToProcess(holeBoundingBoxPositioned);

    // Disassemble the source image into its components
    if(sourceImage)
    {
      std::cout << "Using sourceImage..." << std::endl;
      typedef itk::VectorIndexSelectionCastImageFilter<TImage, ScalarImageType>
          SourceDisassemblerType;
      typename SourceDisassemblerType::Pointer sourceDisassembler =
          SourceDisassemblerType::New();
      sourceDisassembler->SetIndex(component);
      sourceDisassembler->SetInput(sourceImage);
      sourceDisassembler->Update();

      typename ScalarImageType::Pointer croppedSourceImage =
          ScalarImageType::New();
      croppedSourceImage->Allocate();
      ITKHelpers::ExtractRegion(sourceDisassembler->GetOutput(), holeBoundingBox,
                                croppedSourceImage.GetPointer());

      poissonFilter.SetSourceImage(croppedSourceImage.GetPointer());
    }
    else
    {
        std::cout << "No source image provided - assuming Poisson Filling (versus Cloning)." << std::endl;
    }

    poissonFilter.SetGuidanceField(croppedGuidanceField.GetPointer());
    poissonFilter.SetMask(croppedMask.GetPointer());
    poissonFilter.FillMaskedRegion();

    outputChannels[component] = ScalarImageType::New();
    ITKHelpers::DeepCopy(poissonFilter.GetOutput(), outputChannels[component].GetPointer());

    reassembler->SetInput(component, outputChannels[component]);

  } // end loop over components

  reassembler->Update();
//   std::cout << "Output components per pixel: " << reassembler->GetOutput()->GetNumberOfComponentsPerPixel()
//             << std::endl;
//   std::cout << "Output size: " << reassembler->GetOutput()->GetLargestPossibleRegion().GetSize() << std::endl;

//  ITKHelpers::WriteImage(reassembler->GetOutput(), "Output.mha");

  ITKHelpers::DeepCopy(reassembler->GetOutput(), output);
}

/** Specialization for scalar images */
template <typename TScalarPixel>
void FillScalarImage(const itk::Image<TScalarPixel, 2>* const image,
                     const Mask* const mask,
                     const PoissonEditingParent::GuidanceFieldType* const guidanceField,
                     itk::Image<TScalarPixel, 2>* const output,
                     const itk::ImageRegion<2>& regionToProcess,
                     const itk::Image<TScalarPixel, 2>* const sourceImage)
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


/** For scalar images. */
template <typename TScalarPixel>
void FillImage(const itk::Image<TScalarPixel, 2>* const image, const Mask* const mask,
               const PoissonEditingParent::GuidanceFieldType* const guidanceField,
               itk::Image<TScalarPixel, 2>* const output,
               const itk::ImageRegion<2>& regionToProcess,
               const itk::Image<TScalarPixel, 2>* const sourceImage)
{
  FillScalarImage(image, mask, guidanceField, output, regionToProcess, sourceImage);
}

/** For multi-channel images with the same guidance field for each channel. */
template <typename TImage>
void
FillImage(const TImage* const image, const Mask* const mask,
          const PoissonEditingParent::GuidanceFieldType* guidanceField,
          TImage* const output, const itk::ImageRegion<2>& regionToProcess,
          const TImage* const sourceImage)
{
  std::cout << "FillImage with same guidance field for each channel." << std::endl;
  std::vector<PoissonEditingParent::GuidanceFieldType::Pointer>
      guidanceFields(image->GetNumberOfComponentsPerPixel(),
                     const_cast<PoissonEditingParent::GuidanceFieldType*>(guidanceField));
  std::cout << "Duplicated guidance field for each of the "
            << image->GetNumberOfComponentsPerPixel() << " channels." << std::endl;
  FillVectorImage(image, mask, guidanceFields, output, regionToProcess, sourceImage);
}

/** For multi-channel images with different guidance fields for each channel. */
template <typename TImage>
void
FillImage(const TImage* const image, const Mask* const mask,
          const std::vector<PoissonEditingParent::GuidanceFieldType::Pointer>& guidanceFields,
          TImage* const output, const itk::ImageRegion<2>& regionToProcess,
          const TImage* const sourceImage)
{
  // Always call the vector version, as it is the only one that makes sense
  // to have passed a collection of guidance fields.
  FillVectorImage(image, mask, guidanceFields, output, regionToProcess, sourceImage);
}

/** For Image<CovariantVector> images. */
template <typename TComponent, unsigned int NumberOfComponents>
void FillImage(const itk::Image<itk::CovariantVector<TComponent,
                                             NumberOfComponents>, 2>* const image,
               const Mask* const mask,
               const PoissonEditingParent::GuidanceFieldType* const guidanceField,
               itk::Image<itk::CovariantVector<TComponent,
                     NumberOfComponents>, 2>* const output,
               const itk::ImageRegion<2>& regionToProcess,
               const itk::Image<itk::CovariantVector<TComponent,
                     NumberOfComponents>, 2>* const sourceImage)
{
  std::vector<PoissonEditingParent::GuidanceFieldType::Pointer>
      guidanceFields(image->GetNumberOfComponentsPerPixel(),
                     const_cast<PoissonEditingParent::GuidanceFieldType*>(guidanceField));
  FillVectorImage(image, mask, guidanceFields, output, regionToProcess, sourceImage);
}

/** For VectorImage images with the same guidance field for each channel.*/
template <typename TPixel>
static void
FillImage(const itk::VectorImage<TPixel>* const image,
          const Mask* const mask,
          const PoissonEditingParent::GuidanceFieldType* guidanceField,
          itk::VectorImage<TPixel>* const output,
          const itk::ImageRegion<2>& regionToProcess,
          const itk::VectorImage<TPixel>* const sourceImage)
{
    std::vector<PoissonEditingParent::GuidanceFieldType::Pointer>
        guidanceFields(image->GetNumberOfComponentsPerPixel(),
                       const_cast<PoissonEditingParent::GuidanceFieldType*>(guidanceField));
    FillVectorImage(image, mask, guidanceFields, output, regionToProcess, sourceImage);
}

/** For VectorImages with different guidance fields for each channel. */
template <typename TPixel>
void
FillImage(const itk::VectorImage<TPixel>* const image,
          const Mask* const mask,
          const std::vector<PoissonEditingParent::GuidanceFieldType::Pointer>& guidanceFields,
          itk::VectorImage<TPixel>* const output,
          const itk::ImageRegion<2>& regionToProcess,
          const itk::VectorImage<TPixel>* const sourceImage)
{
  FillVectorImage(image, mask, guidanceFields, output, regionToProcess, sourceImage);
}

#endif
