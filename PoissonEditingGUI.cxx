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
#include "Mask.h"
#include "PoissonEditing.h"

// ITK
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

// Qt
#include <QIcon>
#include <QFileDialog>

// Default constructor
PoissonEditingGUI::PoissonEditingGUI()
{
  this->setupUi(this);
  
  this->Image = ImageType::New();
  this->MaskImage = Mask::New();
  this->Result = ImageType::New();
};

void PoissonEditingGUI::on_btnFill_clicked()
{
  FillAllChannels<ImageType>(this->Image, this->MaskImage, this->Result);
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

  this->statusBar()->showMessage("Saved result.");
}


void PoissonEditingGUI::on_actionOpenImage_activated()
{
  FileSelector* fileSelector(new FileSelector);
  fileSelector->exec();

  int result = fileSelector->result();
  if(result) // The user clicked 'ok'
    {
    typedef itk::ImageFileReader<ImageType> ImageReaderType;
    ImageReaderType::Pointer imageReader = ImageReaderType::New();
    imageReader->SetFileName(fileSelector->GetImageFileName());
    imageReader->Update();

    Helpers::DeepCopy<ImageType>(imageReader->GetOutput(), this->Image);
 
    typedef itk::ImageFileReader<Mask> MaskReaderType;
    MaskReaderType::Pointer maskReader = MaskReaderType::New();
    maskReader->SetFileName(fileSelector->GetMaskFileName());
    maskReader->Update();
    
    Helpers::DeepCopy<Mask>(maskReader->GetOutput(), this->MaskImage);    
    }
  else
    {
    std::cout << "User clicked cancel." << std::endl;
    // The user clicked 'cancel' or closed the dialog, do nothing.
    }
}
