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
#include "itkPasteImageFilter.h"

// Qt
#include <QIcon>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QTimer>

// Default constructor
PoissonCloningGUI::PoissonCloningGUI()
{
  DefaultConstructor();
};

PoissonCloningGUI::PoissonCloningGUI(const std::string& sourceImageFileName, const std::string& targetImageFileName, const std::string& maskFileName)
{
  DefaultConstructor();
  this->SourceImageFileName = sourceImageFileName;
  this->TargetImageFileName = targetImageFileName;
  this->MaskImageFileName = maskFileName;

  OpenImages(this->SourceImageFileName, this->TargetImageFileName, this->MaskImageFileName);
}

void PoissonCloningGUI::DefaultConstructor()
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
}

void PoissonCloningGUI::showEvent ( QShowEvent * event )
{
  this->graphicsViewSourceImage->fitInView(this->SourceImagePixmapItem, Qt::KeepAspectRatio);
  this->graphicsViewTargetImage->fitInView(this->TargetImagePixmapItem, Qt::KeepAspectRatio);
}

void PoissonCloningGUI::resizeEvent ( QResizeEvent * event )
{
  this->graphicsViewSourceImage->fitInView(this->SourceImagePixmapItem, Qt::KeepAspectRatio);
  this->graphicsViewTargetImage->fitInView(this->TargetImagePixmapItem, Qt::KeepAspectRatio);
}
  
void PoissonCloningGUI::OpenImages(const std::string& sourceImageFileName, const std::string& targetImageFileName, const std::string& maskFileName)
{
  // Load and display source image
  typedef itk::ImageFileReader<ImageType> ImageReaderType;
  ImageReaderType::Pointer sourceImageReader = ImageReaderType::New();
  sourceImageReader->SetFileName(sourceImageFileName);
  sourceImageReader->Update();

  Helpers::DeepCopyVectorImage<ImageType>(sourceImageReader->GetOutput(), this->SourceImage);

  QImage qimageSourceImage = HelpersQt::GetQImageRGBA<ImageType>(this->SourceImage);
  this->SourceImagePixmapItem = this->SourceScene->addPixmap(QPixmap::fromImage(qimageSourceImage));
    
  // Load and display target image
  ImageReaderType::Pointer targetImageReader = ImageReaderType::New();
  targetImageReader->SetFileName(targetImageFileName);
  targetImageReader->Update();

  Helpers::DeepCopyVectorImage<ImageType>(targetImageReader->GetOutput(), this->TargetImage);

  QImage qimageTargetImage = HelpersQt::GetQImageRGBA<ImageType>(this->TargetImage);
  this->TargetImagePixmapItem = this->TargetScene->addPixmap(QPixmap::fromImage(qimageTargetImage));
  this->TargetScene->setSceneRect(qimageTargetImage.rect());

  // Load and display mask
  typedef itk::ImageFileReader<Mask> MaskReaderType;
  MaskReaderType::Pointer maskReader = MaskReaderType::New();
  maskReader->SetFileName(maskFileName);
  maskReader->Update();

  Helpers::DeepCopy<Mask>(maskReader->GetOutput(), this->MaskImage);

  QImage qimageMask = HelpersQt::GetQMaskImage(this->MaskImage);
  this->MaskImagePixmapItem = this->SourceScene->addPixmap(QPixmap::fromImage(qimageMask));
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

void PoissonCloningGUI::on_btnClone_clicked()
{
  // Extract the portion of the target image the user has selected.
  itk::Index<2> corner;
  corner[0] = this->SelectionImagePixmapItem->pos().x();
  //corner[1] = this->SelectionImagePixmapItem->pos().y();
  corner[1] = this->TargetImage->GetLargestPossibleRegion().GetSize()[1] -
              (this->SelectionImagePixmapItem->pos().y() + this->SelectionImagePixmapItem->boundingRect().height());
//   std::cout << "Height: " << this->SelectionImagePixmapItem->boundingRect().height() << std::endl;
//   std::cout << "y: " << this->SelectionImagePixmapItem->pos().y() << std::endl;
//   std::cout << "Corner: " << corner << std::endl;
  
  itk::Size<2> size = this->SourceImage->GetLargestPossibleRegion().GetSize();

  ImageType::RegionType desiredRegion(corner, size);

  typedef itk::RegionOfInterestImageFilter< ImageType, ImageType > RegionOfInterestImageFilterType;
  RegionOfInterestImageFilterType::Pointer regionOfInterestImageFilter = RegionOfInterestImageFilterType::New();
  regionOfInterestImageFilter->SetRegionOfInterest(desiredRegion);
  regionOfInterestImageFilter->SetInput(this->TargetImage);
  regionOfInterestImageFilter->Update();

  // Perform the cloning
  CloneAllChannels<ImageType>(this->SourceImage, regionOfInterestImageFilter->GetOutput(), this->MaskImage, this->ResultImage);

  // Paste the result back into the appropriate region of the target image
  typedef itk::PasteImageFilter <ImageType, ImageType > PasteImageFilterType;
  PasteImageFilterType::Pointer pasteImageFilter = PasteImageFilterType::New ();
  pasteImageFilter->SetSourceImage(this->ResultImage);
  pasteImageFilter->SetDestinationImage(this->TargetImage);
  pasteImageFilter->SetSourceRegion(this->ResultImage->GetLargestPossibleRegion());
  pasteImageFilter->SetDestinationIndex(corner);
  pasteImageFilter->Update();
  
  // Display the result
  QImage qimage = HelpersQt::GetQImageRGBA<ImageType>(pasteImageFilter->GetOutput());
  qimage = HelpersQt::FitToGraphicsView(qimage, this->graphicsViewResultImage);

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
    OpenImages(fileSelector->GetNamedImageFileName("SourceImage"),
              fileSelector->GetNamedImageFileName("TargetImage"),
              fileSelector->GetNamedImageFileName("MaskImage"));
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
  