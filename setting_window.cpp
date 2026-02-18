#include "setting_window.h"
#include "qboxlayout.h"
#include "qlabel.h"
#include "qpushbutton.h"
#include "ui_setting_window.h"
#include "work_file.h"

#include <QFile>
#include <QMessageBox>

setting_window::setting_window(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::setting_window)
{
    ui->setupUi(this);
    ui->centralwidget->setMinimumSize(300,150);
    QVBoxLayout* main_layout = new QVBoxLayout();
    background_btn = new QPushButton();
    save_btn = new QPushButton("save");
    QWidget *language_widget = lang_widget();
    main_layout->addWidget(background_btn);
    main_layout->addWidget(language_widget);
    main_layout->addWidget(save_btn);
    ui->centralwidget->setLayout(main_layout);

   connect(background_btn,&QPushButton::clicked,this,&setting_window::theme);

    connect(save_btn,&QPushButton::clicked,this,[=](){
        setting_value.clear();
        setting_value.append(background_btn->text());
        setting_value.append(languege_box->currentText());
        work_file *wf = new work_file();
        wf->save_config(setting_value);
        QString str = languege_box->currentText();
        emit signal_save(setting_value);
        languege_box->setCurrentText(str);
        update_combo_box();
        update_lang();
    });
}

setting_window::~setting_window()
{
    delete ui;
}

void setting_window::set_style(bool theme_dark, bool lang_engl_)
{
    them_dark = theme_dark;
    lang_engl = lang_engl_;
    updateBackgroundButtonText();
}

void setting_window::theme()
{
    them_dark = !them_dark;
    updateBackgroundButtonText();
}

void setting_window::updateBackgroundButtonText()
{
    if (lang_engl) {
        background_btn->setText(them_dark ? "dark" : "light");
        languege_box->setCurrentText("English");
        save_btn->setText("save");
        lbl->setText("language: ");
    } else {
        background_btn->setText(them_dark ? "тёмная" : "светлая");
        languege_box->setCurrentText("Russian");
        save_btn->setText("сохранить");
        lbl->setText("язык: ");
    }
    style();
}
void setting_window::style()
{
    if(them_dark){
        save_btn->setStyleSheet("QPushButton { background-color: black; color: white;  font-size: 16px; border: 2px solid darkgray;"
                    "border-radius:15px;"
                    "max-height:30px;"
                    "max-width:150px;"
                    "min-height:30px;"
                                "min-width:100px;}");

        background_btn->setStyleSheet("QPushButton { background-color: black; color: white;  font-size: 16px; border: 2px solid darkgray;"
                                "border-radius:15px;"
                                "max-height:30px;"
                                "max-width:150px;"
                                "min-height:30px;"
                                "min-width:100px;}");

        lbl->setStyleSheet("QPushButton { background-color: black; color: white;");

        languege_box->setStyleSheet("QComboBox {background-color: black;color: white;border: 1px solid white ;border-radius: 5px;padding: 5px;}"
"QComboBox:hover {border: 1px solid white;}"
"QComboBox QAbstractItemView {background-color: black; selection-background-color:white;selection-color:white;color: white;}"
                                    "QComboBox::drop-down {width: 20px; }");

        ui->centralwidget->setStyleSheet("QWidget { background-color: black; color: white; }");
    }else{
        save_btn->setStyleSheet("QPushButton { background-color: #d4cdcd; color: black; font-size: 16px; border: 2px solid darkgray;"
                                "border-radius:15px;"
                                "max-height:30px;"
                                "max-width:150px;"
                                "min-height:30px;"
                                "min-width:100px;}");

        background_btn->setStyleSheet("QPushButton { background-color: #d4cdcd; color: black; font-size: 16px; border: 2px solid darkgray;"
                                      "border-radius:15px;"
                                      "max-height:30px;"
                                      "max-width:150px;"
                                      "min-height:30px;"
                                      "min-width:100px;}");

        lbl->setStyleSheet("QPushButton { background-color: #d4cdcd; color: black;");

        languege_box->setStyleSheet("QComboBox {background-color: rgb(220, 220, 220);color: black;border: 1px solid black ;border-radius: 5px;padding: 5px;}"
         "QComboBox:hover {border: 1px solid black;}"
         "QComboBox QAbstractItemView {background-color: rgb(220, 220, 220); selection-background-color:black;selection-color:black;color: black;}"
         "QComboBox::drop-down {width: 20px; }");

        ui->centralwidget->setStyleSheet("QWidget { background-color: #d4cdcd; color: black; }");
    }
}

void setting_window::update_lang()
{
    if (lang_engl) {
        background_btn->setText(them_dark ? "dark" : "light");
        save_btn->setText("save");
        lbl->setText("language: ");
    } else {
        background_btn->setText(them_dark ? "тёмная" : "светлая");
        save_btn->setText("сохранить");
        lbl->setText("язык: ");
    }
}

void setting_window::update_combo_box()
{
    if(languege_box->currentText() == "English"){
        lang_engl = true;
    }else{
        lang_engl = false;
    }
}


QWidget *setting_window::lang_widget()
{

    QWidget* central_widget = new QWidget();
    QHBoxLayout *central_layout = new QHBoxLayout();
    lbl = new QLabel();
    languege_box = new QComboBox();
    languege_box->addItem("English");
    languege_box->addItem("Russian");

    if(lang_engl){
        lbl->setText("language: ");
    }else{
        lbl->setText("язык: ");
    }

    central_layout->addWidget(lbl);
    central_layout->addWidget(languege_box);
    central_widget->setLayout(central_layout);
    return central_widget;
}
