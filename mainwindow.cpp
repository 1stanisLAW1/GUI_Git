#include "mainwindow.h"
#include "qtimer.h"
#include "setting_window.h"
#include "ui_mainwindow.h"
#include "work_git.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QGraphicsView>
#include <QGraphicsWidget>
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

    QTimer* timer = new QTimer(this);
    connect(timer,&QTimer::timeout,this,[=](){
        QString count_str =  QString::number(description->toPlainText().size())+" / 350";
        current_count_lbl->setText(count_str);
    });
    timer->start(100);


    text.append(" ");
    text.append("https://github.com/settings/tokens");
    text.append("Click on create token");
    text.append("Select the necessary items and create a token");
    text.append("Copy the token. *My token is not active");

    QHBoxLayout* layout = new QHBoxLayout();
    QVBoxLayout* central_layout = new QVBoxLayout();
    QLabel *lbl = new QLabel("Link to GitHab project");
    QLineEdit* dr = new QLineEdit();
    QWidget* rest_widget =  cloning_widget();
    dr->resize(600,20);

    QAction* setting = new QAction("Setting"); // In development
    QAction* helper = new QAction("How to get token");
    ui->menuSetting->addAction(setting);
    ui->menuSetting->addAction(helper);

    connect(setting,&QAction::triggered,this,[=](){
        setting_window* sw = new setting_window();
        sw->resize(170,100);
        sw->setAttribute(Qt::WA_DeleteOnClose); // delete if window close
        sw->show();
    });

    connect(helper,&QAction::triggered,this,&MainWindow::help_dialog);

    wg = new work_git();

    QTabWidget* tab_widget = new QTabWidget();
    tab_widget->setTabPosition(QTabWidget::West);
    tab_widget->resize(600,300);
    tab_widget->addTab(push_widget(),"Push");
    tab_widget->addTab(create_repo_widget(),"Create");// In development
    tab_widget->addTab(history_widget(),"History"); //In development

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
    connect(btn_delete_repo,&QPushButton::clicked,wg,[=,this](){
        if(token_line->text().isEmpty()){
            set_text_in_console("Error: Fill in the token field");
        }else if(dr->text().isEmpty()){
            set_text_in_console("Error: Fill in the URL field");
        }
        wg->delete_repo(token_line->text(),dr->text());
    });

    cons = console_widget();

    set_text_in_console("~path");

    layout->addWidget(lbl);
    layout->addWidget(dr);
    central_layout->addLayout(layout);
    central_layout->addWidget(rest_widget);
    central_layout->addWidget(tab_widget);
    central_layout->addWidget(cons);

    ui->centralwidget->setLayout(central_layout);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete dialog;
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
    btn_path = new QPushButton("Choose a path");
    btn_clear = new QPushButton("Clear console");
    btn_delete_repo = new QPushButton("Delete repo");

    btn_clone->resize(100,30);
    btn_ls->resize(100,30);
    btn_path->resize(100,30);
    btn_clear->resize(100,30);

    central_layout->addWidget(btn_clone);
    central_layout->addWidget(btn_ls);
    central_layout->addWidget(btn_path);
    central_layout->addWidget(btn_clear);
    central_layout->addWidget(btn_delete_repo);


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
    btn_push->resize(100,30);
    btn_push_old->resize(100,30);

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

void MainWindow::help_dialog()
{
    //dialog window
    QDialog* dialog = new QDialog();
    dialog->resize(500,300);

    QHBoxLayout* btn_layout = new QHBoxLayout();
    QVBoxLayout* layout = new QVBoxLayout();
    QVBoxLayout* central_layout = new QVBoxLayout();

    QPushButton* closeButton = new QPushButton("Close");
    closeButton->resize(40,30);
    QPushButton* next_btn = new QPushButton("->");
    QPushButton* back_btn = new QPushButton("<-");
    back_btn->setEnabled(false);

    QLabel* txt_lbl = new QLabel();
    txt_lbl->setStyleSheet("QLabel { color: black; font-size: 18px;}");

    QTextEdit* txt_edit = new QTextEdit(text.at(count));
    txt_edit->setLineWrapMode(QTextEdit::WidgetWidth);
    txt_edit->setAcceptRichText(false);
    txt_edit->setStyleSheet("QTextEdit { color: black; font-size: 18px;}");
    txt_edit->setReadOnly(true);

    QLabel* image = new QLabel();

    image->setAlignment(Qt::AlignCenter);

    image->setFixedSize(900,600);

    layout->addWidget(txt_lbl);
    layout->addWidget(image);

    if(count==1){layout->addWidget(txt_edit); image->hide();}

    btn_layout->addWidget(back_btn);
    btn_layout->addWidget(next_btn);
    layout->addWidget(closeButton);
    central_layout->addLayout(btn_layout);
    central_layout->addLayout(layout);
    dialog->setLayout(central_layout);


    connect(closeButton, &QPushButton::clicked, dialog, &QDialog::accept);

    connect(dialog, &QDialog::finished, this, [dialog]() {
        dialog->deleteLater();
    });

    connect(next_btn,&QPushButton::clicked,this,[this,txt_edit,image,next_btn,back_btn,txt_lbl](){
        if(count<4){
            txt_lbl->show();
            count++;
            next_btn->setEnabled(true);
            back_btn->setEnabled(true);

            QString executablePath = QCoreApplication::applicationDirPath();
            QString path = QString("/../rest_files/image/step%1.png").arg(QString::number(count));
            QString filePath = executablePath + path;

            QPixmap pix = filePath;
            image->setPixmap(pix);

            txt_edit->hide();
            image->show();
            txt_lbl->setText(text.at(count));
            return;
        }
        next_btn->setEnabled(false);
    });

    connect(back_btn,&QPushButton::clicked,this,[this,txt_edit,image,back_btn,next_btn,txt_lbl](){
        next_btn->setEnabled(true);
        back_btn->setEnabled(true);
        if(count>1){
            count--;
            if(count == 1){
                txt_edit->show();
                back_btn->setEnabled(false);
                image->hide();
                txt_lbl->hide();
                txt_edit->setText(text.at(count));
                return;
            }

            QString executablePath = QCoreApplication::applicationDirPath();
            QString path = QString("/../rest_files/image/step%1.png").arg(QString::number(count));
            QString filePath = executablePath + path;

            QPixmap pix = filePath;
            image->setPixmap(pix);

            txt_edit->hide();
            image->show();
            txt_lbl->setText(text.at(count));
            return;
        }
    });

    dialog->show();
}

QWidget *MainWindow::history_widget()
{
    QWidget* hist_widdget = new QWidget();
    QHBoxLayout* central_layout = new QHBoxLayout();

    QGraphicsView* graphicsView = new QGraphicsView();
    scene = new QGraphicsScene();
    graphicsView->setScene(scene);

    table = new QTableWidget();

    table->setColumnCount(4);
    QStringList headers;
    headers << "Remote URL" << "Branch" << "Commit" << "Date";
    table->setHorizontalHeaderLabels(headers);

    central_layout->addWidget(graphicsView);
    central_layout->addWidget(table);
    hist_widdget->setLayout(central_layout);
    return hist_widdget;
}

QWidget *MainWindow::create_repo_widget()
{
    QWidget* widget = new QWidget();

    QVBoxLayout* central_layout = new QVBoxLayout();

    QLabel* name_lbl = new QLabel("Repository name *");
    QLineEdit* name_repo = new QLineEdit();
    QLabel* description_lbl = new QLabel("Description");
    description = new QTextEdit("");

    QHBoxLayout* combobox_layout = new QHBoxLayout();

    QLabel* isPrivate_lbl = new QLabel("Choose visibility *");
    QComboBox* choose_private = new QComboBox();
    choose_private->addItem("Public");
    choose_private->addItem("Private");

    QHBoxLayout* checkbox_layout = new QHBoxLayout();

    QLabel* read_me_lbl = new QLabel("Add README");
    QCheckBox* add_readme = new QCheckBox();

    QPushButton* btn_create = new QPushButton("Create repository");

    current_count_lbl = new QLabel("0 / 350");

    connect(btn_create,&QPushButton::clicked,this,[=](){
        if(name_repo->text() == ""){
            set_text_in_console("Error: Specify the repository name");
            return;
        }
        else if(description->toPlainText().size()>350){
            set_text_in_console("Error: The description is too long");
            return;
        }else if (token_line->text()==""){
            set_text_in_console("Error: Specify the token line");
        }
        if(add_readme->checkState() == Qt::Unchecked){
            checked = false;
        }else{
            checked = true;
        }

        if(choose_private->currentText() == "Public"){
            priv = false;
        }else{
            priv = true;
        }

        wg->create_repositori(token_line->text(),name_repo->text(),description->toPlainText(),priv,checked);
    });

    central_layout->addWidget(name_lbl);
    central_layout->addWidget(name_repo);

    central_layout->addWidget(description_lbl);
    central_layout->addWidget(description);

    central_layout->addWidget(current_count_lbl);

    combobox_layout->addWidget(isPrivate_lbl);
    combobox_layout->addWidget(choose_private);
    central_layout->addLayout(combobox_layout);

    checkbox_layout->addWidget(read_me_lbl);
    checkbox_layout->addWidget(add_readme);
    central_layout->addLayout(checkbox_layout);

    central_layout->addWidget(btn_create);

    widget->setLayout(central_layout);
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
