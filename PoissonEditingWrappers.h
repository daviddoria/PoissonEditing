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

#ifndef PoissonEditingWrappers_H
#define PoissonEditingWrappers_H

#include "PoissonEditing.h"

// Submodules
#include "Mask/Mask.h"

// ITK
#include "itkCovariantVector.h"
#include "itkImage.h"
#include "itkVectorImage.h"

// STL
#include <vector>

/**
* This function performs the hole filling operation on each channel of a VectorImage independently.
* The 'guidanceFields' argument must be the same length as the number of channels of 'image'.
* Each element of the 'guidanceFields' vector is a 2-channel derivative image (channel 0 is the
* x deriviative and channel 1 is the y deriviative.
*/
template <typename TImage>
static void FillVectorImage(const TImage* const image, const Mask* const mask,
                            const std::vector<PoissonEditingParent::GuidanceFieldType*>& guidanceFields, TImage* const output,
                            const itk::ImageRegion<2>& regionToProcess);

/** Overload for scalar images. Note that this takes only a single guidance field instead
  * of a vector of guidance fields. */
template <typename TScalarPixel>
static void FillScalarImage(const itk::Image<TScalarPixel, 2>* const image, const Mask* const mask,
                            const PoissonEditingParent::GuidanceFieldType* const guidanceField,
                            itk::Image<TScalarPixel, 2>* const output, const itk::ImageRegion<2>& regionToProcess);

/** The following functions are overloads that call one of the above functions (FillVectorImage or FillScalarImage) based on the type of images that
  * are passed. */

/** For scalar images. This calls FillScalarImage. */
template <typename TScalarPixel>
static void FillImage(const itk::Image<TScalarPixel, 2>* const image, const Mask* const mask,
                      const PoissonEditingParent::GuidanceFieldType* const guidanceField,
                      itk::Image<TScalarPixel, 2>* const output, const itk::ImageRegion<2>& regionToProcess);

/** For vector images. This calls FillVectorImage with the same guidance field for each channel. */
template <typename TImage>
static void FillImage(const TImage* const image, const Mask* const mask,
                      const PoissonEditingParent::GuidanceFieldType* const guidanceField, TImage* const output,
                      const itk::ImageRegion<2>& regionToProcess);

/** For vector images. This calls FillVectorImage with different guidance fields for each channel. */
template <typename TImage>
static void FillImage(const TImage* const image, const Mask* const mask,
                      const std::vector<PoissonEditingParent::GuidanceFieldType*>& guidanceFields,
                      TImage* const output, const itk::ImageRegion<2>& regionToProcess);

/** For vector images with separate guidance fields specified as smart pointers. */
template <typename TImage>
static void FillImage(const TImage* const image, const Mask* const mask,
                      const std::vector<PoissonEditingParent::GuidanceFieldType::Pointer>& guidanceFields,
                      TImage* const output, const itk::ImageRegion<2>& regionToProcess);

/** For Image<CovariantVector> images. This calls FillVectorImage with the same guidance field for each channel. */
template <typename TComponent, unsigned int NumberOfComponents>
static void FillImage(const itk::Image<itk::CovariantVector<TComponent,
                            NumberOfComponents>, 2>* const image,
                      const Mask* const mask,
                      const PoissonEditingParent::GuidanceFieldType* const guidanceField,
                      itk::Image<itk::CovariantVector<TComponent, NumberOfComponents>, 2>* const output,
                      const itk::ImageRegion<2>& regionToProcess);


#include "PoissonEditingWrappers.hpp"

#endif
