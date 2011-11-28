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

#ifndef PoissonCloningGUI_H
#define PoissonCloningGUI_H

#include "ui_PoissonCloningGUI.h"

// ITK
#include "itkVectorImage.h"

// Custom
#include "Mask.h"

// Qt
#include <QMainWindow>
class QGraphicsPixmapItem;

class PoissonCloningGUI : public QMainWindow, public Ui::PoissonCloningGUI
{
  Q_OBJECT
public:

  PoissonCloningGUI();
  PoissonCloningGUI(const std::string& sourceImageFileName, const std::string& targetImageFileName, const std::string& maskFileName);
  void DefaultConstructor();
  
public slots:

  void on_actionOpenImage_activated();
  void on_actionSaveResult_activated();
  
  void on_btnClone_clicked();
  void on_chkShowMask_clicked();
  
protected:

  void OpenImages(const std::string& sourceImageFileName, const std::string& targetImageFileName, const std::string& maskFileName);

  void showEvent ( QShowEvent * event );
  void resizeEvent ( QResizeEvent * event );
  
  typedef itk::VectorImage<float,2> ImageType;
  ImageType::Pointer ResultImage;
  ImageType::Pointer SourceImage;
  ImageType::Pointer TargetImage;
  Mask::Pointer MaskImage;

  QGraphicsPixmapItem* SourceImagePixmapItem;
  QGraphicsPixmapItem* TargetImagePixmapItem;
  QGraphicsPixmapItem* MaskImagePixmapItem;
  QGraphicsPixmapItem* ResultPixmapItem;
  
  QGraphicsScene* SourceScene;
  QGraphicsScene* TargetScene;
  QGraphicsScene* ResultScene;

  QImage SelectionImage;
  QGraphicsPixmapItem* SelectionImagePixmapItem;

  std::string SourceImageFileName;
  std::string TargetImageFileName;
  std::string MaskImageFileName;
};

#endif // PoissonEditingGUI_H
