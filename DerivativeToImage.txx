#include "Variable.h"
#include "Helpers.h"

#include "itkImageRegionConstIterator.h"
#include "itkLaplacianImageFilter.h"
#include "itkNthElementImageAdaptor.h"
#include "itkSobelOperator.h"
#include "itkDerivativeOperator.h"

#include <vnl/vnl_vector.h>
#include <vnl/vnl_sparse_matrix.h>
#include <vnl/algo/vnl_sparse_lu.h>

#include <Eigen/Sparse>
#include <Eigen/UmfPackSupport>
#include <Eigen/SparseExtra>

template <typename TImage>
DerivativeToImage<TImage>::DerivativeToImage()
{
  this->DerivativeOperators.resize(2);

  this->DerivativeImages.resize(2);
  this->DerivativeImages[0] = FloatScalarImageType::New();
  this->DerivativeImages[1] = FloatScalarImageType::New();

  this->Image = TImage::New();
  this->Mask = UnsignedCharScalarImageType::New();
}

template <typename TImage>
void DerivativeToImage<TImage>::SetImage(typename TImage::Pointer image)
{
  this->Image->Graft(image);
}

template <typename TImage>
void DerivativeToImage<TImage>::SetMask(UnsignedCharScalarImageType::Pointer mask)
{
  this->Mask->Graft(mask);
}

template <typename TImage>
void DerivativeToImage<TImage>::SetXDerivativeOperator(itk::NeighborhoodOperator<float,2>* neighborhoodOperator)
{
  this->DerivativeOperators[0] = neighborhoodOperator;
}


template <typename TImage>
void DerivativeToImage<TImage>::SetYDerivativeOperator(itk::NeighborhoodOperator<float,2>* neighborhoodOperator)
{
  this->DerivativeOperators[1] = neighborhoodOperator;
}

template <typename TImage>
void DerivativeToImage<TImage>::SetXDerivative(FloatScalarImageType::Pointer image)
{
  this->DerivativeImages[0]->Graft(image);
}

template <typename TImage>
void DerivativeToImage<TImage>::SetYDerivative(FloatScalarImageType::Pointer image)
{
  this->DerivativeImages[1]->Graft(image);
}

template <typename TImage>
void DerivativeToImage<TImage>::ReconstructImage(typename TImage::Pointer output)
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
  if(this->DerivativeImages[0]->GetLargestPossibleRegion().GetSize() != this->Image->GetLargestPossibleRegion().GetSize() ||
    this->DerivativeImages[1]->GetLargestPossibleRegion().GetSize() != this->Image->GetLargestPossibleRegion().GetSize() )
    {
    std::cerr << "Derivative images must be the same size as the image!" << std::endl;
    exit(-1);
    }

  unsigned int width = this->Image->GetLargestPossibleRegion().GetSize()[0];
  unsigned int height = this->Image->GetLargestPossibleRegion().GetSize()[1];

  // Build mapping from pixels to variables

  std::vector<Variable> variables;

  unsigned int variableId = 0;

  itk::ImageRegionConstIterator<UnsignedCharScalarImageType> maskIterator(this->Mask, this->Mask->GetLargestPossibleRegion());

  while(!maskIterator.IsAtEnd())
    {
    if(maskIterator.Get()) // only consider non-zero mask pixels as unknowns
      {
      if(Helpers::IsOnBorder(this->Mask->GetLargestPossibleRegion(), maskIterator.GetIndex()))
        {
        std::cerr << "Border pixels are not allowed to be unknown!" << std::endl;
        exit(-1);
        }

      // Create an entry in the matrix

      Variable v;
      v.Id = variableId;
      v.Pixel = maskIterator.GetIndex();
      //v.Component = 0; // this is not used in the DerivativeToImage

      variables.push_back(v);

      variableId++;
      }

    ++maskIterator;
    }

#if 0
  for (unsigned int y = 1; y < height-1; y++)
    {
    for (unsigned int x = 1; x < width-1; x++)
      {
      itk::Index<2> pixelIndex;
      pixelIndex[0] = x;
      pixelIndex[1] = y;

      Variable v;
      v.Id = variableId;
      v.Pixel = pixelIndex;
      //v.Component = 0; // this is not used in the DerivativeToImage

      variables.push_back(v);

      variableId++;
      }// end x
    }// end y
#endif

  unsigned int numberOfVariables = variableId;

  if (numberOfVariables == 0)
    {
    std::cout << "Solver::solve: No missing pixels found\n";
    return;
    }

  // Create the sparse matrix
  // Using VNL
  /*
  vnl_sparse_matrix<double> A(2*numberOfVariables, numberOfVariables);// There are two rows for every variable - one for the x derivative and one for the y derivative
  std::cout << "Matrix is " << A.rows() << " rows x " << A.cols() << " cols." << std::endl;
  vnl_vector<double> b(2*numberOfVariables);
  std::cout << "b is " << b.size() << std::endl;
  */

  // Using Eigen
  Eigen::SparseMatrix<double> A(2*numberOfVariables, numberOfVariables);
  Eigen::VectorXd b(2*numberOfVariables);

  // Create the reverse mapping
  std::map <itk::Index<2>, unsigned int, IndexComparison> PixelToIdMap;
  for(unsigned int i = 0; i < variables.size(); i++)
    {
    PixelToIdMap[variables[i].Pixel] = i;
    }

  // We must add two rows for every variable - one for the x derivative and one for the y derivative
  unsigned int numberOfNonZeroEntries = 0;
  for(unsigned int variableId = 0; variableId < variables.size(); variableId++)
    {
    for(unsigned int component = 0; component < 2; component++)
      {
      unsigned int row = variableId*2 + component;
      //std::cout << "Row: " << row << std::endl;

      itk::NeighborhoodOperator<float,2>* derivativeOperator = this->DerivativeOperators[component];

      itk::Index<2> originalPixel = variables[variableId].Pixel;

      double bvalue = this->DerivativeImages[component]->GetPixel(variables[variableId].Pixel);

      for(unsigned int offset = 0; offset < derivativeOperator->GetSize()[0] * derivativeOperator->GetSize()[1]; offset++)
        {
        if(derivativeOperator->GetElement(offset) == 0)
          {
          continue;
          }
        itk::Index<2> currentPixel = originalPixel + derivativeOperator->GetOffset(offset);

        if(this->Mask->GetPixel(currentPixel)) // create an entry in the matrix
          {
          //A(row, PixelToIdMap[currentPixel]) = sobelOperator.GetElement(offset);
          A.insert(row, PixelToIdMap[currentPixel]) = derivativeOperator->GetElement(offset);
          numberOfNonZeroEntries++;
          //std::cout << "Set A value (" << row << ", " << PixelToIdMap[currentPixel] << ") to "
            //                          << sobelOperator.GetElement(offset) << std::endl;
          }
        else // this pixel contributes to the 'b' vector
          {
          bvalue -= this->Image->GetPixel(currentPixel) * derivativeOperator->GetElement(offset); // we subtract here because we are moving the value to the other side of the equation
          }
        }

      //b[row] = bvalue;
      b(row) = bvalue;
      //std::cout << "Set b value." << std::endl;
      } // end for components
    }// end for variables

  std::cout << "There are " << numberOfNonZeroEntries << " nonzero entries." << std::endl;

  std::cout << "Solver::solve: Solving: " << std::endl
          << "Image dimensions: " << width << "x" << height << std::endl
          << " with " << numberOfVariables << " unknowns" << std::endl;

  // Solve the system with VNL
  /*
  vnl_vector<double> x(b.size());
  vnl_sparse_lu linear_solver(A, vnl_sparse_lu::estimate_condition_verbose);
  linear_solver.solve_transpose(b,&x);
  */

  // Solve the system with Eigen
  Eigen::VectorXd x(numberOfVariables);
  Eigen::SparseMatrix<double> A2 = A.adjoint() * A;
  Eigen::VectorXd b2 = A.adjoint() * b;
  Eigen::SparseLU<Eigen::SparseMatrix<double>,Eigen::UmfPack> lu_of_A(A2);
  if(!lu_of_A.succeeded())
  {
    std::cerr << "decomposiiton failed!" << std::endl;
    return;
  }
  if(!lu_of_A.solve(b2,&x))
  {
    std::cerr << "solving failed!" << std::endl;
    return;
  }

  // Convert solution vector back to image
  Helpers::DeepCopy<TImage>(this->Image, output);
  for(unsigned int i = 0; i < variables.size(); i++)
    {
    //output->SetPixel(variables[i].Pixel, x[variables[i].Id]);
    output->SetPixel(variables[i].Pixel, x(variables[i].Id));
    }
}
