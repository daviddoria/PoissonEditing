#ifndef VARIABLE_H
#define VARIABLE_H

#include "itkIndex.h"
#include <vector>

// Every variable in the linear system was produced by a component of a pixel.
struct Variable
{
  itk::Index<2> Pixel;
  unsigned int Id;
  unsigned int Component;
};

unsigned int FindIdFromPixelAndComponent(std::vector<Variable> variables, itk::Index<2> pixel, unsigned int component);

struct MyComparison
{
  bool operator()(const std::pair<itk::Index<2>, unsigned int>& s1, const std::pair<itk::Index<2>, unsigned int>& s2) const;
};

bool operator<(const itk::Index<2>& s1, const itk::Index<2>& s2);

#endif