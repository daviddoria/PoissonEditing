#include "Variable.h"
#include "Helpers.h"

#include "itkImageRegionConstIterator.h"
#include "itkLaplacianImageFilter.h"
#include "itkNthElementImageAdaptor.h"

#include <vnl/vnl_vector.h>
#include <vnl/vnl_sparse_matrix.h>
#include <vnl/algo/vnl_sparse_lu.h>

template <typename TImage>
PoissonCloning<TImage>::PoissonCloning()
{
  this->SourceImage = TImage::New();
  this->TargetImage = TImage::New();
  this->Mask = UnsignedCharScalarImageType::New();

  this->NumberOfNeighbors = 4;
}


template <typename TImage>
void PoissonCloning<TImage>::SetNumberOfNeighbors(const unsigned int number)
{
  this->NumberOfNeighbors = number;
}

template <typename TImage>
void PoissonCloning<TImage>::SetSourceImage(typename TImage::Pointer image)
{
  this->SourceImage->Graft(image);
}

template <typename TImage>
void PoissonCloning<TImage>::SetTargetImage(typename TImage::Pointer image)
{
  this->TargetImage->Graft(image);
}

template <typename TImage>
void PoissonCloning<TImage>::SetMask(UnsignedCharScalarImageType::Pointer mask)
{
  this->Mask->Graft(mask);
}

template <typename TImage>
void PoissonCloning<TImage>::PasteMaskedRegionIntoTargetImage(typename TImage::Pointer output)
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
  if(!VerifyMask())
    {
    return;
    }

  // Compute the Laplacian of all channels of the image
  typedef itk::LaplacianImageFilter<FloatScalarImageType, FloatScalarImageType>  LaplacianFilterType;

  std::vector<FloatScalarImageType::Pointer> laplacians;
  for(unsigned int i = 0; i < TImage::PixelType::Dimension; i++)
    {
    FloatScalarImageType::Pointer componentImage = FloatScalarImageType::New();
    Helpers::ExtractComponent<TImage>(this->SourceImage, i, componentImage);

    typename LaplacianFilterType::Pointer laplacianFilter = LaplacianFilterType::New();
    laplacianFilter->SetInput(componentImage);
    laplacianFilter->Update();

    FloatScalarImageType::Pointer laplacian = FloatScalarImageType::New();
    laplacian->Graft(laplacianFilter->GetOutput());
    laplacians.push_back(laplacian);
    }

  unsigned int width = this->SourceImage->GetLargestPossibleRegion().GetSize()[0];
  unsigned int height = this->SourceImage->GetLargestPossibleRegion().GetSize()[1];

  // Build mapping from (x,y) to variables and create the b vector

  std::vector<Variable> variables;
  std::cout << "Pixels have dimension: " << TImage::PixelType::Dimension << std::endl;

  unsigned int variableId = 0;
  for (unsigned int y = 1; y < height-1; y++)
    {
    for (unsigned int x = 1; x < width-1; x++)
      {
      for (unsigned int component = 0; component < TImage::PixelType::Dimension; component++)
        {

        itk::Index<2> pixelIndex;
        pixelIndex[0] = x;
        pixelIndex[1] = y;

        if(this->Mask->GetPixel(pixelIndex)) // The mask is non-zero representing that we want to fill this pixel
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

  std::vector<itk::Offset<2> > neighborOffsets;
  if(this->NumberOfNeighbors == 4)
    {
    neighborOffsets = Helpers::Get4NeighborOffsets();
    }
  else if(this->NumberOfNeighbors == 8)
    {
    neighborOffsets = Helpers::Get8NeighborOffsets();
    }

  for(unsigned int i = 0; i < variables.size(); i++)
    {
    itk::Index<2> originalPixel = variables[i].Pixel;
    itk::Index<2> currentPixel = variables[i].Pixel;

    double bvalue = -laplacians[variables[i].Component]->GetPixel(variables[i].Pixel);

    A(i, variables[i].Id) = this->NumberOfNeighbors;

    for(unsigned int offset = 0; offset < neighborOffsets.size(); offset++)
      {
      currentPixel = originalPixel + neighborOffsets[offset];
      if (this->Mask->GetPixel(currentPixel))
        {
        std::pair<itk::Index<2>, unsigned int> mapping(currentPixel, variables[i].Component);
        A(i, PixelComponentToIdMap[mapping]) = -1.0;
        }
      else
        {
        bvalue += this->TargetImage->GetPixel(currentPixel)[variables[i].Component];
        }
      }

    b[variables[i].Id] = bvalue;
  }// end for variables

  // Solve the system
  std::cout << "Solver::solve: Solving: " << std::endl
          << "Image dimensions: " << width << "x" << height << std::endl
          << " with " << numberOfVariables << " unknowns" << std::endl
          << "Pixel dimension: " << TImage::PixelType::Dimension << std::endl;

  vnl_vector<double> x(b.size());

  vnl_sparse_lu linear_solver(A, vnl_sparse_lu::estimate_condition);

  linear_solver.solve_transpose(b,&x);

  // Convert solution vector back to image
  Helpers::DeepCopy<TImage>(this->TargetImage, output);
  for(unsigned int i = 0; i < variables.size(); i++)
    {
    typename TImage::PixelType pixel = output->GetPixel(variables[i].Pixel);
    pixel[variables[i].Component] = x[variables[i].Id];
    output->SetPixel(variables[i].Pixel, pixel);
    }
}

template <typename TImage>
bool PoissonCloning<TImage>::VerifyMask()
{
  // This function checks that the mask is the same size as the image and that
  // there is no mask on the boundary.

  // Verify that the image and the mask are the same size
  if(this->SourceImage->GetLargestPossibleRegion().GetSize() != this->Mask->GetLargestPossibleRegion().GetSize())
    {
    std::cout << "Image size: " << this->SourceImage->GetLargestPossibleRegion().GetSize() << std::endl;
    std::cout << "Mask size: " << this->Mask->GetLargestPossibleRegion().GetSize() << std::endl;
    return false;
    }

  // Verify that no border pixels are masked
  itk::ImageRegionConstIterator<UnsignedCharScalarImageType> maskIterator(this->Mask, this->Mask->GetLargestPossibleRegion());

  while(!maskIterator.IsAtEnd())
    {
    if(maskIterator.GetIndex()[0] == 0 ||
      static_cast<unsigned int>(maskIterator.GetIndex()[0]) == this->Mask->GetLargestPossibleRegion().GetSize()[0]-1 ||
        maskIterator.GetIndex()[1] == 0 ||
        static_cast<unsigned int>(maskIterator.GetIndex()[1]) == this->Mask->GetLargestPossibleRegion().GetSize()[1]-1)
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