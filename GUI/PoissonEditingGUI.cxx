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

#include "PoissonEditingGUI.h"

// Custom
#include "FileSelector.h"
#include "HelpersOutput.h"
#include "HelpersQt.h"
#include "Mask.h"
#include "PoissonEditing.h"

// ITK
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

// Qt
#include <QIcon>
#include <QFileDialog>
#include <QGraphicsPixmapItem>

// Default constructor
PoissonEditingGUI::PoissonEditingGUI()
{
  this->setupUi(this);

  this->Image = ImageType::New();
  this->MaskImage = Mask::New();
  this->Result = ImageType::New();

  this->Scene = new QGraphicsScene;
  this->graphicsView->setScene(this->Scene);
  
  this->ImagePixmapItem = NULL;
  this->MaskImagePixmapItem = NULL;
  this->ResultPixmapItem = NULL;
};

void PoissonEditingGUI::on_btnFill_clicked()
{
  FillAllChannels<ImageType>(this->Image, this->MaskImage, this->Result);

  QImage qimage = HelpersQt::GetQImageRGBA<ImageType>(this->Result);

  QPixmap qpixmap = QPixmap::fromImage(qimage);
  this->ResultPixmapItem = this->Scene->addPixmap(qpixmap);
  this->ResultPixmapItem->setVisible(this->chkShowOutput->isChecked());
}

void PoissonEditingGUI::on_actionSaveResult_activated()
{
  // Get a filename to save
  QString fileName = QFileDialog::getSaveFileName(this, "Save File", ".", "Image Files (*.jpg *.jpeg *.bmp *.png *.mha)");

  if(fileName.toStdString().empty())
    {
    std::cout << "Filename was empty." << std::endl;
    return;
    }

  HelpersOutput::WriteImage<ImageType>(this->Result, fileName.toStdString());
  HelpersOutput::WriteRGBImage<ImageType>(this->Result, fileName.toStdString() + ".png");
  this->statusBar()->showMessage("Saved result.");
}


void PoissonEditingGUI::on_actionOpenImage_activated()
{
  FileSelector* fileSelector(new FileSelector);
  fileSelector->exec();

  int result = fileSelector->result();
  if(result) // The user clicked 'ok'
    {
    // Load and display image
    typedef itk::ImageFileReader<ImageType> ImageReaderType;
    ImageReaderType::Pointer imageReader = ImageReaderType::New();
    imageReader->SetFileName(fileSelector->GetImageFileName());
    imageReader->Update();

    Helpers::DeepCopyVectorImage<ImageType>(imageReader->GetOutput(), this->Image);

    QImage qimageImage = HelpersQt::GetQImageRGBA<ImageType>(this->Image);

    QPixmap qpixmapImage = QPixmap::fromImage(qimageImage);

    this->ImagePixmapItem = this->Scene->addPixmap(qpixmapImage);
    this->ImagePixmapItem->setVisible(this->chkShowInput->isChecked());
    
    // Load and display mask
    typedef itk::ImageFileReader<Mask> MaskReaderType;
    MaskReaderType::Pointer maskReader = MaskReaderType::New();
    maskReader->SetFileName(fileSelector->GetMaskFileName());
    maskReader->Update();

    Helpers::DeepCopy<Mask>(maskReader->GetOutput(), this->MaskImage);

    QImage qimageMask = HelpersQt::GetQMaskImage(this->MaskImage);

    QPixmap qpixmapMask = QPixmap::fromImage(qimageMask);

    this->MaskImagePixmapItem = this->Scene->addPixmap(qpixmapMask);
    this->MaskImagePixmapItem->setVisible(this->chkShowMask->isChecked());
    }
  else
    {
    // std::cout << "User clicked cancel." << std::endl;
    // The user clicked 'cancel' or closed the dialog, do nothing.
    }
}

void PoissonEditingGUI::on_chkShowInput_clicked()
{
  if(!this->ImagePixmapItem)
    {
    return;
    }
  this->ImagePixmapItem->setVisible(this->chkShowInput->isChecked());
}

void PoissonEditingGUI::on_chkShowOutput_clicked()
{
  if(!this->ResultPixmapItem)
    {
    return;
    }
  this->ResultPixmapItem->setVisible(this->chkShowOutput->isChecked());
}

void PoissonEditingGUI::on_chkShowMask_clicked()
{
  if(!this->MaskImagePixmapItem)
    {
    return;
    }
  this->MaskImagePixmapItem->setVisible(this->chkShowMask->isChecked());
}
  