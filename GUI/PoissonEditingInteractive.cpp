/*
Copyright (C) 2011 David Doria, daviddoria@gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QApplication>

#include "PoissonEditingGUI.h"

int main( int argc, char** argv )
{
  QApplication app( argc, argv );

  PoissonEditingGUI* poissonEditingGUI;
  if(argc == 3)
    {
    std::cout << "Using filename arguments." << std::endl;
    poissonEditingGUI = new PoissonEditingGUI(argv[1], argv[2]);
    }
  else
    {
    //std::cout << "Not using filename arguments." << std::endl;
    poissonEditingGUI = new PoissonEditingGUI;
    }
  //myForm.show();
  poissonEditingGUI->showMaximized();

  return app.exec();
}
