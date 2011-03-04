// =============================================================================
// PoissonEditing - Poisson Image Editing for cloning and image estimation
//
// The following code implements:
// Exercise 1, Advanced Computer Graphics Course (Spring 2005)
// Tel-Aviv University, Israel
// http://www.leyvand.com/research/adv-graphics/ex1.htm
//
// * Based on "Poisson Image Editing" paper, Pe'rez et. al. [SIGGRAPH/2003].
// * The code uses TAUCS, A sparse linear solver library by Sivan Toledo
//   (see http://www.tau.ac.il/~stoledo/taucs)
// =============================================================================
//
// COPYRIGHT NOTICE, DISCLAIMER, and LICENSE:
//
// PoissonEditing : Copyright (C) 2005, Tommer Leyvand (tommerl@gmail.com)
//
// Covered code is provided under this license on an "as is" basis, without
// warranty of any kind, either expressed or implied, including, without
// limitation, warranties that the covered code is free of defects,
// merchantable, fit for a particular purpose or non-infringing. The entire risk
// as to the quality and performance of the covered code is with you. Should any
// covered code prove defective in any respect, you (not the initial developer
// or any other contributor) assume the cost of any necessary servicing, repair
// or correction. This disclaimer of warranty constitutes an essential part of
// this license. No use of any covered code is authorized hereunder except under
// this disclaimer.
//
// Permission is hereby granted to use, copy, modify, and distribute this
// source code, or portions hereof, for any purpose, including commercial
// applications, freely and without fee, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//


// http://www.eecs.berkeley.edu/~demmel/cs267/lecture24/lecture24.html
#include "PoissonEditing.h"
#include "Helpers.h"

#include "itkImageRegionConstIterator.h"

#include <map>
#include <iostream>

#include <vnl/vnl_vector.h>
#include <vnl/vnl_sparse_matrix.h>
#include <vnl/algo/vnl_sparse_lu.h>

#include <assert.h>

// Every variable in the linear system was produced by a component of a pixel.
struct Variable
{
  itk::Index<2> Pixel;
  unsigned int Id;
  unsigned int Component;
};

unsigned int FindIdFromPixelAndComponent(std::vector<Variable> variables, itk::Index<2> pixel, unsigned int component);

bool operator<(const itk::Index<2>& s1, const itk::Index<2>& s2)
{
  if((s1[0] < s2[0]))
  {
    return true;
  }

  if(s1[0] == s2[0])
    {
    if(s1[1] < s2[1])
      {
      return true;
      }
    }
  return false;
}

struct MyComparison
{
  bool operator()(const std::pair<itk::Index<2>, unsigned int>& s1, const std::pair<itk::Index<2>, unsigned int>& s2) const
  {
    if((s1.first < s2.first))
    {
      return true;
    }

    if(s1.first == s2.first)
      {
      if(s1.second < s2.second)
        {
        return true;
        }
      }
    return false;
  }
};

void PoissonEditing::FillRegion(FloatVectorImageType::Pointer input, UnsignedCharScalarImageType::Pointer mask, FloatVectorImageType::Pointer output)
{
  /*
   * Variable ids are assigned as follows:
   * Variable 0: 0th component of the 0th unknown pixel
   * Variable 1: 1st component of the 0th unknown pixel
   * ...
   * Variable n-1: (n-1)th component of the 0th unknown pixel
   * Variable n: 0th component of the 1st unknown pixel
   * Variable n+1: 1st component of the 1st unknown pixel
   * ...
   */
  if(!VerifyMask(input, mask))
    {
    return;
    }

  unsigned int width = input->GetLargestPossibleRegion().GetSize()[0];
  unsigned int height = input->GetLargestPossibleRegion().GetSize()[1];

  // Build mapping from (x,y) to variables and create the b vector
  
  std::vector<Variable> variables;
  
  unsigned int variableId = 0;
  for (unsigned int y = 1; y < height-1; y++)
    {
    for (unsigned int x = 1; x < width-1; x++)
      {
      for (unsigned int component = 0; component < FloatVectorImageType::PixelType::Dimension; component++)
        {
        itk::Index<2> pixelIndex;
        pixelIndex[0] = x;
        pixelIndex[1] = y;

        if(mask->GetPixel(pixelIndex)) // The mask is non-zero representing that we want to fill this pixel
          {
          Variable v;
          v.Id = variableId;
          v.Pixel = pixelIndex;
          v.Component = component;

          variables.push_back(v);

          variableId++;
          }
        }// end for comonent
      }// end x
    }// end y

  unsigned int numberOfVariables = variableId;

  if (numberOfVariables == 0)
    {
    std::cout << "Solver::solve: No missing pixels found\n";
    return;
    }

  // Create the sparse matrix
  vnl_sparse_matrix<double> A(numberOfVariables, numberOfVariables);
  std::cout << "Matrix is " << A.rows() << " rows x " << A.cols() << " cols." << std::endl;
  vnl_vector<double> b(numberOfVariables);
  std::cout << "b is " << b.size() << std::endl;

  std::map <std::pair<itk::Index<2>, unsigned int>, unsigned int, MyComparison> PixelComponentToIdMap;
  for(unsigned int i = 0; i < variables.size(); i++)
    {
    std::pair<itk::Index<2>, unsigned int> mapping(variables[i].Pixel, variables[i].Component);
    PixelComponentToIdMap[mapping] = i;
    }
  
  for(unsigned int i = 0; i < variables.size(); i++)
    {
    std::cout << "Constructing matrix values for variable " << i << " of " << variables.size() << std::endl;
    itk::Index<2> originalPixel = variables[i].Pixel;
    itk::Index<2> currentPixel = variables[i].Pixel;

    double bvalue = 0.0;
  
    // Setup a row of A that looks like -f_{x,y-1} - f_{x-1,y} + 4f_{x,y} - f_{x+1,y} - f_{x,y+1}
    A(i, variables[i].Id) = 4.0;
    bvalue += input->GetPixel(currentPixel)[variables[i].Component];

    currentPixel = originalPixel;
    currentPixel[1] -= 1;
    if (mask->GetPixel(currentPixel))
      {
      //A(i, FindIdFromPixelAndComponent(variables, currentPixel, variables[i].Component)) = -1.0;
      std::pair<itk::Index<2>, unsigned int> mapping(variables[i].Pixel, variables[i].Component);
      A(i, PixelComponentToIdMap[mapping]) = -1.0;
      bvalue += input->GetPixel(currentPixel)[variables[i].Component];
      }

    currentPixel = originalPixel;
    currentPixel[0] -= 1;
    if (mask->GetPixel(currentPixel))
      {
      std::pair<itk::Index<2>, unsigned int> mapping(variables[i].Pixel, variables[i].Component);
      A(i, PixelComponentToIdMap[mapping]) = -1.0;
      bvalue += input->GetPixel(currentPixel)[variables[i].Component];
      }

    currentPixel = originalPixel;
    currentPixel[0] += 1;
    if (mask->GetPixel(currentPixel))
      {
      std::pair<itk::Index<2>, unsigned int> mapping(variables[i].Pixel, variables[i].Component);
      A(i, PixelComponentToIdMap[mapping]) = -1.0;
      bvalue += input->GetPixel(currentPixel)[variables[i].Component];
      }

    currentPixel = originalPixel;
    currentPixel[1] += 1;
    if (mask->GetPixel(currentPixel))
      {
      std::pair<itk::Index<2>, unsigned int> mapping(variables[i].Pixel, variables[i].Component);
      A(i, PixelComponentToIdMap[mapping]) = -1.0;
      bvalue += input->GetPixel(currentPixel)[variables[i].Component];
      }

    b[variables[i].Id] = bvalue;
  }// end for variables

  // Solve the system
  std::cout << "Solver::solve: Solving: " << std::endl
          << "Image dimensions: " << width << "x" << height << std::endl
          << " with " << numberOfVariables << " unknowns" << std::endl
          << "Pixel dimension: " << FloatVectorImageType::PixelType::Dimension << std::endl;

  vnl_vector<double> x(b.size());

  vnl_sparse_lu linear_solver(A, vnl_sparse_lu::estimate_condition);
  std::cout << "Determinant: " << linear_solver.determinant() << std::endl;
  //std::cout << "Rcond: " << linear_solver.rcond() << std::endl;
  //std::cout << "Upbnd: " << linear_solver.max_error_bound() << std::endl;
  
  linear_solver.solve(b,&x);
  
  // Convert solution vector back to image
  Helpers::DeepCopy(input, output);
  for(unsigned int i = 0; i < variables.size(); i++)
    {
    FloatVectorImageType::PixelType pixel = output->GetPixel(variables[i].Pixel);
    pixel[variables[i].Component] = x[variables[i].Id];
    output->SetPixel(variables[i].Pixel, pixel);
    }
}


/**
 *  Verifies the validity of the mask. Checks that:
 *  - The binary mask is the same size as the image.
 *  - There is no mask on the boundary.
 *  @return 'false' if verfication failed, 'true' if verification
 *          passed.
 */
bool PoissonEditing::VerifyMask(FloatVectorImageType::Pointer image, UnsignedCharScalarImageType::Pointer mask)
{
  // Verify that the image and the mask are the same size
  if(image->GetLargestPossibleRegion().GetSize() != mask->GetLargestPossibleRegion().GetSize())
    {
    std::cout << "Image size: " << image->GetLargestPossibleRegion().GetSize() << std::endl;
    std::cout << "Mask size: " << mask->GetLargestPossibleRegion().GetSize() << std::endl;
    return false;
    }

  // Verify that no border pixels are masked
  itk::ImageRegionConstIterator<UnsignedCharScalarImageType> maskIterator(mask, mask->GetLargestPossibleRegion());

  while(!maskIterator.IsAtEnd())
    {
    if(maskIterator.GetIndex()[0] == 0 || maskIterator.GetIndex()[0] == mask->GetLargestPossibleRegion().GetSize()[0]-1 ||
      maskIterator.GetIndex()[1] == 0 || maskIterator.GetIndex()[1] == mask->GetLargestPossibleRegion().GetSize()[1]-1)
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


unsigned int FindIdFromPixelAndComponent(std::vector<Variable> variables, itk::Index<2> pixel, unsigned int component)
{
  for(unsigned int i = 0; i < variables.size(); i++)
    {
    if(variables[i].Pixel == pixel && variables[i].Component == component)
      {
      return variables[i].Id;
      }
    }
  std::cerr << "FindIdFromPixelAndComponent failed!" << std::endl;
  exit(-1);

  // This is to make the compiler not complain about a non-void function not returning a value
  return 0;
}