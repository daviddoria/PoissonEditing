#include "Helpers.h"

#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIterator.h"

namespace Helpers
{

float clamp01(float f)
{
    if (f < 0.0f)
    {
      return 0.0f;
    }
    else if(f > 1.0f)
    {
      return 1.0f;
    }
    else
    {
      return f;
    }
}

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