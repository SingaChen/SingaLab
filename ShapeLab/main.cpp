#include "MainWindow.h"
#include <QApplication>



int main(int argc, char *argv[])
{
    /*QChart* m_chart = new QChart();
    cout << "hello world" << endl;*/
    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    return a.exec();
}
