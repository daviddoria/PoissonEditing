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
  
  this->Result = ImageType::New();
};

void PoissonEditingGUI::on_btnFill_clicked()
{
  
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

  HelpersOutput::WriteImage<FloatVectorImageType>(this->Result, fileName.toStdString());

  this->statusBar()->showMessage("Saved result.");
}


void PoissonEditingGUI::on_actionOpenImage_activated()
{
  FileSelector* fileSelector(new FileSelector);
  fileSelector->exec();

  int result = fileSelector->result();
  if(result) // The user clicked 'ok'
    {
    typedef itk::ImageFileReader<FloatVectorImageType> ImageReaderType;
    ImageReaderType::Pointer imageReader = ImageReaderType::New();
    imageReader->SetFileName(fileSelector->GetImageFileName());
    imageReader->Update();

    typedef itk::ImageFileReader<UnsignedCharScalarImageType> MaskReaderType;
    MaskReaderType::Pointer maskReader = MaskReaderType::New();
    maskReader->SetFileName(fileSelector->GetMaskFileName());
    maskReader->Update();
    
    FillAllChannels<FloatVectorImageType>(imageReader->GetOutput(), maskReader->GetOutput(), this->Result);
    
    }
  else
    {
    std::cout << "User clicked cancel." << std::endl;
    // The user clicked 'cancel' or closed the dialog, do nothing.
    }
}
