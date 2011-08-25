#include "Helpers.h"

// ITK
#include "itkImageRegionConstIterator.h"
#include "itkLaplacianImageFilter.h"
#include "itkNthElementImageAdaptor.h"

template <typename TImage>
PoissonCloning<TImage>::PoissonCloning() : PoissonEditing<TImage>() // Call the parent constructor
{

}

template <typename TImage>
void PoissonCloning<TImage>::SetTargetImage(typename TImage::Pointer image)
{
  Helpers::DeepCopy<TImage>(image, this->TargetImage);
}

template <typename TImage>
void PoissonCloning<TImage>::CreateGuidanceField(FloatScalarImageType::Pointer sourceImage)
{
  typedef itk::LaplacianImageFilter<FloatScalarImageType, FloatScalarImageType>  LaplacianFilterType;

  typename LaplacianFilterType::Pointer laplacianFilter = LaplacianFilterType::New();
  laplacianFilter->SetInput(sourceImage);
  laplacianFilter->Update();
  this->SetGuidanceField(laplacianFilter->GetOutput());
}

template <typename TImage>
void PoissonCloning<TImage>::PasteMaskedRegionIntoTargetImage()
{
  if(!this->VerifyMask())
    {
    std::cerr << "Invalid mask!" << std::endl;
    return;
    }

  Helpers::DeepCopy<TImage>(this->TargetImage, this->Output);
  Helpers::WriteImage<TImage>(this->Output, "InitialOutput.mha");
  
  this->CreateGuidanceField(this->SourceImage);
  this->FillMaskedRegion();
}
