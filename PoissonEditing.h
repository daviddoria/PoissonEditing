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

#ifndef POISSONEDITING_H
#define POISSONEDITING_H

// Submodules
#include "Mask/Mask.h"

// ITK
#include "itkCovariantVector.h"
#include "itkImage.h"
#include "itkVectorImage.h"

// STL
#include <vector>

/** This class operates on a single channel image. If you would like to use this technique on a
  * multi-channel image,use the static FillImage functions.
  * Inspiration for this technique came from Tommer Leyvand's implementation:
  * http://www.leyvand.com/research/adv-graphics/ex1.htm

  * The method is based on "Poisson Image Editing" paper, Pe'rez et. al. [SIGGRAPH/2003].
  * More information can be found here:
  * http://en.wikipedia.org/wiki/Discrete_Poisson_equation
  * http://www.eecs.berkeley.edu/~demmel/cs267/lecture24/lecture24.html
  */

template <typename TPixel>
class PoissonEditing
{
public:

  typedef itk::CovariantVector<float, 2> Vector2Type;
  typedef itk::Image<Vector2Type> Vector2ImageType;
  typedef Vector2ImageType GuidanceFieldType;
  typedef Vector2ImageType GradientImageType;
  typedef itk::Image<float, 2> FloatImageType;

  enum FillMethodEnum {VARIATIONAL, POISSON};
  FillMethodEnum FillMethod;

  PoissonEditing();

  void SetFillMethod(FillMethodEnum fillMethod);

  typedef itk::Image<TPixel, 2> ImageType;
  typedef itk::Image<float, 2> FloatScalarImageType;

  /** Specify the image to fill. */
  void SetTargetImage(const ImageType* const image);

  /** Specify the region in which to fill the image. */
  void SetMask(const Mask* const mask);

  /** Specify a source image from which the guidance field will be produced. */
  void SetSourceImage(const ImageType* const image);

  /** Specify a guidance field. */
  void SetGuidanceField(const GuidanceFieldType* const field);

  /** Perform the filling. Use a discretization of the Poisson equation. */
  void FillMaskedRegion();

  /** If no source image is provided, use a zero guidance field. */
  void SetGuidanceFieldToZero();

  /** Get the filled image. */
  ImageType* GetOutput();

  void SetLaplacian(FloatScalarImageType* const laplacian);

  /**
    * This function performs the hole filling operation on each channel of a VectorImage independently.
    * The 'guidanceFields' argument must be the same length as the number of channels of 'image'.
    * Each element of the 'guidanceFields' vector is a 2-channel derivative image (channel 0 is the
    * x deriviative and channel 1 is the y deriviative.
    */
    template <typename TImage>
    static void FillVectorImage(const TImage* const image, const Mask* const mask,
                                const std::vector<GuidanceFieldType*>& guidanceFields, TImage* const output);

    /** Overload for scalar images. Note that this takes only a single guidance field instead
      * of a vector of guidance fields. */
    template <typename TScalarPixel>
    static void FillScalarImage(const itk::Image<TScalarPixel, 2>* const image, const Mask* const mask,
                                const GuidanceFieldType* const guidanceField,
                                itk::Image<TScalarPixel, 2>* const output);

    /** For scalar images. This calls FillScalarImage. */
    template <typename TScalarPixel>
    static void FillImage(const itk::Image<TScalarPixel, 2>* const image, const Mask* const mask,
                          const GuidanceFieldType* const guidanceField,
                          itk::Image<TScalarPixel, 2>* const output);

    /** For vector images. This calls FillVectorImage. */
    template <typename TImage>
    static void FillImage(const TImage* const image, const Mask* const mask,
                          const GuidanceFieldType* const guidanceField, TImage* const output);

    /** For vector images with different guidance fields for each channel. This calls FillVectorImage. */
    template <typename TImage>
    static void FillImage(const TImage* const image, const Mask* const mask,
                          const std::vector<GuidanceFieldType*>& guidanceFields, TImage* const output);

    /** For Image<CovariantVector> images. This calls FillVectorImage. */
    template <typename TComponent, unsigned int NumberOfComponents>
    static void FillImage(const itk::Image<itk::CovariantVector<TComponent,
                                NumberOfComponents>, 2>* const image,
                          const Mask* const mask,
                          const GuidanceFieldType* const guidanceField,
                          itk::Image<itk::CovariantVector<TComponent, NumberOfComponents>, 2>* const output);


protected:

  void LaplacianFromGradient(const GradientImageType* const gradientImage,
                             FloatImageType* const outputLaplacian);

  /** Specify an image to act as the source image. */
  void CreateGuidanceFieldFromImage(const FloatScalarImageType* const sourceImage);

  /** Checks that the mask is the same size as the image and that there are no pixels to be
    * filled on the boundary of the image. */
  bool VerifyMask() const;

  /** The image in which to fill pixels. */
  typename ImageType::Pointer TargetImage;

  /** The result of the algorithm. */
  typename ImageType::Pointer Output;

  /** The guidance field. */
  Vector2ImageType::Pointer GuidanceField;

  /** The image specifying which pixels to fill. */
  Mask::Pointer MaskImage;

  /** The Laplacian. */
  FloatScalarImageType* Laplacian;
};

#include "PoissonEditing.hpp"

#endif
