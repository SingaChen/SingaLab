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
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ChartsViewer
{
public:
    QDockWidget *dockWidget;
    QWidget *dockWidgetContents;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QComboBox *comboBox_polygenMesh;
    QSpacerItem *verticalSpacer_2;
    QComboBox *comboBox_qmeshPatch;
    QHBoxLayout *horizontalLayout;
    QSpinBox *spinBox_rangeFrom;
    QSpinBox *spinBox_rangeTo;
    QCheckBox *checkBox_patchRange;
    QSpacerItem *verticalSpacer;
    QComboBox *comboBox_workStep;
    QSpacerItem *verticalSpacer_3;
    QComboBox *comboBox_parameter;
    QSpacerItem *verticalSpacer_4;
    QPushButton *pushButton_chartActive;
    QWidget *widget;
    QLabel *m_valueLabel;

    void setupUi(QWidget *ChartsViewer)
    {
        if (ChartsViewer->objectName().isEmpty())
            ChartsViewer->setObjectName(QString::fromUtf8("ChartsViewer"));
        ChartsViewer->resize(1235, 884);
        dockWidget = new QDockWidget(ChartsViewer);
        dockWidget->setObjectName(QString::fromUtf8("dockWidget"));
        dockWidget->setGeometry(QRect(1020, 0, 211, 881));
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QString::fromUtf8("dockWidgetContents"));
        verticalLayoutWidget = new QWidget(dockWidgetContents);
        verticalLayoutWidget->setObjectName(QString::fromUtf8("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(0, 0, 211, 411));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        comboBox_polygenMesh = new QComboBox(verticalLayoutWidget);
        comboBox_polygenMesh->setObjectName(QString::fromUtf8("comboBox_polygenMesh"));

        verticalLayout->addWidget(comboBox_polygenMesh);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer_2);

        comboBox_qmeshPatch = new QComboBox(verticalLayoutWidget);
        comboBox_qmeshPatch->setObjectName(QString::fromUtf8("comboBox_qmeshPatch"));

        verticalLayout->addWidget(comboBox_qmeshPatch);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        spinBox_rangeFrom = new QSpinBox(verticalLayoutWidget);
        spinBox_rangeFrom->setObjectName(QString::fromUtf8("spinBox_rangeFrom"));

        horizontalLayout->addWidget(spinBox_rangeFrom);

        spinBox_rangeTo = new QSpinBox(verticalLayoutWidget);
        spinBox_rangeTo->setObjectName(QString::fromUtf8("spinBox_rangeTo"));

        horizontalLayout->addWidget(spinBox_rangeTo);

        checkBox_patchRange = new QCheckBox(verticalLayoutWidget);
        checkBox_patchRange->setObjectName(QString::fromUtf8("checkBox_patchRange"));

        horizontalLayout->addWidget(checkBox_patchRange);


        verticalLayout->addLayout(horizontalLayout);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        comboBox_workStep = new QComboBox(verticalLayoutWidget);
        comboBox_workStep->setObjectName(QString::fromUtf8("comboBox_workStep"));

        verticalLayout->addWidget(comboBox_workStep);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer_3);

        comboBox_parameter = new QComboBox(verticalLayoutWidget);
        comboBox_parameter->setObjectName(QString::fromUtf8("comboBox_parameter"));

        verticalLayout->addWidget(comboBox_parameter);

        verticalSpacer_4 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer_4);

        pushButton_chartActive = new QPushButton(verticalLayoutWidget);
        pushButton_chartActive->setObjectName(QString::fromUtf8("pushButton_chartActive"));

        verticalLayout->addWidget(pushButton_chartActive);

        dockWidget->setWidget(dockWidgetContents);
        widget = new QWidget(ChartsViewer);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setGeometry(QRect(50, 10, 971, 851));
        m_valueLabel = new QLabel(ChartsViewer);
        m_valueLabel->setObjectName(QString::fromUtf8("m_valueLabel"));
        m_valueLabel->setGeometry(QRect(970, 650, 141, 16));
        widget->raise();
        dockWidget->raise();
        m_valueLabel->raise();

        retranslateUi(ChartsViewer);

        QMetaObject::connectSlotsByName(ChartsViewer);
    } // setupUi

    void retranslateUi(QWidget *ChartsViewer)
    {
        ChartsViewer->setWindowTitle(QApplication::translate("ChartsViewer", "ChartWindow", nullptr));
        checkBox_patchRange->setText(QApplication::translate("ChartsViewer", "RangeActive", nullptr));
        pushButton_chartActive->setText(QApplication::translate("ChartsViewer", "Active", nullptr));
        m_valueLabel->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class ChartsViewer: public Ui_ChartsViewer {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CHARTSVIEWER_H
