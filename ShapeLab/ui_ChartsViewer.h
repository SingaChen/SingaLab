/********************************************************************************
** Form generated from reading UI file 'ChartsViewer.ui'
**
** Created by: Qt User Interface Compiler version 5.12.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CHARTSVIEWER_H
#define UI_CHARTSVIEWER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ChartsViewer
{
public:

    void setupUi(QWidget *ChartsViewer)
    {
        if (ChartsViewer->objectName().isEmpty())
            ChartsViewer->setObjectName(QString::fromUtf8("ChartsViewer"));
        ChartsViewer->resize(1235, 884);

        retranslateUi(ChartsViewer);

        QMetaObject::connectSlotsByName(ChartsViewer);
    } // setupUi

    void retranslateUi(QWidget *ChartsViewer)
    {
        ChartsViewer->setWindowTitle(QApplication::translate("ChartsViewer", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ChartsViewer: public Ui_ChartsViewer {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CHARTSVIEWER_H
