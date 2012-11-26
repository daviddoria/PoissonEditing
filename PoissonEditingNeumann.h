/*=========================================================================
 *
 *  Copyright David Doria 2012 daviddoria@gmail.com
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

#ifndef PoissonEditingNeumann_H
#define PoissonEditingNeumann_H

#include "PoissonEditing.h"

/** Perform the filling using a different technique. This class should currently be
  * treated as broken. */
template <typename TPixel>
class PoissonEditingNeumann : public PoissonEditing<TPixel>
{
public:

  /** Perform the filling. Use a discretization of the original variational forumulation. */
  void FillMaskedRegion();

};

#include "PoissonEditingNeumann.hpp"

#endif
