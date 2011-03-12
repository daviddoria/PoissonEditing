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

#ifndef POISSONEDITING_H
#define POISSONEDITING_H

// Inspiration for this technique came from Tommer Leyvand's implementation:
// http://www.leyvand.com/research/adv-graphics/ex1.htm
//
// The method is based on "Poisson Image Editing" paper, Pe'rez et. al. [SIGGRAPH/2003].

// More information can be found here:
// http://en.wikipedia.org/wiki/Discrete_Poisson_equation
// http://www.eecs.berkeley.edu/~demmel/cs267/lecture24/lecture24.html
#include "Types.h"

#include "itkImage.h"
#include "itkCovariantVector.h"

#include <vector>

template <typename TImage>
class PoissonEditing
{
public:
  PoissonEditing();

  void SetImage(typename TImage::Pointer image);
  void SetMask(UnsignedCharScalarImageType::Pointer mask);
  void SetGuidanceField(FloatScalarImageType::Pointer field);
  void SetGuidanceFieldToZero();

  void FillMaskedRegion();

  typename TImage::Pointer GetOutput();

protected:

  bool VerifyMask();

  void FillComponent(FloatScalarImageType::Pointer image);

  typename TImage::Pointer SourceImage;
  typename TImage::Pointer TargetImage;
  typename TImage::Pointer Output;
  FloatScalarImageType::Pointer GuidanceField;
  UnsignedCharScalarImageType::Pointer Mask;
};

#include "PoissonEditing.txx"

#endif