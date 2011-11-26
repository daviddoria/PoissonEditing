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

// Custom
#include "ImageFileSelector.h"
#include "FileSelectionWidget.h"
#include "Helpers.h"
#include "HelpersQt.h"
#include "Mask.h"

// Qt
#include <QPushButton>
#include <QDialogButtonBox>
#include <QFileSystemModel>
#include <QHBoxLayout>
#include <QWidget>

// STL
#include <iostream>

// ITK
#include "itkImageFileReader.h"

// Constructor
ImageFileSelector::ImageFileSelector(const std::vector<std::string>& namedImages) : NamedImages(namedImages)
{
  QVBoxLayout* VLayout = new QVBoxLayout(this);
  
  QHBoxLayout* HLayout = new QHBoxLayout(this);

  VLayout->addLayout(HLayout);

  for(unsigned int i = 0; i < namedImages.size(); ++i)
    {
    Panel* panel = new Panel;
    std::string boldString = "<b>" + namedImages[i] + "</b>";
    panel->Label->setText(boldString.c_str());
    HLayout->addLayout(panel->Layout);
    this->Panels.push_back(panel);
    
    connect(this->Panels[i]->SelectionWidget, SIGNAL(selectionChanged()), this, SLOT(Verify()));
    }

  this->ButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
  this->ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
  connect(this->ButtonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(slot_buttonBox_accepted()));
  connect(this->ButtonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(slot_buttonBox_rejected()));
  
  VLayout->addWidget(this->ButtonBox);

  this->setLayout(VLayout);
  this->show();

};

void ImageFileSelector::Verify()
{
  bool allLoaded = true;
  for(unsigned int i = 0; i < this->NamedImages.size(); ++i)
    {
    allLoaded = allLoaded && this->Panels[i]->SelectionWidget->IsValid();
    }

  if(allLoaded)
    {
     this->ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
//     if(this->Image->GetLargestPossibleRegion() == this->ImageMask->GetLargestPossibleRegion())
//       {
//       //this->buttonBox->setEnabled(true);
//       return;
//       }
    }
  //this->buttonBox->setEnabled(false);
}

std::string ImageFileSelector::GetNamedImageFileName(const std::string& namedImage)
{
  for(unsigned int i = 0; i < this->NamedImages.size(); ++i)
   {
    if(namedImage.compare(this->NamedImages[i]) == 0)
     {
     return this->FileNames[i];
     }
   }
  std::cerr << "Warning: You requested a named image that was not selected." << std::endl;
  return std::string("NoFile");
}

void ImageFileSelector::slot_buttonBox_accepted()
{
  // Set the working directory to the first panel's directory
  std::string workingDirectory = this->Panels[0]->SelectionWidget->currentIndex().data(QFileSystemModel::FilePathRole).toString().toStdString() + "/";

  QDir::setCurrent(QString(workingDirectory.c_str()));
  std::cout << "Working directory set to: " << workingDirectory << std::endl;

  this->FileNames.resize(this->Panels.size());
  
  for(unsigned int i = 0; i < this->NamedImages.size(); ++i)
    {
    this->FileNames[i] = this->Panels[i]->SelectionWidget->GetFileName();
    }
   
  this->setResult(QDialog::Accepted);

  this->accept();
}

void ImageFileSelector::slot_buttonBox_rejected()
{
  this->setResult(QDialog::Rejected);
  this->reject();
}
