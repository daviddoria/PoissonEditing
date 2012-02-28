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

#include "PoissonEditingWidget.h"

// Custom
#include "ImageFileSelector.h"
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
#include <QtConcurrentRun>

// Called by all constructors
void PoissonEditingWidget::SharedConstructor()
{
  std::cout << "SharedConstructor()" << std::endl;
  this->setupUi(this);

  this->ProgressDialog = new QProgressDialog();

  connect(&this->FutureWatcher, SIGNAL(finished()), this, SLOT(slot_finished()));
  connect(&this->FutureWatcher, SIGNAL(finished()), this->ProgressDialog , SLOT(cancel()));

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
PoissonEditingWidget::PoissonEditingWidget()
{
  std::cout << "PoissonEditingWidget()" << std::endl;
  SharedConstructor();
};

PoissonEditingWidget::PoissonEditingWidget(const std::string& imageFileName, const std::string& maskFileName)
{
  std::cout << "PoissonEditingWidget(string, string)" << std::endl;
  SharedConstructor();
  this->SourceImageFileName = imageFileName;
  this->MaskImageFileName = maskFileName;
  OpenImageAndMask(this->SourceImageFileName, this->MaskImageFileName);
}

void PoissonEditingWidget::showEvent ( QShowEvent * event )
{
  if(this->ImagePixmapItem)
    {
    this->graphicsView->fitInView(this->ImagePixmapItem, Qt::KeepAspectRatio);
    }
}

void PoissonEditingWidget::resizeEvent ( QResizeEvent * event )
{
  if(this->ImagePixmapItem)
    {
    this->graphicsView->fitInView(this->ImagePixmapItem, Qt::KeepAspectRatio);
    }
}

void PoissonEditingWidget::on_btnFill_clicked()
{
  QFuture<void> future = QtConcurrent::run(FillAllChannels<ImageType>, Image.GetPointer(),
                                           MaskImage.GetPointer(), Result.GetPointer());
  this->FutureWatcher.setFuture(future);

  this->ProgressDialog->setMinimum(0);
  this->ProgressDialog->setMaximum(0);
  this->ProgressDialog->setWindowModality(Qt::WindowModal);
  this->ProgressDialog->exec();
}

void PoissonEditingWidget::on_actionSaveResult_activated()
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

void PoissonEditingWidget::OpenImageAndMask(const std::string& imageFileName, const std::string& maskFileName)
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

void PoissonEditingWidget::on_actionOpenImage_activated()
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

void PoissonEditingWidget::on_chkShowInput_clicked()
{
  if(!this->ImagePixmapItem)
    {
    return;
    }
  this->ImagePixmapItem->setVisible(this->chkShowInput->isChecked());
}

void PoissonEditingWidget::on_chkShowOutput_clicked()
{
  if(!this->ResultPixmapItem)
    {
    return;
    }
  this->ResultPixmapItem->setVisible(this->chkShowOutput->isChecked());
}

void PoissonEditingWidget::on_chkShowMask_clicked()
{
  if(!this->MaskImagePixmapItem)
    {
    return;
    }
  this->MaskImagePixmapItem->setVisible(this->chkShowMask->isChecked());
}

void PoissonEditingWidget::slot_StartProgressBar()
{
  this->progressBar->show();
}

void PoissonEditingWidget::slot_StopProgressBar()
{
  this->progressBar->hide();
}

void PoissonEditingWidget::slot_IterationComplete()
{
  QImage qimage = HelpersQt::GetQImageRGBA<ImageType>(this->Result);
  this->ResultPixmapItem = this->Scene->addPixmap(QPixmap::fromImage(qimage));
  this->ResultPixmapItem->setVisible(this->chkShowOutput->isChecked());
}
