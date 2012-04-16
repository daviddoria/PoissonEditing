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

// Submodules
#include "Helpers/Helpers.h"
#include "ITKHelpers/ITKHelpers.h"

// ITK
#include "itkAddImageFilter.h"
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

  Laplacian = NULL;
}

template <typename TPixel>
void PoissonEditing<TPixel>::SetLaplacian(FloatScalarImageType* const laplacian)
{
  Laplacian = laplacian;
}

template <typename TPixel>
void PoissonEditing<TPixel>::SetFillMethod(FillMethodEnum fillMethod)
{
  FillMethod = fillMethod;
}

template <typename TPixel>
void PoissonEditing<TPixel>::SetTargetImage(const ImageType* const image)
{
  ITKHelpers::DeepCopy(image, this->TargetImage.GetPointer());
}

template <typename TPixel>
void PoissonEditing<TPixel>::SetGuidanceField(const GuidanceFieldType* const field)
{
  ITKHelpers::DeepCopy(field, this->GuidanceField.GetPointer());
}

template <typename TPixel>
void PoissonEditing<TPixel>::SetSourceImage(const ImageType* const image)
{
  CreateGuidanceFieldFromImage(image);
}

template <typename TPixel>
void PoissonEditing<TPixel>::SetMask(const Mask* const mask)
{
  ITKHelpers::DeepCopy(mask, this->MaskImage.GetPointer());
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
void PoissonEditing<TPixel>::FillMaskedRegionNeumann()
{
  typedef std::map<itk::Index<2>, unsigned int, itk::Index<2>::LexicographicCompare> VariableIdMapType;
  VariableIdMapType variableIdMap;

  itk::ImageRegionIterator<Mask> maskIterator(MaskImage, MaskImage->GetLargestPossibleRegion());

  while(!maskIterator.IsAtEnd())
    {
     itk::Index<2> pixelIndex = maskIterator.GetIndex();

     if(this->MaskImage->IsHole(pixelIndex))
       {
       // std::cout << "Adding variable " << variableIdMap.size() << std::endl;
       variableIdMap.insert(VariableIdMapType::value_type(pixelIndex, variableIdMap.size()));
       }
     ++maskIterator;
    }

  if(variableIdMap.size() == 0)
    {
    std::cerr << "No masked pixels found!" << std::endl;
    return;
    }

  std::cout << "There are " << variableIdMap.size() << " variables." << std::endl;

  // Create the sparse matrix
  Eigen::SparseMatrix<double> A(variableIdMap.size(), variableIdMap.size());
  Eigen::VectorXd b(variableIdMap.size());

  unsigned int numberOfInteriorPixels = 0;
  unsigned int numberOfBoundaryPixels = 0;
  
  for(VariableIdMapType::const_iterator iter = variableIdMap.begin(); iter != variableIdMap.end(); ++iter)
    {
    itk::Index<2> currentPixelLocation = iter->first;
    //std::cout << "originalPixel " << originalPixel << std::endl;
    // The right hand side of the equation starts equal to the value of the guidance field

    assert(this->MaskImage->IsHole(currentPixelLocation));
  
    Vector2Type guidanceVectorP = this->GuidanceField->GetPixel(currentPixelLocation);

    // If we are on the boundary, use the Neumann boundary condition only
    // (we cannot use the interior condition because it relies on
    // An actual value (the Dirchlet boundary) outside the hole).
    // What is n_x and n_y? Just 1/0 and 1/0? Or do you have to do an actual blurred boundary normal computation?
    if(MaskImage->HasValid4Neighbor(currentPixelLocation))
    {
      numberOfBoundaryPixels++;
      std::vector<itk::Index<2> > validNeighbors = MaskImage->GetValid4Neighbors(currentPixelLocation);
      float bvalue = 0.0f;
      for(unsigned int neighborId = 0; neighborId < validNeighbors.size(); ++neighborId)
      {
        bvalue += TargetImage->GetPixel(validNeighbors[neighborId]);
      }

      A.insert(variableIdMap[currentPixelLocation], variableIdMap[currentPixelLocation]) = validNeighbors.size();
      b[variableIdMap[currentPixelLocation]] = bvalue;
    }
    else // Internal pixels
    {
      numberOfInteriorPixels++;

      std::vector<itk::Index<2> > valid4Neighbors =
               ITKHelpers::Get4NeighborIndicesInsideRegion(currentPixelLocation,
                                                           MaskImage->GetLargestPossibleRegion());
      // This is the |N_p| f_p term - we put the value |N_p| at the column of this variable (variableId)
      // and we are currently setting up the 'variableId' equation/row.
      A.insert(variableIdMap[currentPixelLocation], variableIdMap[currentPixelLocation]) = valid4Neighbors.size();

      // Loop over the valid neighbors
      float bvalue = 0.0f;
      for(unsigned int neigbhborId = 0; neigbhborId < valid4Neighbors.size(); ++neigbhborId)
        {
        itk::Index<2> currentNeighborLocation = valid4Neighbors[neigbhborId];
        if(this->MaskImage->IsHole(currentNeighborLocation))
          {
          unsigned int neighborVariableId = variableIdMap[currentNeighborLocation];
          A.insert(variableIdMap[currentPixelLocation], neighborVariableId) = -1.0f;
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
      b[variableIdMap[currentPixelLocation]] = bvalue;
    }
  }// end for variables

  std::cout << "There were " << numberOfInteriorPixels << " interior pixels." << std::endl;
  std::cout << "There were " << numberOfBoundaryPixels << " boundary pixels." << std::endl;
  
  // Solve the system with Eigen
  Eigen::VectorXd x(variableIdMap.size());
  Eigen::SparseLU<Eigen::SparseMatrix<double>,Eigen::UmfPack> lu_of_A(A);
  if(!lu_of_A.succeeded())
  {
    throw std::runtime_error("Decomposiiton failed!");
  }
  if(!lu_of_A.solve(b,&x))
  {
    throw std::runtime_error("Solving failed!");
  }

  // Initialize the output by copying the target image into the output.
  // Pixels that are not filled will remain the same in the output.
  ITKHelpers::DeepCopy(this->TargetImage.GetPointer(), this->Output.GetPointer());
  //Helpers::WriteImage<TImage>(this->Output, "InitializedOutput.mha");

  // Convert solution vector back to image
  for(VariableIdMapType::const_iterator iter = variableIdMap.begin(); iter != variableIdMap.end(); ++iter)
    {
    this->Output->SetPixel(iter->first, x(iter->second));
    }
}

template <typename TPixel>
void PoissonEditing<TPixel>::FillMaskedRegionVariational()
{
  typedef std::map<itk::Index<2>, unsigned int, itk::Index<2>::LexicographicCompare> VariableIdMapType;
  VariableIdMapType variableIdMap;

  itk::ImageRegionIterator<Mask> maskIterator(MaskImage, MaskImage->GetLargestPossibleRegion());

  while(!maskIterator.IsAtEnd())
    {
     itk::Index<2> pixelIndex = maskIterator.GetIndex();

     if(this->MaskImage->IsHole(pixelIndex))
       {
       //std::cout << "Adding variable " << variableIdMap.size() << std::endl;
       variableIdMap.insert(VariableIdMapType::value_type(pixelIndex, variableIdMap.size()));
       }
     ++maskIterator;
    }

  if(variableIdMap.size() == 0)
    {
    std::cerr << "No masked pixels found!" << std::endl;
    return;
    }

  // Create the sparse matrix
  Eigen::SparseMatrix<double> A(variableIdMap.size(), variableIdMap.size());
  Eigen::VectorXd b(variableIdMap.size());

  for(VariableIdMapType::const_iterator iter = variableIdMap.begin(); iter != variableIdMap.end(); ++iter)
    {
    itk::Index<2> currentPixelLocation = iter->first;
    unsigned int variableId = iter->second;
    //std::cout << "originalPixel " << originalPixel << std::endl;
    // The right hand side of the equation starts equal to the value of the guidance field

    Vector2Type guidanceVectorP = this->GuidanceField->GetPixel(currentPixelLocation);

    std::vector<itk::Index<2> > valid4Neighbors =
            ITKHelpers::Get4NeighborIndicesInsideRegion(currentPixelLocation,
                                                        MaskImage->GetLargestPossibleRegion());

    // This is the |N_p| f_p term - we put the value |N_p| at the column of this variable (variableId)
    // and we are currently setting up the 'variableId' equation/row.
    A.insert(variableId, variableId) = valid4Neighbors.size();
  
    // Loop over the valid neighbors
    float bvalue = 0.0f;
    for(unsigned int neigbhborId = 0; neigbhborId < valid4Neighbors.size(); ++neigbhborId)
      {
      itk::Index<2> currentNeighborLocation = valid4Neighbors[neigbhborId];
      if(this->MaskImage->IsHole(currentNeighborLocation))
        {
        unsigned int neighborVariableId = variableIdMap[currentNeighborLocation];
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
  Eigen::VectorXd x(variableIdMap.size());
  Eigen::SparseLU<Eigen::SparseMatrix<double>,Eigen::UmfPack> lu_of_A(A);
  if(!lu_of_A.succeeded())
  {
    throw std::runtime_error("Decomposiiton failed!");
  }
  if(!lu_of_A.solve(b,&x))
  {
    throw std::runtime_error("Solving failed!");
  }

  // Initialize the output by copying the target image into the output.
  // Pixels that are not filled will remain the same in the output.
  ITKHelpers::DeepCopy(this->TargetImage.GetPointer(), this->Output.GetPointer());
  //Helpers::WriteImage<TImage>(this->Output, "InitializedOutput.mha");

  // Convert solution vector back to image
  for(VariableIdMapType::const_iterator iter = variableIdMap.begin(); iter != variableIdMap.end(); ++iter)
    {
    this->Output->SetPixel(iter->first, x(iter->second));
    }
}

template <typename TPixel>
void PoissonEditing<TPixel>::FillMaskedRegionPoisson()
{
  //Helpers::WriteImage<TImage>(this->TargetImage, "FillMaskedRegion_TargetImage.mha");

  //Helpers::WriteImage<TImage>(this->Output, "InitializedOutput.mha");

  typedef std::map<itk::Index<2>, unsigned int, itk::Index<2>::LexicographicCompare> VariableIdMapType;
  VariableIdMapType variableIdMap;

  itk::ImageRegionIterator<Mask> maskIterator(MaskImage, MaskImage->GetLargestPossibleRegion());

  while(!maskIterator.IsAtEnd())
    {
     itk::Index<2> pixelIndex = maskIterator.GetIndex();

     if(this->MaskImage->IsHole(pixelIndex))
       {
       //std::cout << "Adding variable " << variableIdMap.size() << std::endl;
       variableIdMap.insert(VariableIdMapType::value_type(pixelIndex, variableIdMap.size()));
       }
     ++maskIterator;
    }

  if(variableIdMap.size() == 0)
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
  Eigen::SparseMatrix<double> A(variableIdMap.size(), variableIdMap.size());
  Eigen::VectorXd b(variableIdMap.size());

  FloatImageType::Pointer laplacian = FloatImageType::New();
  laplacian->SetRegions(MaskImage->GetLargestPossibleRegion());
  laplacian->Allocate();
  if(!Laplacian)
  {
    LaplacianFromGradient(GuidanceField, laplacian);
  }
  else
  {
    ITKHelpers::DeepCopy(Laplacian, laplacian.GetPointer());
  }

  ITKHelpers::WriteImage(laplacian.GetPointer(), "laplacian.mha");

  for(VariableIdMapType::const_iterator iter = variableIdMap.begin(); iter != variableIdMap.end(); ++iter)
    {
    itk::Index<2> originalPixel = iter->first;
    unsigned int variableId = iter->second;
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
        A.insert(variableId, variableIdMap[currentPixel]) = laplacianOperator.GetElement(offset);
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
  Eigen::VectorXd x(variableIdMap.size());
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
  // Initialize the output by copying the target image into the output.
  // Pixels that are not filled will remain the same in the output.
  ITKHelpers::DeepCopy(this->TargetImage.GetPointer(), this->Output.GetPointer());
  
  for(VariableIdMapType::const_iterator iter = variableIdMap.begin(); iter != variableIdMap.end(); ++iter)
    {
    this->Output->SetPixel(iter->first, x(iter->second));
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
void
PoissonEditing<TPixel>::LaplacianFromGradient(const PoissonEditing<TPixel>::GradientImageType* const gradientImage,
                                              FloatImageType* const outputLaplacian)
{
  typedef itk::VectorIndexSelectionCastImageFilter<GradientImageType, FloatImageType> IndexSelectionType;
  
  IndexSelectionType::Pointer xIndexSelectionFilter = IndexSelectionType::New();
  xIndexSelectionFilter->SetIndex(0);
  xIndexSelectionFilter->SetInput(gradientImage);
  xIndexSelectionFilter->Update();

  // The the x derivative of the x derivative (second x partial)
  FloatImageType::Pointer xSecondDerivative = FloatImageType::New();
  ITKHelpers::CentralDifferenceDerivative(xIndexSelectionFilter->GetOutput(), 0, xSecondDerivative.GetPointer());
  
  IndexSelectionType::Pointer yIndexSelectionFilter = IndexSelectionType::New();
  yIndexSelectionFilter->SetIndex(1);
  yIndexSelectionFilter->SetInput(gradientImage);
  yIndexSelectionFilter->Update();

  // The the y derivative of the y derivative (second y partial)
  FloatImageType::Pointer ySecondDerivative = FloatImageType::New();
  ITKHelpers::CentralDifferenceDerivative(yIndexSelectionFilter->GetOutput(), 1, ySecondDerivative.GetPointer());
  
  typedef itk::AddImageFilter<FloatImageType, FloatImageType> AddImageFilterType;
  AddImageFilterType::Pointer addFilter = AddImageFilterType::New ();
  addFilter->SetInput1(xSecondDerivative);
  addFilter->SetInput2(ySecondDerivative);
  addFilter->Update();

  ITKHelpers::DeepCopy(addFilter->GetOutput(), outputLaplacian);
}

template <typename TVectorImage, typename TGuidanceField = itk::Image<itk::CovariantVector<float, 2>, 2> >
void FillAllChannels(const TVectorImage* const image, const Mask* const mask,
                     const std::vector<TGuidanceField*> guidanceFields, TVectorImage* const output)
{
  if(!mask)
  {
    throw std::runtime_error("You must specify a mask!");
  }

  if(guidanceFields.size() != image->GetNumberOfComponentsPerPixel())
  {
    std::stringstream ss;
    ss << "There are " << image->GetNumberOfComponentsPerPixel() << " channels but "
       << guidanceFields.size() << " guidance fields were specified (these must match).";
    throw std::runtime_error(ss.str());
  }
  
  typedef itk::Image<typename TVectorImage::InternalPixelType, 2> ScalarImageType;

  typedef itk::ImageToVectorImageFilter<ScalarImageType> ReassemblerType;
  typename ReassemblerType::Pointer reassembler = ReassemblerType::New();

  // Perform the Poisson reconstruction on each channel independently
  typedef PoissonEditing<typename TVectorImage::InternalPixelType> PoissonEditingFilterType;

  std::vector<PoissonEditingFilterType> poissonFilters;

  for(unsigned int component = 0; component < image->GetNumberOfComponentsPerPixel(); component++)
    {
    std::cout << "Filling component " << component << std::endl;
    PoissonEditingFilterType poissonFilter;

    // Disassemble the target image into its components
    typedef itk::VectorIndexSelectionCastImageFilter<TVectorImage, ScalarImageType> TargetDisassemblerType;
    typename TargetDisassemblerType::Pointer targetDisassembler = TargetDisassemblerType::New();
    targetDisassembler->SetIndex(component);
    targetDisassembler->SetInput(image);
    targetDisassembler->Update();
    poissonFilter.SetTargetImage(targetDisassembler->GetOutput());

    poissonFilter.SetGuidanceField(guidanceFields[component]);

    poissonFilter.SetMask(mask);

    // Perform the actual filling
    poissonFilter.FillMaskedRegionPoisson();

    poissonFilters.push_back(poissonFilter);

    reassembler->SetNthInput(component, poissonFilters[component].GetOutput());
    }

  reassembler->Update();
  std::cout << "Output components per pixel: " << reassembler->GetOutput()->GetNumberOfComponentsPerPixel()
            << std::endl;
  std::cout << "Output size: " << reassembler->GetOutput()->GetLargestPossibleRegion().GetSize() << std::endl;

  ITKHelpers::DeepCopy(reassembler->GetOutput(), output);
}
