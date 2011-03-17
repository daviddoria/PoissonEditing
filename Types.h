#ifndef TYPES_H
#define TYPES_H

#include "itkImage.h"
#include "itkVectorImage.h"
#include "itkCovariantVector.h"

typedef itk::Image<unsigned char, 2> UnsignedCharScalarImageType;
typedef itk::Image<float, 2> FloatScalarImageType;
typedef itk::Image<itk::CovariantVector<float, 1>, 2> FloatSingleChannelImageType;

//typedef itk::Image<itk::CovariantVector<float, 3>, 2> FloatVectorImageType;
typedef itk::VectorImage<float, 2> FloatVectorImageType;
typedef itk::Image<itk::CovariantVector<unsigned char, 3>, 2> UnsignedCharVectorImageType;

#endif