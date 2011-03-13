#include "Helpers.h"

#include "itkImageRegionConstIterator.h"
#include "itkLaplacianImageFilter.h"
#include "itkNthElementImageAdaptor.h"

#include <vnl/vnl_vector.h>
#include <vnl/vnl_sparse_matrix.h>
#include <vnl/algo/vnl_sparse_lu.h>

template <typename TImage>
PoissonCloning<TImage>::PoissonCloning()// : PoissonEditing<TImage>()
{
  // Shouldn't have to do this again here, but the TargetImage is created in PoissonEditing constructor and then is NULL when we get here?
  this->SourceImage = TImage::New();
  this->TargetImage = TImage::New();
  this->Output = TImage::New();
  this->GuidanceField = FloatScalarImageType::New();
  this->Mask = UnsignedCharScalarImageType::New();
}

template <typename TImage>
void PoissonCloning<TImage>::SetTargetImage(typename TImage::Pointer image)
{
  //this->TargetImage->Graft(image);
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

  for(unsigned int i = 0; i < TImage::PixelType::Dimension; i++)
    {
    FloatScalarImageType::Pointer sourceComponentImage = FloatScalarImageType::New();
    FloatScalarImageType::Pointer targetComponentImage = FloatScalarImageType::New();
    Helpers::ExtractComponent<TImage>(this->TargetImage, i, targetComponentImage);
    Helpers::ExtractComponent<TImage>(this->SourceImage, i, sourceComponentImage);
    this->CreateGuidanceField(sourceComponentImage);
    this->FillComponent(targetComponentImage);
    Helpers::SetComponent<TImage>(this->Output, i, targetComponentImage);
    }
}
