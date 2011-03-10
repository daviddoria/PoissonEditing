#ifndef HELPERS_H
#define HELPERS_H

#include "Types.h"

#include "itkImage.h"

namespace Helpers
{

template<typename TImage>
void DeepCopy(typename TImage::Pointer input, typename TImage::Pointer output);

template<typename TImage>
void ExtractComponent(typename TImage::Pointer input, unsigned int component, FloatScalarImageType::Pointer output);
//void ExtractComponent(typename TImage::Pointer input, unsigned int component, typename itk::Image<typename TImage::PixelType, 2>::Pointer output);

template<typename TImage>
void SetComponent(typename TImage::Pointer input, unsigned int component, FloatScalarImageType::Pointer componentImage);

template<typename TImage>
void WriteImage(typename TImage::Pointer input, std::string filename);

template<typename TImage>
void CastAndWriteImage(typename TImage::Pointer input, std::string filename);

template<typename TImage>
void CastAndWriteScalarImage(typename TImage::Pointer input, std::string filename);

template<typename TImage>
void ScaleAndWriteScalarImage(typename TImage::Pointer input, std::string filename);

template<typename TImage>
void ClampImage(typename TImage::Pointer image);

std::vector<itk::Offset<2> > Get4NeighborOffsets();

std::vector<itk::Offset<2> > Get8NeighborOffsets();

bool IsOnBorder(itk::ImageRegion<2>, itk::Index<2>);

}

#include "Helpers.txx"

#endif