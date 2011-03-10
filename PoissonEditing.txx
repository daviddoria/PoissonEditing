#include "Helpers.h"
#include "IndexComparison.h"

#include "itkImageRegionConstIterator.h"
#include "itkLaplacianOperator.h"

#include <vnl/vnl_vector.h>
#include <vnl/vnl_sparse_matrix.h>
#include <vnl/algo/vnl_sparse_lu.h>

template <typename TImage>
PoissonEditing<TImage>::PoissonEditing()
{
  this->Image = TImage::New();
  this->Output = TImage::New();
  this->Mask = UnsignedCharScalarImageType::New();
}

template <typename TImage>
void PoissonEditing<TImage>::SetImage(typename TImage::Pointer image)
{
  this->Image->Graft(image);
}

template <typename TImage>
void PoissonEditing<TImage>::SetMask(UnsignedCharScalarImageType::Pointer mask)
{
  this->Mask->Graft(mask);
}

template <typename TImage>
void PoissonEditing<TImage>::FillMaskedRegion()
{
  if(!VerifyMask())
    {
    std::cerr << "Invalid mask!" << std::endl;
    return;
    }

  Helpers::DeepCopy<TImage>(this->Image, this->Output);

  for(unsigned int i = 0; i < TImage::PixelType::Dimension; i++)
    {
    FloatScalarImageType::Pointer componentImage = FloatScalarImageType::New();
    Helpers::ExtractComponent<TImage>(this->Image, i, componentImage);
    FillComponent(componentImage);
    Helpers::SetComponent<TImage>(this->Output, i, componentImage);
    }
}

template <typename TImage>
void PoissonEditing<TImage>::FillComponent(FloatScalarImageType::Pointer image)
{

  unsigned int width = image->GetLargestPossibleRegion().GetSize()[0];
  unsigned int height = image->GetLargestPossibleRegion().GetSize()[1];

  // This stores the forward mapping from variable id to pixel location
  std::vector<itk::Index<2> > variables;

  for (unsigned int y = 1; y < height-1; y++)
    {
    for (unsigned int x = 1; x < width-1; x++)
      {
      itk::Index<2> pixelIndex;
      pixelIndex[0] = x;
      pixelIndex[1] = y;

      if(this->Mask->GetPixel(pixelIndex)) // The mask is non-zero representing that we want to fill this pixel
        {
        variables.push_back(pixelIndex);
        }
      }// end x
    }// end y

  unsigned int numberOfVariables = variables.size();

  if (numberOfVariables == 0)
    {
    std::cerr << "No masked pixels found" << std::endl;
    return;
    }

  typedef itk::LaplacianOperator<float, 2> LaplacianOperatorType;
  LaplacianOperatorType laplacianOperator;
  itk::Size<2> radius;
  radius.Fill(1);
  laplacianOperator.CreateToRadius(radius);

  // Create the sparse matrix
  vnl_sparse_matrix<double> A(numberOfVariables, numberOfVariables);
  std::cout << "Matrix is " << A.rows() << " rows x " << A.cols() << " cols." << std::endl;
  vnl_vector<double> b(numberOfVariables);
  std::cout << "b is " << b.size() << std::endl;

  // Create the reverse mapping from pixel to variable id
  std::map <itk::Index<2>, unsigned int, IndexComparison> PixelToIdMap;
  for(unsigned int i = 0; i < variables.size(); i++)
    {
    PixelToIdMap[variables[i]] = i;
    }

  for(unsigned int variableId = 0; variableId < variables.size(); variableId++)
    {
    itk::Index<2> originalPixel = variables[variableId];

    double bvalue = 0.0; // The right hand side of the equation is zero unless the current pixel has any known neighbors

    // Loop over the kernel around the current pixel
    for(unsigned int offset = 0; offset < laplacianOperator.GetSize()[0] * laplacianOperator.GetSize()[1]; offset++)
      {
      if(laplacianOperator.GetElement(offset) == 0)
        {
        continue; // this pixel isn't going to contribute anyway
        }

      itk::Index<2> currentPixel = originalPixel + laplacianOperator.GetOffset(offset);

      if (this->Mask->GetPixel(currentPixel))
        {
        // If the pixel is masked, add it as part of the unknown matrix
        A(variableId, PixelToIdMap[currentPixel]) = laplacianOperator.GetElement(offset);
        }
      else
        {
        // If the pixel is known, move its contribution to the known (right) side of the equation
        bvalue -= image->GetPixel(currentPixel) * laplacianOperator.GetElement(offset);
        }
      }
    b[variableId] = bvalue;
  }// end for variables

  // Solve the system
  std::cout << "Solver::solve: Solving: " << std::endl
          << "Image dimensions: " << width << "x" << height << std::endl
          << " with " << numberOfVariables << " unknowns" << std::endl;

  vnl_vector<double> x(b.size());

  vnl_sparse_lu linear_solver(A, vnl_sparse_lu::estimate_condition);

  linear_solver.solve_transpose(b,&x);

  // Convert solution vector back to image
  for(unsigned int i = 0; i < variables.size(); i++)
    {
    image->SetPixel(variables[i], x[i]);
    }
}

template <typename TImage>
typename TImage::Pointer PoissonEditing<TImage>::GetOutput()
{
  return this->Output;
}

template <typename TImage>
bool PoissonEditing<TImage>::VerifyMask()
{
  // This function checks that the mask is the same size as the image and that
  // there is no mask on the boundary.

  // Verify that the image and the mask are the same size
  if(this->Image->GetLargestPossibleRegion().GetSize() != this->Mask->GetLargestPossibleRegion().GetSize())
    {
    std::cout << "Image size: " << this->Image->GetLargestPossibleRegion().GetSize() << std::endl;
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