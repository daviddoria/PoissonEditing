#include "Helpers.h"

#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIterator.h"

namespace Helpers
{

bool IsOnBorder(itk::ImageRegion<2> region, itk::Index<2> index)
{
  if(index[0] == region.GetIndex()[0] || index[1] == region.GetIndex()[1] ||
    index[0] == region.GetIndex()[0] + region.GetSize()[0]-1 || index[1] == region.GetIndex()[1] + region.GetSize()[1]-1)
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