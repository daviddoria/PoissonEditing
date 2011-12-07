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
#include "ImageFileSelector.h"
#include "HelpersOutput.h"
#include "HelpersQt.h"
#include "Mask.h"
#include "PoissonEditing.h"
#include "PoissonEditingComputationThread.h"
#include "PoissonEditingComputationObject.h"

// ITK
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

// Qt
#include <QIcon>
#include <QFileDialog>
#include <QGraphicsPixmapItem>

// Called by all constructors
void PoissonEditingGUI::DefaultConstructor()
{
  std::cout << "DefaultConstructor()" << std::endl;
  this->setupUi(this);

  this->progressBar->setMinimum(0);
  this->progressBar->setMaximum(0);
  this->progressBar->hide();

  this->ComputationThread = new PoissonEditingComputationThreadClass;
  connect(this->ComputationThread, SIGNAL(StartProgressBarSignal()), this, SLOT(slot_StartProgressBar()));
  connect(this->ComputationThread, SIGNAL(StopProgressBarSignal()), this, SLOT(slot_StopProgressBar()));
  connect(this->ComputationThread, SIGNAL(IterationCompleteSignal()), this, SLOT(slot_IterationComplete()));

  this->Image = ImageType::New();
  this->MaskImage = Mask::New();
  this->Result = ImageType::New();

  this->Scene = new QGraphicsScene;
  this->graphicsView->setScene(this->Scene);

  this->ImagePixmapItem = NULL;
  this->MaskImagePixmapItem = NULL;
  this->ResultPixmapItem = NULL;
}

// Default constructor
PoissonEditingGUI::PoissonEditingGUI()
{
  std::cout << "PoissonEditingGUI()" << std::endl;
  DefaultConstructor();
};

PoissonEditingGUI::PoissonEditingGUI(const std::string& imageFileName, const std::string& maskFileName)
{
  std::cout << "PoissonEditingGUI(string, string)" << std::endl;
  DefaultConstructor();
  this->SourceImageFileName = imageFileName;
  this->MaskImageFileName = maskFileName;
  OpenImageAndMask(this->SourceImageFileName, this->MaskImageFileName);
}

void PoissonEditingGUI::showEvent ( QShowEvent * event )
{
  if(this->ImagePixmapItem)
    {
    this->graphicsView->fitInView(this->ImagePixmapItem, Qt::KeepAspectRatio);
    }
}

void PoissonEditingGUI::resizeEvent ( QResizeEvent * event )
{
  if(this->ImagePixmapItem)
    {
    this->graphicsView->fitInView(this->ImagePixmapItem, Qt::KeepAspectRatio);
    }
}

void PoissonEditingGUI::on_btnFill_clicked()
{
  ComputationThread->Operation = ComputationThreadClass::ALLSTEPS;
  PoissonEditingComputationObject* computationObject = new PoissonEditingComputationObject;
  computationObject->MaskImage = this->MaskImage;
  computationObject->Image = this->Image;
  computationObject->Result = this->Result;
  ComputationThread->SetObject(computationObject);
  ComputationThread->start();
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

void PoissonEditingGUI::OpenImageAndMask(const std::string& imageFileName, const std::string& maskFileName)
{
  // Load and display image
  typedef itk::ImageFileReader<ImageType> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(imageFileName);
  imageReader->Update();

  Helpers::DeepCopyVectorImage<ImageType>(imageReader->GetOutput(), this->Image);

  QImage qimageImage = HelpersQt::GetQImageRGBA<ImageType>(this->Image);
  this->ImagePixmapItem = this->Scene->addPixmap(QPixmap::fromImage(qimageImage));
  this->graphicsView->fitInView(this->ImagePixmapItem);
  this->ImagePixmapItem->setVisible(this->chkShowInput->isChecked());

  // Load and display mask
  typedef itk::ImageFileReader<Mask> MaskReaderType;
  MaskReaderType::Pointer maskReader = MaskReaderType::New();
  maskReader->SetFileName(maskFileName);
  maskReader->Update();

  Helpers::DeepCopy<Mask>(maskReader->GetOutput(), this->MaskImage);

  QImage qimageMask = HelpersQt::GetQMaskImage(this->MaskImage);
  this->MaskImagePixmapItem = this->Scene->addPixmap(QPixmap::fromImage(qimageMask));
  this->MaskImagePixmapItem->setVisible(this->chkShowMask->isChecked());
}

void PoissonEditingGUI::on_actionOpenImage_activated()
{
  std::cout << "on_actionOpenImage_activated" << std::endl;
  std::vector<std::string> namedImages;
  namedImages.push_back("Image");
  namedImages.push_back("Mask");
  
  ImageFileSelector* fileSelector(new ImageFileSelector(namedImages));
  fileSelector->exec();

  int result = fileSelector->result();
  if(result) // The user clicked 'ok'
    {
    OpenImageAndMask(fileSelector->GetNamedImageFileName("Image"), fileSelector->GetNamedImageFileName("Mask"));
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

void PoissonEditingGUI::slot_StartProgressBar()
{
  this->progressBar->show();
}

void PoissonEditingGUI::slot_StopProgressBar()
{
  this->progressBar->hide();
}

void PoissonEditingGUI::slot_IterationComplete()
{
  QImage qimage = HelpersQt::GetQImageRGBA<ImageType>(this->Result);
  this->ResultPixmapItem = this->Scene->addPixmap(QPixmap::fromImage(qimage));
  this->ResultPixmapItem->setVisible(this->chkShowOutput->isChecked());
}
