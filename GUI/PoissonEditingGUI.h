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

#ifndef PoissonEditingGUI_H
#define PoissonEditingGUI_H

#include "ui_PoissonEditingGUI.h"

// ITK
#include "itkVectorImage.h"

// Custom
#include "Mask.h"

// Qt
#include <QMainWindow>
class QGraphicsPixmapItem;

class PoissonEditingGUI : public QMainWindow, public Ui::PoissonEditingGUI
{
  Q_OBJECT
public:

  PoissonEditingGUI();
  
public slots:

  void on_actionOpenImage_activated();
  void on_actionSaveResult_activated();
  
  void on_btnFill_clicked();
  
  void on_chkShowInput_clicked();
  void on_chkShowOutput_clicked();
  void on_chkShowMask_clicked();
  
protected:
  typedef itk::VectorImage<float,2> ImageType;
  ImageType::Pointer Result;
  ImageType::Pointer Image;
  Mask::Pointer MaskImage;

  QGraphicsPixmapItem* ImagePixmapItem;
  QGraphicsPixmapItem* MaskImagePixmapItem;
  QGraphicsPixmapItem* ResultPixmapItem;
  
  QGraphicsScene* Scene;
};

#endif // PoissonEditingGUI_H
