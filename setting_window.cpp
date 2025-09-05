#include "setting_window.h"
#include "qboxlayout.h"
#include "qpushbutton.h"
#include "ui_setting_window.h"

setting_window::setting_window(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::setting_window)
{
    ui->setupUi(this);
    QVBoxLayout* main_layout = new QVBoxLayout();
    background_btn = new QPushButton("light");
    main_layout->addWidget(background_btn);
    ui->centralwidget->setLayout(main_layout);
    connect(background_btn,&QPushButton::clicked,this,&setting_window::theme);

}

setting_window::~setting_window()
{
    delete ui;
}

void setting_window::theme()
{
    theme_dark =!theme_dark;
    if(theme_dark){
        background_btn->setText("dark");
    }else{
        background_btn->setText("light");
    }
}
