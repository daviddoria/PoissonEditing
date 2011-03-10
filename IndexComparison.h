#ifndef INDEXCOMPARISON_H
#define INDEXCOMPARISON_H

#include "itkIndex.h"

struct IndexComparison
{
  bool operator()(const itk::Index<2>& s1, const itk::Index<2>& s2) const;
};

bool operator<(const itk::Index<2>& s1, const itk::Index<2>& s2);

#endif