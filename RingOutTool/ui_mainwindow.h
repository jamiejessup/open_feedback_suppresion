/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Sun Jan 19 15:05:24 2014
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>
#include <QtGui/QMainWindow>
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "qcustomplot.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout;
    QCustomPlot *responsePlot;
    QGroupBox *statusBox;
    QHBoxLayout *horizontalLayout;
    QLabel *PeakStatusLabel;
    QPushButton *addPeakButton;
    QGroupBox *filterFileGroup;
    QVBoxLayout *verticalLayout_3;
    QListWidget *filterList;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *removeButton;
    QPushButton *openButton;
    QPushButton *saveButton;
    QPushButton *saveAsButton;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1024, 1024);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(centralWidget->sizePolicy().hasHeightForWidth());
        centralWidget->setSizePolicy(sizePolicy);
        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        responsePlot = new QCustomPlot(centralWidget);
        responsePlot->setObjectName(QString::fromUtf8("responsePlot"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(responsePlot->sizePolicy().hasHeightForWidth());
        responsePlot->setSizePolicy(sizePolicy1);

        verticalLayout->addWidget(responsePlot);

        statusBox = new QGroupBox(centralWidget);
        statusBox->setObjectName(QString::fromUtf8("statusBox"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Maximum);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(statusBox->sizePolicy().hasHeightForWidth());
        statusBox->setSizePolicy(sizePolicy2);
        horizontalLayout = new QHBoxLayout(statusBox);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        PeakStatusLabel = new QLabel(statusBox);
        PeakStatusLabel->setObjectName(QString::fromUtf8("PeakStatusLabel"));
        QSizePolicy sizePolicy3(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(PeakStatusLabel->sizePolicy().hasHeightForWidth());
        PeakStatusLabel->setSizePolicy(sizePolicy3);

        horizontalLayout->addWidget(PeakStatusLabel);

        addPeakButton = new QPushButton(statusBox);
        addPeakButton->setObjectName(QString::fromUtf8("addPeakButton"));
        QSizePolicy sizePolicy4(QSizePolicy::Maximum, QSizePolicy::Fixed);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(addPeakButton->sizePolicy().hasHeightForWidth());
        addPeakButton->setSizePolicy(sizePolicy4);

        horizontalLayout->addWidget(addPeakButton);


        verticalLayout->addWidget(statusBox);

        filterFileGroup = new QGroupBox(centralWidget);
        filterFileGroup->setObjectName(QString::fromUtf8("filterFileGroup"));
        sizePolicy2.setHeightForWidth(filterFileGroup->sizePolicy().hasHeightForWidth());
        filterFileGroup->setSizePolicy(sizePolicy2);
        verticalLayout_3 = new QVBoxLayout(filterFileGroup);
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        filterList = new QListWidget(filterFileGroup);
        filterList->setObjectName(QString::fromUtf8("filterList"));
        filterList->setSelectionMode(QAbstractItemView::MultiSelection);

        verticalLayout_3->addWidget(filterList);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        removeButton = new QPushButton(filterFileGroup);
        removeButton->setObjectName(QString::fromUtf8("removeButton"));

        horizontalLayout_3->addWidget(removeButton);

        openButton = new QPushButton(filterFileGroup);
        openButton->setObjectName(QString::fromUtf8("openButton"));

        horizontalLayout_3->addWidget(openButton);

        saveButton = new QPushButton(filterFileGroup);
        saveButton->setObjectName(QString::fromUtf8("saveButton"));
        saveButton->setEnabled(false);

        horizontalLayout_3->addWidget(saveButton);

        saveAsButton = new QPushButton(filterFileGroup);
        saveAsButton->setObjectName(QString::fromUtf8("saveAsButton"));
        saveAsButton->setEnabled(false);

        horizontalLayout_3->addWidget(saveAsButton);


        verticalLayout_3->addLayout(horizontalLayout_3);


        verticalLayout->addWidget(filterFileGroup);

        MainWindow->setCentralWidget(centralWidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "OFS Ringout Tool", 0, QApplication::UnicodeUTF8));
        statusBox->setTitle(QApplication::translate("MainWindow", "Peak Detection", 0, QApplication::UnicodeUTF8));
        PeakStatusLabel->setText(QApplication::translate("MainWindow", "Peak Detected At: ", 0, QApplication::UnicodeUTF8));
        addPeakButton->setText(QApplication::translate("MainWindow", "Add Frequency to Filter List", 0, QApplication::UnicodeUTF8));
        filterFileGroup->setTitle(QApplication::translate("MainWindow", "Filter Specification", 0, QApplication::UnicodeUTF8));
        removeButton->setText(QApplication::translate("MainWindow", "Remove Filter", 0, QApplication::UnicodeUTF8));
        openButton->setText(QApplication::translate("MainWindow", "Open Existing Filter List", 0, QApplication::UnicodeUTF8));
        saveButton->setText(QApplication::translate("MainWindow", "Save Filter List", 0, QApplication::UnicodeUTF8));
        saveAsButton->setText(QApplication::translate("MainWindow", "Save Filter List As ...", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
