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

#ifndef PoissonEditingNeumann_HPP
#define PoissonEditingNeumann_HPP

#include "PoissonEditingNeumann.h" // Appease syntax parser

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
PoissonEditingNeumann<TPixel>::PoissonEditingNeumann() : PoissonEditing<TPixel>()
{
  this->FillMethod = VARIATIONAL;
}


template <typename TPixel>
void PoissonEditingNeumann<TPixel>::FillMaskedRegion()
{
  typedef std::map<itk::Index<2>, unsigned int, itk::Index<2>::LexicographicCompare> VariableIdMapType;
  VariableIdMapType variableIdMap;

  itk::ImageRegionIterator<Mask> maskIterator(this->MaskImage, this->MaskImage->GetLargestPossibleRegion());

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
    // What is n_x and n_y? Just 1/0 and 1/0? Or do you have to do an actual
    // blurred boundary normal computation?
    if(this->MaskImage->HasValid4Neighbor(currentPixelLocation))
    {
      numberOfBoundaryPixels++;
      std::vector<itk::Index<2> > validNeighbors = this->MaskImage->GetValid4Neighbors(currentPixelLocation);
      float bvalue = 0.0f;
      for(unsigned int neighborId = 0; neighborId < validNeighbors.size(); ++neighborId)
      {
        bvalue += this->TargetImage->GetPixel(validNeighbors[neighborId]);
      }

      A.insert(variableIdMap[currentPixelLocation], variableIdMap[currentPixelLocation]) = validNeighbors.size();
      b[variableIdMap[currentPixelLocation]] = bvalue;
    }
    else // Internal pixels
    {
      numberOfInteriorPixels++;

      std::vector<itk::Index<2> > valid4Neighbors =
               ITKHelpers::Get4NeighborIndicesInsideRegion(currentPixelLocation,
                                                           this->MaskImage->GetLargestPossibleRegion());
      // This is the |N_p| f_p term - we put the value |N_p| at the column of this variable (variableId)
      // and we are currently setting up the 'variableId' equation/row.
      A.insert(variableIdMap[currentPixelLocation], variableIdMap[currentPixelLocation]) =
          valid4Neighbors.size();

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
