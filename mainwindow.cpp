#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "work_git.h"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QThread>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QHBoxLayout* layout = new QHBoxLayout();
    QVBoxLayout* central_layout = new QVBoxLayout();
    QPushButton* btn_path = new QPushButton("Choose a path");
    QPushButton* btn_clear = new QPushButton("Clear console");
    QLabel *lbl = new QLabel("Link to GitHab project");
    QLineEdit* dr = new QLineEdit();
    dr->setFixedSize(600,20);

    wg = new work_git();

    QTabWidget* tab_widget = new QTabWidget();
    tab_widget->setFixedSize(600,300);
    tab_widget->addTab(cloning_widget(),"Cloning");
    tab_widget->addTab(push_widget(),"Push");

    QThread* thread = new QThread();
    wg->moveToThread(thread);
    thread->start();

    connect(btn_clone,&QPushButton::clicked,wg,[=,this](){
        list.append(dr->text());
        if(path_cd.isEmpty()){
            list.append("C:/");
        }else{
            list.append(path_cd + "/");
        }
        wg->clone_repo(list);
        list.clear();
    });

    connect(btn_ls,&QPushButton::clicked,wg,[=,this](){
        wg->check_direct(path_cd);
    });

    connect(btn_clear,&QPushButton::clicked,this,[this](){
        cons->clear();
    });

    connect(wg,&work_git::message_signal,this,&MainWindow::set_text_in_console);
    connect(btn_path,&QPushButton::clicked,this,&MainWindow::select_path);
    connect(btn_push,&QPushButton::clicked,wg,[=,this](){

        list.append(dr->text()+".git");
        if(path_cd.isEmpty()){
            list.append("C:/");
        }else{
            list.append(path_cd);
        }

        if(commit_new->text().isEmpty()){
            list.append("commit");
        }else{
            list.append(commit_new->text());
        }

        list.append(token_line->text());
        list.append(branch_line->text());

        wg->check_push(list,0);

        list.clear();
    });

    connect(btn_push_old,&QPushButton::clicked,wg,[=,this](){

        list.append(dr->text()+".git");
        if(path_cd.isEmpty()){
            list.append("C:/");
        }else{
            list.append(path_cd);
        }

        if(commit_old->text().isEmpty()){
            list.append("commit");
        }else{
            list.append(commit_old->text());
        }

        list.append(token_line->text());
        list.append(branch_line->text());

        wg->check_push(list,1);

        list.clear();
    });

    cons = console_widget();

    set_text_in_console("~path");

    layout->addWidget(lbl);
    layout->addWidget(dr);
    layout->addWidget(btn_path);
    layout->addWidget(btn_clear);
    central_layout->addLayout(layout);
    central_layout->addWidget(tab_widget);
    central_layout->addWidget(cons);

    ui->centralwidget->setLayout(central_layout);
}

MainWindow::~MainWindow()
{
    delete ui;
}

QTextEdit *MainWindow::console_widget()
{
    // настройка консоли
    QTextEdit* console = new QTextEdit();
    console->setLineWrapMode(QTextEdit::WidgetWidth);
    console->setAcceptRichText(false);
    console->resize(890,200);
    console->setStyleSheet("QTextEdit { color: black; font-size: 14px;}");
    console->setReadOnly(true);
    return console;
}

void MainWindow::select_path()
{
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::Directory);  // Режим выбора папки
    dialog.setOption(QFileDialog::ShowDirsOnly, true);  // Показывать только папки
    dialog.setViewMode(QFileDialog::Detail);     // Подробный вид
    dialog.exec();
    path_cd = dialog.selectedFiles().first();
    set_text_in_console("~path "+dialog.selectedFiles().first());
}

QWidget *MainWindow::cloning_widget()
{
    QWidget* widget = new QWidget();
    QHBoxLayout* central_layout = new QHBoxLayout();
    btn_clone = new QPushButton("Clone");
    btn_ls = new QPushButton("Сheck directory");

    btn_clone->setFixedSize(100,30);
    btn_ls->setFixedSize(100,30);

    central_layout->addWidget(btn_clone);
    central_layout->addWidget(btn_ls);

    widget->setLayout(central_layout);

    return widget;
}

QWidget *MainWindow::push_widget()
{
    QWidget* widget = new QWidget();
    QVBoxLayout* main_layout = new QVBoxLayout();
    QHBoxLayout* central_layout = new QHBoxLayout();
    QVBoxLayout* for_new_repo_layout = new QVBoxLayout();
    QVBoxLayout* for_old_repo_layout = new QVBoxLayout();
    QVBoxLayout* for_branch_layout = new QVBoxLayout();

    QLabel* lbl = new QLabel("Token:");
    QLabel* lbl_commit = new QLabel("commit:");
    QLabel* lbl_commit_2 = new QLabel("commit:");
    QLabel* lbl_branch = new QLabel("branch:");
    token_line = new QLineEdit();
    btn_push = new QPushButton("Push new repo");
    btn_push_old = new QPushButton("Push repo");
    commit_new = new QLineEdit("First commit");
    commit_old = new QLineEdit("Next commit");
    branch_line = new QLineEdit("master");
    btn_push->setFixedSize(100,30);
    btn_push_old->setFixedSize(100,30);

    for_new_repo_layout->addWidget(btn_push);
    for_new_repo_layout->addWidget(lbl_commit);
    for_new_repo_layout->addWidget(commit_new);
    for_old_repo_layout->addWidget(btn_push_old);
    for_old_repo_layout->addWidget(lbl_commit_2);
    for_old_repo_layout->addWidget(commit_old);

    central_layout->addLayout(for_new_repo_layout);
    central_layout->addLayout(for_old_repo_layout);

    for_branch_layout->addWidget(lbl_branch);
    for_branch_layout->addWidget(branch_line);

    main_layout->addWidget(lbl);
    main_layout->addWidget(token_line);
    main_layout->addLayout(for_branch_layout);
    main_layout->addLayout(central_layout);

    widget->setLayout(main_layout);

    return widget;
}


void MainWindow::set_text_in_console(QString message)
{
    QString text = cons->toPlainText();
    if(text.isEmpty()){
        cons->setText(message);
    }else{
        QString new_text = text + "\n" + message;
        cons->setText(new_text);
    }
}
