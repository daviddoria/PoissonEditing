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

#include <QApplication>

#include "PoissonCloningWidget.h"

int main( int argc, char** argv )
{
  QApplication app( argc, argv );

  std::cout << "PoissonCloningGUI" << std::endl;
  PoissonCloningWidget* poissonCloningWidget = NULL;
  if(argc == 4)
    {
    std::cout << "Using filename arguments." << std::endl;
    std::string sourceImageFileName = argv[1];
    std::string targetImageFileName = argv[2];
    std::string maskImageFileName = argv[3];
    poissonCloningWidget = new PoissonCloningWidget(sourceImageFileName, targetImageFileName, maskImageFileName);
    }
  else if(argc == 1)
    {
    std::cout << "Not using filename arguments." << std::endl;
    poissonCloningWidget = new PoissonCloningWidget;
    }
  else
  {
    throw std::runtime_error("Invalid arguments!");
  }

  poissonCloningWidget->show();
  //poissonCloningWidget->showMaximized();

  return app.exec();
}
