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
#include "IndexComparison.h"

// ITK
#include "itkImageRegionConstIterator.h"
#include "itkLaplacianOperator.h"
#include "itkCastImageFilter.h"
#include "itkVectorIndexSelectionCastImageFilter.h"
#include "itkImageToVectorImageFilter.h"

// Eigen
#include <Eigen/Sparse>
#include <Eigen/UmfPackSupport>
#include <Eigen/SparseExtra>

template <typename TImage>
PoissonEditing<TImage>::PoissonEditing()
{
  this->SourceImage = NULL;
  this->TargetImage = TImage::New();
  this->GuidanceField = FloatScalarImageType::New();
  this->Output = TImage::New();
  this->MaskImage = NULL;
}

template <typename TImage>
void PoissonEditing<TImage>::SetImage(const typename TImage::Pointer image)
{
  // Copy the 'image' to both the source and target image. The target image can be overridden from PoissonCloning.
  this->SourceImage = image;
  Helpers::DeepCopy<TImage>(image, this->TargetImage);
}

template <typename TImage>
void PoissonEditing<TImage>::SetGuidanceField(const FloatScalarImageType::Pointer field)
{
  //this->GuidanceField->Graft(field);
  Helpers::DeepCopy<FloatScalarImageType>(field, this->GuidanceField);
}

template <typename TImage>
void PoissonEditing<TImage>::SetMask(const Mask::Pointer mask)
{
  this->MaskImage = mask;
}

template <typename TImage>
void PoissonEditing<TImage>::SetGuidanceFieldToZero()
{
  // In the hole filling problem, we want the guidance field fo be zero
  FloatScalarImageType::Pointer guidanceField = FloatScalarImageType::New();
  guidanceField->SetRegions(this->SourceImage->GetLargestPossibleRegion());
  guidanceField->Allocate();
  guidanceField->FillBuffer(0);
  this->SetGuidanceField(guidanceField);
}

/*
template <typename TImage>
void PoissonEditing<TImage>::FillMaskedRegion()
{
  // This function is the core of the Poisson Editing algorithm

  //Helpers::WriteImage<TImage>(this->TargetImage, "FillMaskedRegion_TargetImage.mha");

  // Initialize the output by copying the target image into the output. Pixels that are not filled will remain the same in the output.
  Helpers::DeepCopy<TImage>(this->TargetImage, this->Output);
  //Helpers::WriteImage<TImage>(this->Output, "InitializedOutput.mha");
  
  unsigned int width = this->MaskImage->GetLargestPossibleRegion().GetSize()[0];
  unsigned int height = this->MaskImage->GetLargestPossibleRegion().GetSize()[1];

  // This stores the forward mapping from variable id to pixel location
  std::vector<itk::Index<2> > variables;

  for (unsigned int y = 0; y < height; y++)
    {
    for (unsigned int x = 0; x < width; x++)
      {
      itk::Index<2> pixelIndex;
      pixelIndex[0] = x;
      pixelIndex[1] = y;

      if(this->MaskImage->IsHole(pixelIndex))
        {
        variables.push_back(pixelIndex);
        }
      }// end x
    }// end y

  unsigned int numberOfVariables = variables.size();

  if(numberOfVariables == 0)
    {
    std::cerr << "No masked pixels found!" << std::endl;
    return;
    }

  typedef itk::LaplacianOperator<float, 2> LaplacianOperatorType;
  LaplacianOperatorType laplacianOperator;
  itk::Size<2> radius;
  radius.Fill(1);
  laplacianOperator.CreateToRadius(radius);

  // Create the sparse matrix
  Eigen::SparseMatrix<double> A(numberOfVariables, numberOfVariables);
  Eigen::VectorXd b(numberOfVariables);
  
  // Create the reverse mapping from pixel to variable id
  std::map <itk::Index<2>, unsigned int, IndexComparison> PixelToIdMap;
  for(unsigned int i = 0; i < variables.size(); i++)
    {
    PixelToIdMap[variables[i]] = i;
    }

  for(unsigned int variableId = 0; variableId < variables.size(); variableId++)
    {
    itk::Index<2> originalPixel = variables[variableId];
    //std::cout << "originalPixel " << originalPixel << std::endl;
    // The right hand side of the equation starts equal to the value of the guidance field

    double bvalue = this->GuidanceField->GetPixel(originalPixel);

    // Loop over the kernel around the current pixel
    for(unsigned int offset = 0; offset < laplacianOperator.GetSize()[0] * laplacianOperator.GetSize()[1]; offset++)
      {
      if(laplacianOperator.GetElement(offset) == 0)
        {
        continue; // this pixel isn't going to contribute anyway
        }

      itk::Index<2> currentPixel = originalPixel + laplacianOperator.GetOffset(offset);

      if(!this->MaskImage->GetLargestPossibleRegion().IsInside(currentPixel))
        {
        continue; // this pixel is on the border, just ignore it.
        }

      if(this->MaskImage->IsHole(currentPixel))
        {
        // If the pixel is masked, add it as part of the unknown matrix
        A.insert(variableId, PixelToIdMap[currentPixel]) = laplacianOperator.GetElement(offset);
        }
      else
        {
        // If the pixel is known, move its contribution to the known (right) side of the equation
        bvalue -= this->TargetImage->GetPixel(currentPixel) * laplacianOperator.GetElement(offset);
        }
      }
    b[variableId] = bvalue;
  }// end for variables

  // Solve the system with Eigen
  Eigen::VectorXd x(numberOfVariables);
  Eigen::SparseLU<Eigen::SparseMatrix<double>,Eigen::UmfPack> lu_of_A(A);
  if(!lu_of_A.succeeded())
  {
    std::cerr << "decomposiiton failed!" << std::endl;
    return;
  }
  if(!lu_of_A.solve(b,&x))
  {
    std::cerr << "solving failed!" << std::endl;
    return;
  }

  // Convert solution vector back to image
  for(unsigned int i = 0; i < variables.size(); i++)
    {
    this->Output->SetPixel(variables[i], x(i));
    }
} // end FillMaskedRegion
*/


template <typename TImage>
void PoissonEditing<TImage>::FillMaskedRegion()
{
  // This function is the core of the Poisson Editing algorithm

  //Helpers::WriteImage<TImage>(this->TargetImage, "FillMaskedRegion_TargetImage.mha");

  // Initialize the output by copying the target image into the output. Pixels that are not filled will remain the same in the output.
  Helpers::DeepCopy<TImage>(this->TargetImage, this->Output);
  //Helpers::WriteImage<TImage>(this->Output, "InitializedOutput.mha");

  typedef itk::Image<int, 2> IntImageType;
  IntImageType::Pointer variableIdImage = IntImageType::New();
  variableIdImage->SetRegions(this->SourceImage->GetLargestPossibleRegion());
  variableIdImage->Allocate();
  variableIdImage->FillBuffer(-1);

  std::vector<itk::Index<2> > variablePixels;

  itk::ImageRegionIterator<IntImageType> variableIdImageIterator(variableIdImage, variableIdImage->GetLargestPossibleRegion());

  while(!variableIdImageIterator.IsAtEnd())
    {
     itk::Index<2> pixelIndex = variableIdImageIterator.GetIndex();

     if(this->MaskImage->IsHole(pixelIndex))
       {
       variableIdImageIterator.Set(variablePixels.size());
       variablePixels.push_back(pixelIndex);
       }
     ++variableIdImageIterator;
    }

  if(variablePixels.size() == 0)
    {
    std::cerr << "No masked pixels found!" << std::endl;
    return;
    }

  typedef itk::LaplacianOperator<float, 2> LaplacianOperatorType;
  LaplacianOperatorType laplacianOperator;
  itk::Size<2> radius;
  radius.Fill(1);
  laplacianOperator.CreateToRadius(radius);

  // Create the sparse matrix
  Eigen::SparseMatrix<double> A(variablePixels.size(), variablePixels.size());
  Eigen::VectorXd b(variablePixels.size());

  for(unsigned int variableId = 0; variableId < variablePixels.size(); variableId++)
    {
    itk::Index<2> originalPixel = variablePixels[variableId];
    //std::cout << "originalPixel " << originalPixel << std::endl;
    // The right hand side of the equation starts equal to the value of the guidance field

    double bvalue = this->GuidanceField->GetPixel(originalPixel);

    // Loop over the kernel around the current pixel
    for(unsigned int offset = 0; offset < laplacianOperator.GetSize()[0] * laplacianOperator.GetSize()[1]; offset++)
      {
      if(laplacianOperator.GetElement(offset) == 0)
        {
        continue; // this pixel isn't going to contribute anyway
        }

      itk::Index<2> currentPixel = originalPixel + laplacianOperator.GetOffset(offset);

      if(!this->MaskImage->GetLargestPossibleRegion().IsInside(currentPixel))
        {
        continue; // this pixel is on the border, just ignore it.
        }

      if(this->MaskImage->IsHole(currentPixel))
        {
        // If the pixel is masked, add it as part of the unknown matrix
        A.insert(variableId, variableIdImage->GetPixel(currentPixel)) = laplacianOperator.GetElement(offset);
        }
      else
        {
        // If the pixel is known, move its contribution to the known (right) side of the equation
        bvalue -= this->TargetImage->GetPixel(currentPixel) * laplacianOperator.GetElement(offset);
        }
      }
    b[variableId] = bvalue;
  }// end for variables

  // Solve the system with Eigen
  Eigen::VectorXd x(variablePixels.size());
  Eigen::SparseLU<Eigen::SparseMatrix<double>,Eigen::UmfPack> lu_of_A(A);
  if(!lu_of_A.succeeded())
  {
    std::cerr << "decomposiiton failed!" << std::endl;
    return;
  }
  if(!lu_of_A.solve(b,&x))
  {
    std::cerr << "solving failed!" << std::endl;
    return;
  }

  // Convert solution vector back to image
  for(unsigned int i = 0; i < variablePixels.size(); i++)
    {
    this->Output->SetPixel(variablePixels[i], x(i));
    }
} // end FillMaskedRegion


template <typename TImage>
typename TImage::Pointer PoissonEditing<TImage>::GetOutput()
{
  return this->Output;
}

template <typename TImage>
bool PoissonEditing<TImage>::VerifyMask() const
{
  // This function checks that the mask is the same size as the image and that
  // there is no mask on the boundary.

  // Verify that the image and the mask are the same size
  if(this->SourceImage->GetLargestPossibleRegion().GetSize() != this->MaskImage->GetLargestPossibleRegion().GetSize())
    {
    std::cout << "Image size: " << this->SourceImage->GetLargestPossibleRegion().GetSize() << std::endl;
    std::cout << "Mask size: " << this->MaskImage->GetLargestPossibleRegion().GetSize() << std::endl;
    return false;
    }

  // Verify that no border pixels are masked
  itk::ImageRegionConstIterator<Mask> maskIterator(this->MaskImage, this->MaskImage->GetLargestPossibleRegion());

  while(!maskIterator.IsAtEnd())
    {
    if(maskIterator.GetIndex()[0] == 0 ||
      static_cast<unsigned int>(maskIterator.GetIndex()[0]) == this->MaskImage->GetLargestPossibleRegion().GetSize()[0]-1 ||
        maskIterator.GetIndex()[1] == 0 ||
        static_cast<unsigned int>(maskIterator.GetIndex()[1]) == this->MaskImage->GetLargestPossibleRegion().GetSize()[1]-1)
      if(maskIterator.Get())
        {
        std::cout << "Mask is invalid! Pixel " << maskIterator.GetIndex() << " is masked!" << std::endl;
        return false;
        }
    ++maskIterator;
    }

  std::cout << "Mask is valid!" << std::endl;
  return true;

}

template <typename TVectorImage>
void FillAllChannels(const typename TVectorImage::Pointer image, const Mask::Pointer mask, typename TVectorImage::Pointer output)
{
  typedef itk::Image<typename TVectorImage::InternalPixelType, 2> ScalarImageType;
  
  typedef itk::VectorIndexSelectionCastImageFilter<TVectorImage, ScalarImageType> DisassemblerType;
  typedef itk::ImageToVectorImageFilter<ScalarImageType> ReassemblerType;
  typename ReassemblerType::Pointer reassembler = ReassemblerType::New();
  
  // Perform the Poisson reconstruction on each channel (source/Laplacian pair) independently
  std::vector<PoissonEditing<ScalarImageType> > poissonFilters;//(imageReader->GetOutput()->GetNumberOfComponentsPerPixel());
    
  for(unsigned int component = 0; component < image->GetNumberOfComponentsPerPixel(); component++)
    {
    std::cout << "Component " << component << std::endl;
    // Disassemble the image into its components
    typename DisassemblerType::Pointer sourceDisassembler = DisassemblerType::New();
    sourceDisassembler->SetIndex(component);
    sourceDisassembler->SetInput(image);
    sourceDisassembler->Update();
      
    PoissonEditing<ScalarImageType> poissonFilter;
    poissonFilters.push_back(poissonFilter);
  
    poissonFilters[component].SetImage(sourceDisassembler->GetOutput());
    poissonFilters[component].SetGuidanceFieldToZero();
    poissonFilters[component].SetMask(mask);
    poissonFilters[component].FillMaskedRegion();

    reassembler->SetNthInput(component, poissonFilters[component].GetOutput());
    }
  
  reassembler->Update();
  std::cout << "Output components per pixel: " << reassembler->GetOutput()->GetNumberOfComponentsPerPixel() << std::endl;
  std::cout << "Output size: " << reassembler->GetOutput()->GetLargestPossibleRegion().GetSize() << std::endl;
  
  Helpers::DeepCopyVectorImage<TVectorImage>(reassembler->GetOutput(), output);
}
