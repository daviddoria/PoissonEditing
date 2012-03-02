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

#include "PoissonEditing.h" // Appease syntax parser

// Custom
#include "Helpers.h"
#include "IndexComparison.h"

// ITK
#include "itkCastImageFilter.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageToVectorImageFilter.h"
#include "itkLaplacianOperator.h"
#include "itkLaplacianImageFilter.h"
#include "itkVectorIndexSelectionCastImageFilter.h"


// Eigen
#include <Eigen/Sparse>
#include <Eigen/UmfPackSupport>
#include <Eigen/SparseExtra>

template <typename TPixel>
PoissonEditing<TPixel>::PoissonEditing()
{
  this->TargetImage = ImageType::New();
  this->Output = ImageType::New();

  this->GuidanceField = GuidanceFieldType::New();
  this->MaskImage = Mask::New();
}

template <typename TPixel>
void PoissonEditing<TPixel>::SetFillMethod(FillMethodEnum fillMethod)
{
  FillMethod = fillMethod;
}

template <typename TPixel>
void PoissonEditing<TPixel>::SetTargetImage(const ImageType* const image)
{
  Helpers::DeepCopy(image, this->TargetImage.GetPointer());
}

template <typename TPixel>
void PoissonEditing<TPixel>::SetGuidanceField(const GuidanceFieldType* const field)
{
  Helpers::DeepCopy(field, this->GuidanceField.GetPointer());
}

template <typename TPixel>
void PoissonEditing<TPixel>::SetSourceImage(const ImageType* const image)
{
  CreateGuidanceFieldFromImage(image);
}

template <typename TPixel>
void PoissonEditing<TPixel>::SetMask(const Mask* const mask)
{
  Helpers::DeepCopy(mask, this->MaskImage.GetPointer());
}

template <typename TPixel>
void PoissonEditing<TPixel>::SetGuidanceFieldToZero()
{
  // In the hole filling problem, we want the guidance field fo be zero
  GuidanceField->SetRegions(this->TargetImage->GetLargestPossibleRegion());
  GuidanceField->Allocate();
  GuidanceField->FillBuffer(0);
}

template <typename TPixel>
void PoissonEditing<TPixel>::FillMaskedRegionVariational()
{
  typedef itk::Image<int, 2> IntImageType;
  IntImageType::Pointer variableIdImage = IntImageType::New();
  variableIdImage->SetRegions(this->TargetImage->GetLargestPossibleRegion());
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

  // Create the sparse matrix
  Eigen::SparseMatrix<double> A(variablePixels.size(), variablePixels.size());
  Eigen::VectorXd b(variablePixels.size());

  for(unsigned int variableId = 0; variableId < variablePixels.size(); variableId++)
    {
    itk::Index<2> currentPixelLocation = variablePixels[variableId];
    //std::cout << "originalPixel " << originalPixel << std::endl;
    // The right hand side of the equation starts equal to the value of the guidance field

    Vector2Type guidanceVectorP = this->GuidanceField->GetPixel(currentPixelLocation);

    std::vector<itk::Index<2> > valid4Neighbors = Helpers::GetValid4NeighborIndices(currentPixelLocation, MaskImage->GetLargestPossibleRegion());

    // This is the |N_p| f_p term - we put the value |N_p| at the column of this variable (variableId) and we are currently setting up the 'variableId' equation/row.
    A.insert(variableId, variableId) = valid4Neighbors.size();
  
    // Loop over the valid neighbors
    float bvalue = 0.0f;
    for(unsigned int neigbhborId = 0; neigbhborId < valid4Neighbors.size(); ++neigbhborId)
      {
      itk::Index<2> currentNeighborLocation = valid4Neighbors[neigbhborId];
      if(this->MaskImage->IsHole(currentNeighborLocation))
        {
        unsigned int neighborVariableId = variableIdImage->GetPixel(currentNeighborLocation);
        A.insert(variableId, neighborVariableId) = -1.0f;
        }
      else
        {
        bvalue += TargetImage->GetPixel(currentNeighborLocation);
        }
      Vector2Type guidanceVectorQ = this->GuidanceField->GetPixel(currentNeighborLocation);
      Vector2Type averageGuidanceVector = (guidanceVectorQ + guidanceVectorP) / 2.0f;
      itk::Offset<2> neighborOffset = currentNeighborLocation - currentPixelLocation;
      bvalue += averageGuidanceVector[0] * neighborOffset[0] + averageGuidanceVector[1] * neighborOffset[1];
      }

    b[variableId] = bvalue;
  }// end for variables

  // Solve the system with Eigen
  Eigen::VectorXd x(variablePixels.size());
  Eigen::SparseLU<Eigen::SparseMatrix<double>,Eigen::UmfPack> lu_of_A(A);
  if(!lu_of_A.succeeded())
  {
    throw std::runtime_error("Decomposiiton failed!");
  }
  if(!lu_of_A.solve(b,&x))
  {
    throw std::runtime_error("Solving failed!");
  }

  // Initialize the output by copying the target image into the output. Pixels that are not filled will remain the same in the output.
  Helpers::DeepCopy(this->TargetImage.GetPointer(), this->Output.GetPointer());
  //Helpers::WriteImage<TImage>(this->Output, "InitializedOutput.mha");

  // Convert solution vector back to image
  for(unsigned int i = 0; i < variablePixels.size(); i++)
    {
    this->Output->SetPixel(variablePixels[i], x(i));
    }
}

template <typename TPixel>
void PoissonEditing<TPixel>::FillMaskedRegionPoisson()
{
  //Helpers::WriteImage<TImage>(this->TargetImage, "FillMaskedRegion_TargetImage.mha");

  // Initialize the output by copying the target image into the output. Pixels that are not filled will remain the same in the output.
  Helpers::DeepCopy(this->TargetImage.GetPointer(), this->Output.GetPointer());
  //Helpers::WriteImage<TImage>(this->Output, "InitializedOutput.mha");

  typedef itk::Image<int, 2> IntImageType;
  IntImageType::Pointer variableIdImage = IntImageType::New();
  variableIdImage->SetRegions(this->TargetImage->GetLargestPossibleRegion());
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

  FloatImageType::Pointer laplacian = FloatImageType::New();
  laplacian->SetRegions(MaskImage->GetLargestPossibleRegion());
  laplacian->Allocate();
  LaplacianFromGradient(GuidanceField, laplacian);
  
  for(unsigned int variableId = 0; variableId < variablePixels.size(); variableId++)
    {
    itk::Index<2> originalPixel = variablePixels[variableId];
    //std::cout << "originalPixel " << originalPixel << std::endl;
    // The right hand side of the equation starts equal to the value of the guidance field

    double bvalue = laplacian->GetPixel(originalPixel);

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
    throw std::runtime_error("Decomposiiton failed!");
  }
  if(!lu_of_A.solve(b,&x))
  {
    throw std::runtime_error("Solving failed!");
  }

  // Convert solution vector back to image
  for(unsigned int i = 0; i < variablePixels.size(); i++)
    {
    this->Output->SetPixel(variablePixels[i], x(i));
    }
} // end FillMaskedRegion


template <typename TPixel>
typename PoissonEditing<TPixel>::ImageType* PoissonEditing<TPixel>::GetOutput()
{
  return this->Output;
}

template <typename TPixel>
bool PoissonEditing<TPixel>::VerifyMask() const
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


template <typename TPixel>
void PoissonEditing<TPixel>::CreateGuidanceFieldFromImage(const FloatScalarImageType* const sourceImage)
{
  typedef itk::LaplacianImageFilter<FloatScalarImageType, FloatScalarImageType>  LaplacianFilterType;

  typename LaplacianFilterType::Pointer laplacianFilter = LaplacianFilterType::New();
  laplacianFilter->SetInput(sourceImage);
  laplacianFilter->Update();
  this->SetGuidanceField(laplacianFilter->GetOutput());
}

template <typename TPixel>
void PoissonEditing<TPixel>::LaplacianFromGradient(const PoissonEditing<TPixel>::GradientImageType* const gradientImage, FloatImageType* const outputLaplacian)
{
  typedef itk::VectorIndexSelectionCastImageFilter<GradientImageType, FloatImageType> IndexSelectionType;
  
  IndexSelectionType::Pointer xFilter = IndexSelectionType::New();
  xFilter->SetIndex(0);
  xFilter->SetInput(gradientImage);
  xFilter->Update();

  IndexSelectionType::Pointer yFilter = IndexSelectionType::New();
  yFilter->SetIndex(1);
  yFilter->SetInput(gradientImage);
  yFilter->Update();

  typedef itk::AddImageFilter<FloatImageType, FloatImageType> AddImageFilterType;
  AddImageFilterType::Pointer addFilter = AddImageFilterType::New ();
  addFilter->SetInput1(xFilter->GetOutput());
  addFilter->SetInput2(yFilter->GetOutput());
  addFilter->Update();

  Helpers::DeepCopy(addFilter->GetOutput(), outputLaplacian);
}



/** This function should be called with none or one of sourceImage or guidanceField non-null. For example, to use a source image, call:
 * FillAllChannels(image, mask, sourceImage, NULL, output)
 * To use a precomputed guidance field, call:
 * FillAllChannels(image, mask, NULL, guidanceField, output)
 * To use a zero guidance field, call:
 * FillAllChannels(image, mask, NULL, NULL, output)
 */
// template <typename TVectorImage, typename TGuidanceField>
// void FillAllChannels(const TVectorImage* const image, const Mask* const mask, const TGuidanceField* const guidanceField,
//                      PoissonEditing<typename TVectorImage::InternalPixelType>::FillMethodEnum fillMethod, TVectorImage* const output)
// {
//   if(!mask)
//   {
//     throw std::runtime_error("You must specify a mask!");
//   }
// 
//   typedef itk::Image<typename TVectorImage::InternalPixelType, 2> ScalarImageType;
//   typedef itk::Image<typename TGuidanceField::InternalPixelType, 2> ScalarGuidanceFieldType;
// 
//   typedef itk::ImageToVectorImageFilter<ScalarImageType> ReassemblerType;
//   typename ReassemblerType::Pointer reassembler = ReassemblerType::New();
// 
//   // Perform the Poisson reconstruction on each channel (source/Laplacian pair) independently
//   typedef PoissonEditing<typename TVectorImage::InternalPixelType> PoissonEditingFilterType;
//   
//   std::vector<PoissonEditingFilterType> poissonFilters;//(imageReader->GetOutput()->GetNumberOfComponentsPerPixel());
//     
//   for(unsigned int component = 0; component < image->GetNumberOfComponentsPerPixel(); component++)
//     {
//     std::cout << "Filling component " << component << std::endl;
//     PoissonEditingFilterType poissonFilter;
//   
//     // Disassemble the target image into its components
//     typedef itk::VectorIndexSelectionCastImageFilter<TVectorImage, ScalarImageType> TargetDisassemblerType;
//     typename TargetDisassemblerType::Pointer targetDisassembler = TargetDisassemblerType::New();
//     targetDisassembler->SetIndex(component);
//     targetDisassembler->SetInput(image);
//     targetDisassembler->Update();
//     poissonFilter.SetTargetImage(targetDisassembler->GetOutput());
//   
//     if(sourceImage)
//     {
//       std::cout << "FillAllChannels::Using source image." << std::endl;
//       typedef itk::VectorIndexSelectionCastImageFilter<TVectorImage, ScalarImageType> SourceDisassemblerType;
//       typename SourceDisassemblerType::Pointer sourceDisassembler = SourceDisassemblerType::New();
//       sourceDisassembler->SetIndex(component);
//       sourceDisassembler->SetInput(sourceImage);
//       sourceDisassembler->Update();
// 
//       poissonFilter.SetSourceImage(sourceDisassembler->GetOutput());
//     }
//     else if(guidanceField)
//     {
//       std::cout << "FillAllChannels::Using guidance field." << std::endl;
//       typedef itk::VectorIndexSelectionCastImageFilter<TGuidanceField, ScalarGuidanceFieldType> GuidanceFieldDisassemblerType;
//       typename GuidanceFieldDisassemblerType::Pointer guidanceFieldDisassembler = GuidanceFieldDisassemblerType::New();
//       guidanceFieldDisassembler->SetIndex(component);
//       guidanceFieldDisassembler->SetInput(guidanceField);
//       guidanceFieldDisassembler->Update();
// 
//       poissonFilter.SetGuidanceField(guidanceFieldDisassembler->GetOutput());
//     }
//     else
//     {
//       std::cout << "FillAllChannels:: Using zero guidance field." << std::endl;
//       poissonFilter.SetGuidanceFieldToZero();
//     }
// 
//     poissonFilter.SetMask(mask);
// 
//     // Perform the actual filling
//     if(FillMethod = VARIATIONAL
//     poissonFilter.FillMaskedRegion();
// 
//     poissonFilters.push_back(poissonFilter);
// 
//     reassembler->SetNthInput(component, poissonFilters[component].GetOutput());
//     }
//   
//   reassembler->Update();
//   std::cout << "Output components per pixel: " << reassembler->GetOutput()->GetNumberOfComponentsPerPixel() << std::endl;
//   std::cout << "Output size: " << reassembler->GetOutput()->GetLargestPossibleRegion().GetSize() << std::endl;
//   
//   Helpers::DeepCopyVectorImage(reassembler->GetOutput(), output);
// }
