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

#ifndef POISSONCLONING_H
#define POISSONCLONING_H

// Inspiration for this technique came from Tommer Leyvand's implementation:
// http://www.leyvand.com/research/adv-graphics/ex1.htm
//
// The method is based on "Poisson Image Editing" paper, Pe'rez et. al. [SIGGRAPH/2003].

// More information can be found here:
// http://en.wikipedia.org/wiki/Discrete_Poisson_equation
// http://www.eecs.berkeley.edu/~demmel/cs267/lecture24/lecture24.html

// Custom
#include "PoissonEditing.h"

// ITK
#include "itkImage.h"
#include "itkCovariantVector.h"

// STL
#include <vector>

template <typename TImage>
class PoissonCloning : public PoissonEditing<TImage>
{
public:
  //using PoissonEditing<TImage>::FloatScalarImageType;
  typedef typename PoissonEditing<TImage>::FloatScalarImageType FloatScalarImageType;
 
  PoissonCloning();
  void SetTargetImage(typename TImage::Pointer image);
  void PasteMaskedRegionIntoTargetImage();

protected:

  void CreateGuidanceField(const typename FloatScalarImageType::Pointer sourceImage);

};

template <typename TVectorImage>
void CloneAllChannels(const TVectorImage* const sourceImage, const TVectorImage* const targetImage,
                      Mask* const mask, TVectorImage* const output);

#include "PoissonCloning.txx"

#endif
