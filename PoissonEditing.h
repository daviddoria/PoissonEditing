/*=========================================================================
 *
 *  Copyright David Doria 2011 daviddoria@gmail.com
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

#ifndef POISSONEDITING_H
#define POISSONEDITING_H

// Inspiration for this technique came from Tommer Leyvand's implementation:
// http://www.leyvand.com/research/adv-graphics/ex1.htm
//
// The method is based on "Poisson Image Editing" paper, Pe'rez et. al. [SIGGRAPH/2003].

// More information can be found here:
// http://en.wikipedia.org/wiki/Discrete_Poisson_equation
// http://www.eecs.berkeley.edu/~demmel/cs267/lecture24/lecture24.html

// Custom
#include "Mask.h"

// ITK
#include "itkCovariantVector.h"
#include "itkImage.h"
#include "itkVectorImage.h"

// STL
#include <vector>

/** This class operates on a single channel image. If you would like to use this technique on a multi-channel image,
 * use the free function FillAllChannels.
 */

template <typename TPixel>
class PoissonEditing
{
public:
  PoissonEditing();

  typedef itk::Image<TPixel, 2> ImageType;
  typedef itk::Image<float, 2> FloatScalarImageType;

  /** Specify the image to fill. */
  void SetTargetImage(const ImageType* const image);

  /** Specify the region in which to fill the image. */
  void SetMask(const Mask* const mask);

  /** Specify a source image from which the guidance field will be produced. */
  void SetSourceImage(const ImageType* const image);
  
  /** Specify a guidance field. */
  void SetGuidanceField(const FloatScalarImageType* const field);

  /** Perform the filling. */
  void FillMaskedRegion();

  /** If no source image is provided, use a zero guidance field. */
  void SetGuidanceFieldToZero();

  /** Get the filled image. */
  typename ImageType::Pointer GetOutput();
  
protected:

  /** Specify an image to act as the source image. */
  void CreateGuidanceFieldFromImage(const FloatScalarImageType* const sourceImage);

  /** Checks that the mask is the same size as the image and that there are no pixels to be filled on the boundary of the image. */
  bool VerifyMask() const;
  
  /** The image in which to fill pixels. */
  typename ImageType::Pointer TargetImage;
  
  /** The result of the algorithm. */
  typename ImageType::Pointer Output;

  /** The guidance field. */
  FloatScalarImageType::Pointer GuidanceField;
  
  // The image specifying which pixels to fill.
  Mask::Pointer MaskImage;
};

/**
 */
template <typename TVectorImage, typename TGuidanceField = itk::VectorImage<float, 2> >
void FillAllChannels(const TVectorImage* const image, const Mask* const mask, const TVectorImage* const sourceImage, const TGuidanceField* const guidanceField, TVectorImage* const output);

#include "PoissonEditing.hpp"

#endif
