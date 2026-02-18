#include "mainwindow.h"
#include "qtimer.h"
#include "setting_window.h"
#include "ui_mainwindow.h"
#include "work_database.h"
#include "work_file.h"
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

    work_file *wf = new work_file();
    w_db = new work_database();
    wg = new work_git();
    setting_window *sw = new setting_window();

    set_style_bool(wf->load_setting());

    QTimer* timer = new QTimer(this);
    connect(timer,&QTimer::timeout,this,[=](){
        QString count_str =  QString::number(description->toPlainText().size())+" / 350";
        current_count_lbl->setText(count_str);
    });
    timer->start(100);

    QHBoxLayout* layout = new QHBoxLayout();
    QVBoxLayout* central_layout = new QVBoxLayout();
    QLabel *lbl = new QLabel("Link to GitHab project");
    QLineEdit* dr = new QLineEdit();
    QWidget* rest_widget =  cloning_widget();
    dr->resize(600,20);

    setting = new QAction();
    helper = new QAction();
    ui->menuSetting->addAction(setting);
    ui->menuSetting->addAction(helper);


    QThread* thread = new QThread();
    wg->moveToThread(thread);
    thread->start();

    QThread* thread_db = new QThread();
    w_db->moveToThread(thread_db);
    thread_db->start();


    QString query = QString("SELECT URL_R, branch, comm, data FROM data_push");

    w_db->initDatabase();
    list_ = w_db->loadTable(query);
    QTabWidget* tab_widget = new QTabWidget();
    tab_widget->setTabPosition(QTabWidget::West);
    tab_widget->resize(600,300);
    tab_widget->addTab(push_widget(),"Push");
    tab_widget->addTab(create_repo_widget(),"Create");
    tab_widget->addTab(history_widget(),"History"); //In development

    cons = console_widget();

    QString token = wf->load_token();
    if(token != ""){
        token_line->setText(token);
    }

    set_text_in_console("~path");

    layout->addWidget(lbl);
    layout->addWidget(dr);
    central_layout->addLayout(layout);
    central_layout->addWidget(rest_widget);
    central_layout->addWidget(tab_widget);
    central_layout->addWidget(cons);

    ui->centralwidget->setLayout(central_layout);

    append_all_widgets();

    set_style();

    connect(sw,&setting_window::signal_save,this,&MainWindow::slot_setting);
    connect(setting,&QAction::triggered,this,[=](){
            sw->resize(170,100);
           // sw->setAttribute(Qt::WA_DeleteOnClose); // delete if window close
            sw->set_style(them_dark,lang_engl);
            sw->show();
    });
    connect(thread_db, &QThread::started, w_db, &work_database::initDatabase);
    connect(btn_clone,&QPushButton::clicked,wg,[=,this](){
        list.append(dr->text());
        if(path_cd.isEmpty()){
            list.append("C:/");
        }else{
            list.append(path_cd + "/");
        }
        list.append(branch_line->text());
        wg->clone_repo(list);
        list.clear();
    });
    connect(btn_ls,&QPushButton::clicked,wg,[=,this](){
        wg->check_direct(path_cd);
    });
    connect(btn_clear,&QPushButton::clicked,this,[this](){
        cons->clear();
    });
    connect(helper,&QAction::triggered,this,&MainWindow::help_dialog);
    connect(wg,&work_git::message_signal,this,&MainWindow::set_text_in_console);
    connect(w_db,&work_database::message_signal,this,&MainWindow::set_text_in_console);
    connect(btn_path,&QPushButton::clicked,this,&MainWindow::select_path);
    connect(btn_push,&QPushButton::clicked,wg,[=](){

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

        token_push();

        QMetaObject::invokeMethod(w_db, "add_data",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, dr->text()+".git"),
                                  Q_ARG(QString, branch_line->text()),
                                  Q_ARG(QString, commit_new->text()));

        QTimer::singleShot(2000, [this]() {
           update_table();
        });
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

        token_push();

        QMetaObject::invokeMethod(w_db, "add_data",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, dr->text()+".git"),
                                  Q_ARG(QString, branch_line->text()),
                                  Q_ARG(QString, commit_new->text()));

        QTimer::singleShot(2000, [this]() {
            update_table();
        });
    });
    connect(btn_delete_repo,&QPushButton::clicked,wg,[=,this](){
        if(token_line->text().isEmpty()){
            set_text_in_console("Error: Fill in the token field");
        }else if(dr->text().isEmpty()){
            set_text_in_console("Error: Fill in the URL field");
        }
        wg->delete_repo(token_line->text(),dr->text());

    });

    if(lang_engl){
        text.clear();
        text.append(" ");
        text.append("https://github.com/settings/tokens");
        text.append("Click on «create token»");
        text.append("Select the necessary items and create a token");
        text.append("Copy the token. *My token is not active");
    }else{
        text.clear();
        text.append(" ");
        text.append("https://github.com/settings/tokens");
        text.append("Нажмите на «Создать токен»");
        text.append("Выберите необходимые элементы и создайте токен");
        text.append("Скопируйте токен. *Мой токен не активен");
    }
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

    QPushButton* closeButton = new QPushButton();
    closeButton->resize(40,30);
    QPushButton* next_btn = new QPushButton("->");
    QPushButton* back_btn = new QPushButton("<-");
    back_btn->setEnabled(false);

    QLabel* txt_lbl = new QLabel();

    QTextEdit* txt_edit = new QTextEdit(text.at(count));
    txt_edit->setLineWrapMode(QTextEdit::WidgetWidth);
    txt_edit->setAcceptRichText(false);
    txt_edit->setReadOnly(true);

    btn_list = {closeButton,next_btn,back_btn};

    if(them_dark == false){
        for(int a = 0;a<btn_list.size();a++){
            btn_list.at(a)->setStyleSheet("QPushButton { background-color: #d4cdcd;; color: black; font-size: 16px; border: 2px solid darkgray;"
                                       "border-radius:15px;"
                                       "max-height:30px;"
                                       "max-width:150px;"
                                       "min-height:30px;"
                                       "min-width:100px;}");
        }
        txt_lbl->setStyleSheet("QLabel { background-color: #d4cdcd;; color: black;");
        txt_edit->setStyleSheet("QTextEdit { background-color: #d4cdcd;; color: black;");
        dialog->setStyleSheet("QWidget { background-color: #d4cdcd;; color: black; }");
    }else{
        for(int a = 0;a<btn_list.size();a++){
            closeButton->setStyleSheet("QPushButton { background-color: black; color: white;  font-size: 16px; border: 2px solid darkgray;"
                                       "border-radius:15px;"
                                       "max-height:30px;"
                                       "max-width:150px;"
                                       "min-height:30px;"
                                       "min-width:100px;}");
        }
        txt_lbl->setStyleSheet("QLabel { background-color: black; color: white;");
        txt_edit->setStyleSheet("QTextEdit { background-color: black; color: white;");
        dialog->setStyleSheet("QWidget { background-color: black; color: white; }");
    }

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
    if(lang_engl){
        closeButton->setText("Close");
    }else{
        closeButton->setText("Закрыть");
    }


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
           // QString path = QString("/../../../rest_files/image/step%1.png").arg(QString::number(count));
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
            //QString path = QString("/../../../rest_files/image/step%1.png").arg(QString::number(count));
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

    connect(dialog, &QDialog::finished, [this](int result) {
        if (result == QDialog::Accepted) {
            qDebug() << "Dialog finished with Accepted";
        } else {
            qDebug() << "Dialog finished with Rejected";
            count = 1;
        }
    });
}

QWidget *MainWindow::history_widget()
{
    QWidget* hist_widdget = new QWidget();
    QHBoxLayout* central_layout = new QHBoxLayout();

    table = new QTableWidget();

    table->setColumnCount(4);
    table->setRowCount(list_.size());
    QStringList headers;
    headers << "Remote URL" << "Branch" << "Commit" << "Date";
    table->setHorizontalHeaderLabels(headers);

    central_layout->addWidget(table);
    hist_widdget->setLayout(central_layout);

    for(int i = 0;i<list_.size();i++){
        for(int a = 0;a<4;a++){
            QTableWidgetItem* item = new QTableWidgetItem(list_[i][a]);
            table->setItem(i,a,item);
        }
    }

    select_item_from_table(table);

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
        if(lang_engl == true){
            if(name_repo->text() == ""){
                set_text_in_console("Error: Specify the repository name");
                return;
            }
            else if(description->toPlainText().size()>350){
                set_text_in_console("Error: The description is too long");
                return;
            }else if (token_line->text()==""){
                set_text_in_console("Error: Specify the token line");
                return;
            }
        }else{
            if(name_repo->text() == ""){
                set_text_in_console("Ошибка: Укажите имя репозитория");
                return;
            }
            else if(description->toPlainText().size()>350){
                set_text_in_console("Ошибка: описание слишком длинное");
                return;
            }else if (token_line->text()==""){
                set_text_in_console("Ошибка: Укажите строку токена");
                return;
            }
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

        token_push();

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

void MainWindow::set_style()
{
    if(them_dark == false){
        style_btn = "QPushButton { background-color: #d4cdcd; color: black; font-size: 16px; border: 2px solid darkgray;"
                    "border-radius:15px;"
                    "max-height:30px;"
                    "max-width:150px;"
                    "min-height:30px;"
                    "min-width:100px;}";

        style_ = "%1 { background-color: #d4cdcd; color: black; font-size: 16px;}";

        tabWidget_Style =
            "QTabWidget::pane{border:1px solid #444;background:#d4cdcd;top:-1px;}"
            "QTabBar::tab{background:#c7c3c3;color:black;padding:4px 8px;border:1px solid #111;"
            "border-bottom:none;border-top-left-radius:2px;border-top-right-radius:2px;margin-right:1px;}"
            "QTabBar::tab:selected{background:#d4cdcd;color:black;border-color:#666;border-bottom:1px solid #d4cdcd;}"
            "QTabBar::tab:!selected{margin-top:1px;}"
            "QTabBar::tab:hover:!selected{background: darkgray;}";

        tabl_style =     "QTableWidget {"
                     "    background: #d4cdcd;"
                     "    color: black;"
                     "    gridline-color: #333;"
                     "    border: none;"
                     "}"
                     "QTableWidget::item {"
                     "    border-bottom: 1px solid #222;"
                     "    padding: 6px;"
                     "}"
                     "QTableWidget::item:selected {"
                     "    background: gray;"
                     "}"
                     "QHeaderView::section {"
                     "    background: darkgray;"
                     "    color: #d4cdcd;"
                     "    padding: 8px;"
                     "    border: none;"
                     "    border-bottom: 2px solid #444;"
                     "}";

        menu_style = "QMenuBar { background-color: rgb(220, 220, 220); color: black; }"
                     "QMenuBar::item:selected { background-color: rgb(220, 220, 220); color: rgb(255, 106, 37); }";

    }else if(them_dark == true){
        style_btn = "QPushButton { background-color: black; color: #d4cdcd;  font-size: 16px; border: 2px solid darkgray;"
                    "border-radius:15px;"
                    "max-height:30px;"
                    "max-width:150px;"
                    "min-height:30px;"
                    "min-width:100px;}";

        style_ = "%1 { background-color: black; color: #d4cdcd; font-size: 16px; }";

        tabWidget_Style =
            "QTabWidget::pane{border:1px solid #444;background:black;top:-1px;}"
            "QTabBar::tab{background:#222;color:#d4cdcd;padding:4px 10px;border:1px solid #444;"
            "border-bottom:none;border-top-left-radius:2px;border-top-right-radius:2px;margin-right:1px;}"
            "QTabBar::tab:selected{background:black;color:#d4cdcd;border-color:#666;border-bottom:1px solid black;}"
            "QTabBar::tab:!selected{margin-top:1px;}"
            "QTabBar::tab:hover:!selected{background:#333;}";

        tabl_style =     "QTableWidget {"
                     "    background: black;"
                     "    color: #d4cdcd;"
                     "    gridline-color: #333;"
                     "    border: none;"
                     "}"
                     "QTableWidget::item {"
                     "    border-bottom: 1px solid #222;"
                     "    padding: 6px;"
                     "}"
                     "QTableWidget::item:selected {"
                     "    background: #333;"
                     "}"
                     "QHeaderView::section {"
                     "    background: #111;"
                     "    color: #d4cdcd;"
                     "    padding: 8px;"
                     "    border: none;"
                     "    border-bottom: 2px solid #444;"
                     "}";

        menu_style = "QMenuBar { background-color:black; color: #d4cdcd; }"
                     "QMenuBar::item:selected { background-color: black; color: rgb(255, 106, 37); }";
    }

    for(int i = 0;i<all_widgets.size();i++){
        all_widgets.at(i)->setStyleSheet(style_btn);
        QString class_name = all_widgets.at(i)->metaObject()->className();
        if(class_name == "QPushButton"){
            all_widgets.at(i)->setStyleSheet(style_btn);
        }else if(class_name == "QTabWidget"){
            all_widgets.at(i)->setStyleSheet(tabWidget_Style);
        }else if(class_name == "QTableWidget"){
            all_widgets.at(i)->setStyleSheet(tabl_style);
        }else{
            QString style = style_.arg(class_name);
            all_widgets.at(i)->setStyleSheet(style);
        }
    }
    QString style = style_.arg(ui->centralwidget->metaObject()->className());
    ui->centralwidget->setStyleSheet(style);
    QString style_2 = style_.arg(ui->menuSetting->metaObject()->className());
    ui->menuSetting->setStyleSheet(style_2);
    QString style_3 = menu_style.arg(ui->menuBar->metaObject()->className());
    ui->menuBar->setStyleSheet(style_3);

    set_language();

}

void MainWindow::slot_setting(QVector<QString> settings)
{
    if(settings.size()!=2){qDebug()<<"Error list"; return;}

    if(settings.at(0) == "dark"||settings.at(0) == "тёмная"){them_dark = true;}
    else{them_dark = false;}

    if(settings.at(1) == "Russian"){lang_engl = false;}
    else {lang_engl = true;}

    set_style();
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

void MainWindow::append_all_widgets()
{
    vec_ = ui->centralwidget->findChildren<QObject*>();
    foreach (QObject* obj, vec_) {
        if (QWidget* widget = qobject_cast<QWidget*>(obj)) {
            all_widgets.append(widget);
        }
    }
}

void MainWindow::get_txt()
{
    foreach (QLabel* lbl, ui->centralwidget->findChildren<QLabel*>()) {
        qDebug()<<lbl->text();
    }

    foreach (QPushButton* btn, ui->centralwidget->findChildren<QPushButton*>()) {
        qDebug()<<btn->text();
    }

    foreach (QTabWidget* tb, ui->centralwidget->findChildren<QTabWidget*>()) {
        for(int i = 0; i<tb->count();i++){
            qDebug()<<tb->tabText(i);
        }
    }

}

void MainWindow::set_language()
{
    if(lang_engl == false){
        int i = 0;
        foreach (QLabel* lbl, ui->centralwidget->findChildren<QLabel*>()) {
            lbl->setText(russ_lang.at(i));
            i++;
        }
        foreach (QPushButton* btn, ui->centralwidget->findChildren<QPushButton*>()) {
            btn->setText(russ_lang.at(i));
            i++;
        }

        setting->setText("Настройки");
        helper->setText("Как получить токен");
        ui->menuSetting->setTitle("Программа");

        foreach (QTabWidget* tb, ui->centralwidget->findChildren<QTabWidget*>()) {
            for(int a = 0; a<tb->count();a++){
                tb->setTabText(a,russ_lang.at(i));
                i++;
            }
        }
    }else{        int i = 0;
        foreach (QLabel* lbl, ui->centralwidget->findChildren<QLabel*>()) {
            lbl->setText(engl_lang.at(i));
            i++;
        }
        foreach (QPushButton* btn, ui->centralwidget->findChildren<QPushButton*>()) {
            btn->setText(engl_lang.at(i));
            i++;
        }

        foreach (QTabWidget* tb, ui->centralwidget->findChildren<QTabWidget*>()) {
            for(int a = 0; a<tb->count();a++){
                tb->setTabText(a,engl_lang.at(i));
                i++;
            }
        }

        setting->setText("Settings");
        helper->setText("How to get token");
        ui->menuSetting->setTitle("Programm");
    }
}

void MainWindow::set_style_bool(QList<QString> list)
{
    if(list.size()!=2){qDebug()<<"Error list"; return;}

    if(list.at(0) == "dark"||list.at(0) == "тёмная"){them_dark = true;}
    else{them_dark = false;}

    if(list.at(1) == "Russian"){lang_engl = false;}
    else{lang_engl = true;}
}

void MainWindow::update_table()
{
    QStringList headers;
    for(int col = 0; col < table->columnCount(); col++) {
        if(table->horizontalHeaderItem(col)) {
            headers << table->horizontalHeaderItem(col)->text();
        }
    }

    table->clear();

    table->setColumnCount(4);
    table->setHorizontalHeaderLabels(QStringList() << "URL_R" << "Branch" << "Commit" << "Date");

    QString query = QString("SELECT URL_R, branch, comm, data FROM data_push");
    w_db->initDatabase();
    list_ = w_db->loadTable(query);

    table->setRowCount(list_.size());

    for(int i = 0; i < list_.size(); i++) {
        for(int a = 0; a < 4 && a < list_[i].size(); a++) {
            QTableWidgetItem* item = new QTableWidgetItem(list_[i][a]);
            table->setItem(i, a, item);
        }
    }
}

void MainWindow::select_item_from_table(QTableWidget *table)
{
    paint_branch* pb = new paint_branch();
    QList<QStringList> li;

    for(int i = 0; i < table->rowCount(); i++){
        QStringList list;
        for(int a = 0; a < 4; a++){
            list.append(table->item(i, a)->text());
        }
        li.append(list);
    }

    std::sort(li.begin(), li.end(), [](const QStringList& a, const QStringList& b) {
        QDate date1 = QDate::fromString(a[3], "dd.MM.yyyy");
        QDate date2 = QDate::fromString(b[3], "dd.MM.yyyy");
        return date1 < date2;
    });

    if(graphicsView) {
        delete graphicsView;
    }

    graphicsView = pb->paint(li);

    graphicsView->setRenderHint(QPainter::Antialiasing);
    graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    graphicsView->setWindowTitle("Git Branches Visualization");

    graphicsView->resize(1024, 768);

    graphicsView->show();

    graphicsView->centerOn(graphicsView->scene()->sceneRect().center());
}

void MainWindow::token_push()
{
    if(token_line->text().isEmpty()){
        set_text_in_console("Specify the token line");
        return;
    }

    QString token = token_line->text();

    QTimer::singleShot(0, this, [=](){
        work_file wf;
        wf.save_token(token);
    });
}
