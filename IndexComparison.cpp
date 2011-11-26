/*=========================================================================
 *
 *  Copyright David Doria 2011 daviddoria@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

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
