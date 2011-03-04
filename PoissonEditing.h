#ifndef POISSONEDITING_H
#define POISSONEDITING_H

/*
 * To install TAUCS on Fedora 13:
 * sudo yum install compat-libf2c*
 * Make sure /usr/lib is on LD_LIBRARY_PATH
 */

/*
 * A prebuilt TAUCS for Fedora 13:
 * http://rpm.pbone.net/index.php3/stat/4/idpl/15164322/dir/fedora_13/com/taucs-2.2-4.fc13.i686.rpm.html
 */

// =============================================================================
// PoissonEditing - Poisson Image Editing for cloning and image estimation
//
// The following code implements:
// Exercise 1, Advanced Computer Graphics Course (Spring 2005)
// Tel-Aviv University, Israel
// http://www.leyvand.com/research/adv-graphics/ex1.htm
//
// * Based on "Poisson Image Editing" paper, Pe'rez et. al. [SIGGRAPH/2003].
// * The code uses TAUCS, A sparse linear solver library by Sivan Toledo
//   (see http://www.tau.ac.il/~stoledo/taucs)
// =============================================================================
//
// COPYRIGHT NOTICE, DISCLAIMER, and LICENSE:
//
// PoissonEditing : Copyright (C) 2005, Tommer Leyvand (tommerl@gmail.com)
//
// Covered code is provided under this license on an "as is" basis, without
// warranty of any kind, either expressed or implied, including, without
// limitation, warranties that the covered code is free of defects,
// merchantable, fit for a particular purpose or non-infringing. The entire risk
// as to the quality and performance of the covered code is with you. Should any
// covered code prove defective in any respect, you (not the initial developer
// or any other contributor) assume the cost of any necessary servicing, repair
// or correction. This disclaimer of warranty constitutes an essential part of
// this license. No use of any covered code is authorized hereunder except under
// this disclaimer.
//
// Permission is hereby granted to use, copy, modify, and distribute this
// source code, or portions hereof, for any purpose, including commercial
// applications, freely and without fee, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//

#include "Types.h"

#include "itkImage.h"
#include "itkCovariantVector.h"

class PoissonEditing
{
public:
/**
  *	Poisson approximate the missing regions of I denoted by the color c.
  *	@param I input image
  *	@param c the color denoting the missing regions
  *	@param O the output image
  */
  void FillRegion(FloatVectorImageType::Pointer input, UnsignedCharScalarImageType::Pointer mask, FloatVectorImageType::Pointer output);

  bool VerifyMask(FloatVectorImageType::Pointer image, UnsignedCharScalarImageType::Pointer mask);
};

#endif