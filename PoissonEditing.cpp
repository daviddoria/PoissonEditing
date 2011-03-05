/*
Copyright (C) 2011 David Doria, daviddoria@gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "PoissonEditing.h"
#include "Helpers.h"
#include "Variable.h"

#include "itkImageRegionConstIterator.h"

#include <map>
#include <iostream>

//#include <assert.h>

#if 0
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
#endif

#if 0
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
#endif