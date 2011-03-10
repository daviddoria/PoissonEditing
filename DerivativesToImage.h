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

/*
 * This class takes the derivative images of an image 'I' along with the border pixels of 'I' and
 * reconstructs the image. 'I' must be a scalar image (i.e. RGB is not allowed).
 */

#ifndef DERIVATIVETOIMAGE_h
#define DERIVATIVETOIMAGE_h

#include "Types.h"
#include "Variable.h"

#include "itkImage.h"
#include "itkCovariantVector.h"
#include "itkNeighborhoodOperator.h"

#include <vector>

template <typename TImage>
class DerivativeToImage
{
public:
  DerivativeToImage();

  // These are the two derivative images
  void SetLaplacianImage(FloatScalarImageType::Pointer image);

  // These are the kernels used to create the derivative images
  void SetLaplacianOperator(itk::NeighborhoodOperator<float,2>* neighborhoodOperator);

  // The image will be reconstructed where the mask is non-zero
  void SetMask(UnsignedCharScalarImageType::Pointer mask);

  // This is the image which contains the boundary pixels
  void SetImage(typename TImage::Pointer image);

  // The main driver function
  void ReconstructImage(typename TImage::Pointer output);

protected:

  FloatScalarImageType::Pointer LaplacianImage;
  itk::NeighborhoodOperator<float,2>* LaplacianOperator;

  typename TImage::Pointer Image;
  typename UnsignedCharScalarImageType::Pointer Mask;

};

#include "DerivativeToImage.txx"

#endif