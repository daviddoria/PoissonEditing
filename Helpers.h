#ifndef HELPERS_H
#define HELPERS_H

#include "Types.h"

#include "itkImage.h"

namespace Helpers
{


void DeepCopy(FloatVectorImageType::Pointer input, FloatVectorImageType::Pointer output);


template<typename TImage>
void ExtractComponent(typename TImage::Pointer input, unsigned int component, FloatScalarImageType::Pointer output);
//void ExtractComponent(typename TImage::Pointer input, unsigned int component, typename itk::Image<typename TImage::PixelType, 2>::Pointer output);

template<typename TImage>
void WriteImage(typename TImage::Pointer input, std::string filename);

template<typename TImage>
void WriteScaledImage(typename TImage::Pointer input, std::string filename);

template<typename TImage>
void CastAndWriteImage(typename TImage::Pointer input, std::string filename);

template<typename TImage>
void ClampImage(typename TImage::Pointer image);

}

#include "Helpers.txx"

#endif