#include "Variable.h"

Variable::Variable()
{
  this->Component = 0;
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

bool IndexComparison::operator()(const itk::Index<2>& s1, const itk::Index<2>& s2) const
{
  return s1 < s2;
}

bool MyComparison::operator()(const std::pair<itk::Index<2>, unsigned int>& s1, const std::pair<itk::Index<2>, unsigned int>& s2) const
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

