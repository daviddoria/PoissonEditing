#include "IndexComparison.h"

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