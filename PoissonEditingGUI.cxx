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

// ITK
#include "itkCastImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkMaskImageFilter.h"
#include "itkRegionOfInterestImageFilter.h"
#include "itkVector.h"

// Qt
#include <QFileDialog>
#include <QIcon>
#include <QTextEdit>


// Default constructor
PoissonEditingGUI::PoissonEditingGUI()
{
  this->setupUi(this);
};

void PoissonEditingGUI::on_btnFill_clicked()
{
  
}

void PoissonEditingGUI::on_actionSaveResult_activated()
{
//   // Get a filename to save
//   QString fileName = QFileDialog::getSaveFileName(this, "Save File", ".", "Image Files (*.jpg *.jpeg *.bmp *.png *.mha)");
// 
//   DebugMessage<std::string>("Got filename: ", fileName.toStdString());
//   if(fileName.toStdString().empty())
//     {
//     std::cout << "Filename was empty." << std::endl;
//     return;
//     }
// 
//   HelpersOutput::WriteImage<FloatVectorImageType>(this->Inpainting.GetCurrentOutputImage(), fileName.toStdString());
// 
//   this->statusBar()->showMessage("Saved result.");
}


void PoissonEditingGUI::on_actionOpenImage_activated()
{
  FileSelector* fileSelector(new FileSelector);
  fileSelector->exec();

  int result = fileSelector->result();
  if(result) // The user clicked 'ok'
    {
    std::cout << "User clicked ok." << std::endl;
    //fileSelector->GetImageFileName());
    //fileSelector->GetMaskFileName(), fileSelector->IsMaskInverted());
    }
  else
    {
    std::cout << "User clicked cancel." << std::endl;
    // The user clicked 'cancel' or closed the dialog, do nothing.
    }
}

// void Form::OpenImage(const std::string& fileName)
// {
//   //std::cout << "OpenImage()" << std::endl;
//   /*
//   // The non static version of the above is something like this:
//   QFileDialog myDialog;
//   QDir fileFilter("Image Files (*.jpg *.jpeg *.bmp *.png *.mha);;PNG Files (*.png)");
//   myDialog.setFilter(fileFilter);
//   QString fileName = myDialog.exec();
//   */
// 
//   typedef itk::ImageFileReader<FloatVectorImageType> ReaderType;
//   ReaderType::Pointer reader = ReaderType::New();
//   reader->SetFileName(fileName);
//   reader->Update();
// 
//   //this->Image = reader->GetOutput();
//   
//   Helpers::DeepCopy<FloatVectorImageType>(reader->GetOutput(), this->UserImage);
//   
//   //std::cout << "UserImage region: " << this->UserImage->GetLargestPossibleRegion() << std::endl;
// 
//   Helpers::ITKVectorImagetoVTKImage(this->UserImage, this->ImageLayer.ImageData);
// 
//   this->Renderer->ResetCamera();
//   this->qvtkWidget->GetRenderWindow()->Render();
// 
//   this->statusBar()->showMessage("Opened image.");
//   actionOpenMask->setEnabled(true);
// 
//   this->AllForwardLookOutlinesLayer.ImageData->SetDimensions(this->UserImage->GetLargestPossibleRegion().GetSize()[0],
//                                                              this->UserImage->GetLargestPossibleRegion().GetSize()[1], 1);
//   this->AllForwardLookOutlinesLayer.ImageData->AllocateScalars();
//   this->AllSourcePatchOutlinesLayer.ImageData->SetDimensions(this->UserImage->GetLargestPossibleRegion().GetSize()[0],
//                                                              this->UserImage->GetLargestPossibleRegion().GetSize()[1], 1);
//   this->AllSourcePatchOutlinesLayer.ImageData->AllocateScalars();
// 
// }

