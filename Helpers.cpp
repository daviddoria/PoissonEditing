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

#include "Helpers.h"

#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIterator.h"

namespace Helpers
{

bool IsOnBorder(const itk::ImageRegion<2> region, const itk::Index<2> index)
{
  if(index[0] == region.GetIndex()[0] || index[1] == region.GetIndex()[1] ||
    static_cast<unsigned int>(index[0]) == region.GetIndex()[0] + region.GetSize()[0]-1 ||
    static_cast<unsigned int>(index[1]) == region.GetIndex()[1] + region.GetSize()[1]-1)
    {
    return true;
    }
  return false;
}


std::vector<itk::Offset<2> > Get4NeighborOffsets()
{
  std::vector<itk::Offset<2> > offsets;

  itk::Offset<2> offset;
  offset[0] = -1;
  offset[1] = 0;
  offsets.push_back(offset);

  offset[0] = 1;
  offset[1] = 0;
  offsets.push_back(offset);

  offset[0] = 0;
  offset[1] = -1;
  offsets.push_back(offset);

  offset[0] = 0;
  offset[1] = 1;
  offsets.push_back(offset);

  return offsets;
}

std::vector<itk::Offset<2> > Get8NeighborOffsets()
{
  std::vector<itk::Offset<2> > offsets = Get4NeighborOffsets();

  itk::Offset<2> offset;
  offset[0] = -1;
  offset[1] = 1;
  offsets.push_back(offset);

  offset[0] = 1;
  offset[1] = -1;
  offsets.push_back(offset);

  offset[0] = -1;
  offset[1] = -1;
  offsets.push_back(offset);

  offset[0] = 1;
  offset[1] = 1;
  offsets.push_back(offset);

  return offsets;
}

} // end namespace