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

#ifndef HELPERS_H
#define HELPERS_H

#include "itkImage.h"

namespace Helpers
{

template<typename TImage>
void DeepCopy(const TImage* const input, TImage* const output);

template<typename TImage>
void DeepCopyVectorImage(const TImage* const input, TImage* const output);

template<typename TImage>
void WriteImage(const TImage* const input, const std::string& filename);

template<typename TImage>
void CastAndWriteScalarImage(const TImage* const input, const std::string& filename);

template<typename TImage>
void ScaleAndWriteScalarImage(const TImage* const input, const std::string& filename);

template<typename TImage>
void ClampImage(TImage* const image);

template<typename TImage>
void ClampVectorImage(TImage* const image);

std::vector<itk::Offset<2> > Get4NeighborOffsets();

std::vector<itk::Index<2> > GetValid4NeighborIndices(const itk::Index<2>& pixel, const itk::ImageRegion<2>& region);

std::vector<itk::Offset<2> > Get8NeighborOffsets();

bool IsOnBorder(const itk::ImageRegion<2>&, const itk::Index<2>&);

unsigned int CountValid4Neighbors(const itk::Index<2>& pixel, const itk::ImageRegion<2>& region);

template <class TImage>
float MinValue(const TImage* const image);

template <class TImage>
float MaxValue(const TImage* const image);

template <class TImage>
void CentralDifferenceDerivative(const TImage* const image, const unsigned int direction, TImage* const output);

}

#include "Helpers.hxx"

#endif
