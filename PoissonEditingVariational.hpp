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

#ifndef PoissonEditingVariational_HPP
#define PoissonEditingVariational_HPP

#include "PoissonEditingVariational.h" // Appease syntax parser

// Submodules
#include "Helpers/Helpers.h"
#include "ITKHelpers/ITKHelpers.h"

// ITK
#include "itkAddImageFilter.h"
#include "itkImageRegionConstIterator.h"
#include "itkComposeImageFilter.h"
#include "itkLaplacianOperator.h"
#include "itkLaplacianImageFilter.h"
#include "itkVectorIndexSelectionCastImageFilter.h"

// Eigen
#include <Eigen/Sparse>

template <typename TPixel>
PoissonEditingVariational<TPixel>::PoissonEditingVariational() : PoissonEditing<TPixel>()
{
  this->FillMethod = VARIATIONAL;
}

template <typename TPixel>
void PoissonEditingVariational<TPixel>::FillMaskedRegion()
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
  Eigen::UmfPackLU<Eigen::SparseMatrix<double> > lu_of_A(A);
  Eigen::VectorXd x = lu_of_A.solve(b);
  if(lu_of_A.info() != Eigen::Success)
  {
    throw std::runtime_error("Decomposition failed!");
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

#endif
