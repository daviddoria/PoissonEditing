#include "Helpers.h"

#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIterator.h"

namespace Helpers
{


void DeepCopy(FloatVectorImageType::Pointer input, FloatVectorImageType::Pointer output)
{
  output->SetRegions(input->GetLargestPossibleRegion());
  output->Allocate();

  itk::ImageRegionConstIterator<FloatVectorImageType> inputIterator(input, input->GetLargestPossibleRegion());
  itk::ImageRegionIterator<FloatVectorImageType> outputIterator(output, output->GetLargestPossibleRegion());

  while(!inputIterator.IsAtEnd())
    {
    outputIterator.Set(inputIterator.Get());
    ++inputIterator;
    ++outputIterator;
    }
}

} // end namespace