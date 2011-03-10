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
  this->LaplacianImage = FloatScalarImageType::New();

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
void DerivativeToImage<TImage>::SetLaplacianOperator(itk::NeighborhoodOperator<float,2>* neighborhoodOperator)
{
  this->LaplacianOperator = neighborhoodOperator;
}

template <typename TImage>
void DerivativeToImage<TImage>::SetLaplacianImage(FloatScalarImageType::Pointer image)
{
  this->LaplacianImage->Graft(image);
}


template <typename TImage>
void DerivativeToImage<TImage>::ReconstructImage(typename TImage::Pointer output)
{

  if(this->LaplacianImage->GetLargestPossibleRegion().GetSize() != this->Image->GetLargestPossibleRegion().GetSize())
    {
    std::cerr << "Laplacian image must be the same size as the image!" << std::endl;
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

  unsigned int numberOfVariables = variableId;

  if (numberOfVariables == 0)
    {
    std::cout << "Solver::solve: No missing pixels found\n";
    return;
    }

  // Create the sparse matrix
  Eigen::SparseMatrix<double> A(2*numberOfVariables, numberOfVariables);
  Eigen::VectorXd b(2*numberOfVariables);

  // Create the reverse mapping
  std::map <itk::Index<2>, unsigned int, IndexComparison> PixelToIdMap;
  for(unsigned int i = 0; i < variables.size(); i++)
    {
    PixelToIdMap[variables[i].Pixel] = i;
    }

  unsigned int numberOfNonZeroEntries = 0;
  for(unsigned int variableId = 0; variableId < variables.size(); variableId++)
    {
    itk::Index<2> originalPixel = variables[variableId].Pixel;

    double bvalue = this->LaplacianImage->GetPixel(variables[variableId].Pixel);

    for(unsigned int offset = 0; offset < LaplacianOperator->GetSize()[0] * LaplacianOperator->GetSize()[1]; offset++)
      {
      itk::Index<2> currentPixel = originalPixel + LaplacianOperator->GetOffset(offset);

      if(this->Mask->GetPixel(currentPixel)) // create an entry in the matrix
        {
        A.insert(variableId, PixelToIdMap[currentPixel]) = LaplacianOperator->GetElement(offset);
        numberOfNonZeroEntries++;
        }
      else // this pixel contributes to the 'b' vector
        {
        bvalue -= this->Image->GetPixel(currentPixel) * LaplacianOperator->GetElement(offset);
        }
      }

    b(variableId) = bvalue;
    }// end for variables

  std::cout << "There are " << numberOfNonZeroEntries << " nonzero entries." << std::endl
            << "Image dimensions: " << width << "x" << height << std::endl
            << " with " << numberOfVariables << " unknowns" << std::endl;

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
  Helpers::DeepCopy<TImage>(this->Image, output);
  for(unsigned int i = 0; i < variables.size(); i++)
    {
    //output->SetPixel(variables[i].Pixel, x[variables[i].Id]);
    output->SetPixel(variables[i].Pixel, x(variables[i].Id));
    }
}
