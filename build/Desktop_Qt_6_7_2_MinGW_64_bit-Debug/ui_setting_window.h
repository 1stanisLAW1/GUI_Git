/********************************************************************************
** Form generated from reading UI file 'setting_window.ui'
**
** Created by: Qt User Interface Compiler version 6.7.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SETTING_WINDOW_H
#define UI_SETTING_WINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_setting_window
{
public:
    QWidget *centralwidget;

    void setupUi(QMainWindow *setting_window)
    {
        if (setting_window->objectName().isEmpty())
            setting_window->setObjectName("setting_window");
        setting_window->resize(800, 600);
        centralwidget = new QWidget(setting_window);
        centralwidget->setObjectName("centralwidget");
        setting_window->setCentralWidget(centralwidget);

        retranslateUi(setting_window);

        QMetaObject::connectSlotsByName(setting_window);
    } // setupUi

    void retranslateUi(QMainWindow *setting_window)
    {
        setting_window->setWindowTitle(QCoreApplication::translate("setting_window", "Settings", nullptr));
    } // retranslateUi

};

namespace Ui {
    class setting_window: public Ui_setting_window {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SETTING_WINDOW_H
