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

#include "PoissonCloningGUI.h"

// Custom
#include "ImageFileSelector.h"
#include "HelpersOutput.h"
#include "HelpersQt.h"
#include "Mask.h"
#include "PoissonCloning.h"

// ITK
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

// Qt
#include <QIcon>
#include <QFileDialog>
#include <QGraphicsPixmapItem>

// Default constructor
PoissonCloningGUI::PoissonCloningGUI()
{
  this->setupUi(this);

  this->SourceImage = ImageType::New();
  this->TargetImage = ImageType::New();
  this->MaskImage = Mask::New();
  this->ResultImage = ImageType::New();

  this->SourceScene = new QGraphicsScene;
  this->graphicsViewSourceImage->setScene(this->SourceScene);

  this->TargetScene = new QGraphicsScene;
  this->graphicsViewTargetImage->setScene(this->TargetScene);

  this->ResultScene = new QGraphicsScene;
  this->graphicsViewResultImage->setScene(this->ResultScene);
  
  this->SourceImagePixmapItem = NULL;
  this->TargetImagePixmapItem = NULL;
  this->MaskImagePixmapItem = NULL;
  this->ResultPixmapItem = NULL;

  this->SelectionImagePixmapItem = NULL;
};

void PoissonCloningGUI::on_btnFill_clicked()
{
  CloneAllChannels<ImageType>(this->SourceImage, this->TargetImage, this->MaskImage, this->ResultImage);

  QImage qimage = HelpersQt::GetQImageRGBA<ImageType>(this->ResultImage);

  QPixmap qpixmap = QPixmap::fromImage(qimage);
  this->ResultPixmapItem = this->ResultScene->addPixmap(qpixmap);
  //this->ResultPixmapItem->setVisible(this->chkShowOutput->isChecked());
}

void PoissonCloningGUI::on_actionSaveResult_activated()
{
  // Get a filename to save
  QString fileName = QFileDialog::getSaveFileName(this, "Save File", ".", "Image Files (*.jpg *.jpeg *.bmp *.png *.mha)");

  if(fileName.toStdString().empty())
    {
    std::cout << "Filename was empty." << std::endl;
    return;
    }

  HelpersOutput::WriteImage<ImageType>(this->ResultImage, fileName.toStdString());
  HelpersOutput::WriteRGBImage<ImageType>(this->ResultImage, fileName.toStdString() + ".png");
  this->statusBar()->showMessage("Saved result.");
}


void PoissonCloningGUI::on_actionOpenImage_activated()
{
  std::vector<std::string> namedImages;
  namedImages.push_back("SourceImage");
  namedImages.push_back("TargetImage");
  namedImages.push_back("MaskImage");
  
  ImageFileSelector* fileSelector(new ImageFileSelector(namedImages));
  fileSelector->exec();

  int result = fileSelector->result();
  if(result) // The user clicked 'ok'
    {
    // Load and display source image
    typedef itk::ImageFileReader<ImageType> ImageReaderType;
    ImageReaderType::Pointer sourceImageReader = ImageReaderType::New();
    sourceImageReader->SetFileName(fileSelector->GetNamedImageFileName("SourceImage"));
    sourceImageReader->Update();

    Helpers::DeepCopyVectorImage<ImageType>(sourceImageReader->GetOutput(), this->SourceImage);

    QImage qimageSourceImage = HelpersQt::GetQImageRGBA<ImageType>(this->SourceImage);

    QPixmap qpixmapSourceImage = QPixmap::fromImage(qimageSourceImage);

    this->SourceImagePixmapItem = this->SourceScene->addPixmap(qpixmapSourceImage);

    // Load and display target image
    ImageReaderType::Pointer targetImageReader = ImageReaderType::New();
    targetImageReader->SetFileName(fileSelector->GetNamedImageFileName("TargetImage"));
    targetImageReader->Update();

    Helpers::DeepCopyVectorImage<ImageType>(targetImageReader->GetOutput(), this->TargetImage);

    QImage qimageTargetImage = HelpersQt::GetQImageRGBA<ImageType>(this->TargetImage);

    QPixmap qpixmapTargetImage = QPixmap::fromImage(qimageTargetImage);

    this->SourceImagePixmapItem = this->TargetScene->addPixmap(qpixmapTargetImage);

    // Load and display mask
    typedef itk::ImageFileReader<Mask> MaskReaderType;
    MaskReaderType::Pointer maskReader = MaskReaderType::New();
    maskReader->SetFileName(fileSelector->GetNamedImageFileName("MaskImage"));
    maskReader->Update();

    Helpers::DeepCopy<Mask>(maskReader->GetOutput(), this->MaskImage);

    QImage qimageMask = HelpersQt::GetQMaskImage(this->MaskImage);

    QPixmap qpixmapMask = QPixmap::fromImage(qimageMask);

    this->MaskImagePixmapItem = this->SourceScene->addPixmap(qpixmapMask);
    this->MaskImagePixmapItem->setVisible(this->chkShowMask->isChecked());

    // Setup selection region
    QColor semiTransparentRed(255,0,0, 127);

    this->SelectionImage = QImage(sourceImageReader->GetOutput()->GetLargestPossibleRegion().GetSize()[0],
                                  sourceImageReader->GetOutput()->GetLargestPossibleRegion().GetSize()[1], QImage::Format_ARGB32);
    this->SelectionImage.fill(semiTransparentRed.rgba());

    this->SelectionImagePixmapItem = this->TargetScene->addPixmap(QPixmap::fromImage(this->SelectionImage));
    //this->SelectionImagePixmapItem->setFlag(QGraphicsItem::ItemIsMovable, true);
    this->SelectionImagePixmapItem->setFlag(QGraphicsItem::ItemIsMovable);
    
    }
  else
    {
    // std::cout << "User clicked cancel." << std::endl;
    // The user clicked 'cancel' or closed the dialog, do nothing.
    }
}

void PoissonCloningGUI::on_chkShowMask_clicked()
{
  if(!this->MaskImagePixmapItem)
    {
    return;
    }
  this->MaskImagePixmapItem->setVisible(this->chkShowMask->isChecked());
}
  