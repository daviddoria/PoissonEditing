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

  void SetXDerivative(FloatScalarImageType::Pointer image);
  void SetYDerivative(FloatScalarImageType::Pointer image);

  void SetXDerivativeOperator(itk::NeighborhoodOperator<float,2>* neighborhoodOperator);
  void SetYDerivativeOperator(itk::NeighborhoodOperator<float,2>* neighborhoodOperator);

  void SetImage(typename TImage::Pointer image);

  void ReconstructImage(typename TImage::Pointer output);

protected:

  std::vector<FloatScalarImageType::Pointer> DerivativeImages;
  std::vector<itk::NeighborhoodOperator<float,2>*> DerivativeOperators;

  typename TImage::Pointer Image;

};

#include "DerivativeToImage.txx"

#endif