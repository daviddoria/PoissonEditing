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

#include "PoissonCloning.h" // Appease syntax parser

// Custom
#include "Helpers.h"

// ITK
#include "itkImageRegionConstIterator.h"
#include "itkLaplacianImageFilter.h"
#include "itkNthElementImageAdaptor.h"

template <typename TImage>
PoissonCloning<TImage>::PoissonCloning() : PoissonEditing<TImage>(), GuidanceField(NULL)
{

}

template <typename TImage>
void PoissonCloning<TImage>::SetTargetImage(TImage* const image)
{
  Helpers::DeepCopy<TImage>(image, this->TargetImage);
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


template <typename TVectorImage>
void CloneAllChannels(const TVectorImage* const image, const TVectorImage* const targetImage,
                      Mask* const mask, TVectorImage* const output)
{
  typedef itk::Image<typename TVectorImage::InternalPixelType, 2> ScalarImageType;
  
  typedef itk::VectorIndexSelectionCastImageFilter<TVectorImage, ScalarImageType> DisassemblerType;
  typedef itk::ImageToVectorImageFilter<ScalarImageType> ReassemblerType;
  typename ReassemblerType::Pointer reassembler = ReassemblerType::New();

  // Perform the Poisson reconstruction on each channel (source/Laplacian pair) independently
  std::vector<PoissonCloning<ScalarImageType> > poissonFilters;

  for(unsigned int component = 0; component < image->GetNumberOfComponentsPerPixel(); component++)
    {
    std::cout << "Component " << component << std::endl;
    // Disassemble the images into their components
    typename DisassemblerType::Pointer sourceDisassembler = DisassemblerType::New();
    sourceDisassembler->SetIndex(component);
    sourceDisassembler->SetInput(image);
    sourceDisassembler->Update();

    typename DisassemblerType::Pointer targetDisassembler = DisassemblerType::New();
    targetDisassembler->SetIndex(component);
    targetDisassembler->SetInput(targetImage);
    targetDisassembler->Update();
    
    PoissonCloning<ScalarImageType> poissonFilter;
    poissonFilters.push_back(poissonFilter);

    poissonFilters[component].SetImage(sourceDisassembler->GetOutput());
    poissonFilters[component].SetTargetImage(targetDisassembler->GetOutput());
    poissonFilters[component].SetMask(mask);
    poissonFilters[component].PasteMaskedRegionIntoTargetImage();

    reassembler->SetNthInput(component, poissonFilters[component].GetOutput());
    }

  reassembler->Update();
  std::cout << "Output components per pixel: " << reassembler->GetOutput()->GetNumberOfComponentsPerPixel() << std::endl;
  std::cout << "Output size: " << reassembler->GetOutput()->GetLargestPossibleRegion().GetSize() << std::endl;

  Helpers::DeepCopyVectorImage(reassembler->GetOutput(), output);
}
