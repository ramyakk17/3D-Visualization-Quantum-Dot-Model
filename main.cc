//
//  Qt OpenGL Lorenz Viewer
//

#include <QApplication>
#include "viewer.h"
#include <QtCore/QCoreApplication>

using namespace std;

//
//  Main function
//
int main(int argc, char *argv[])
{
   //  Create the application
   QApplication app(argc,argv);

   //  Create and show Viewer widget
   Viewer viewer;
   viewer.show();
   //  Main loop for application
   return app.exec();
}



