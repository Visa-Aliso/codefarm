#include "MainWindow.h"
#include <QApplication>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("Code Farm");
    QApplication::setOrganizationName("CodeFarmQt");
    MainWindow window;
    window.show();
    return app.exec();
}
