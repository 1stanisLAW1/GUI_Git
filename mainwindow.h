#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "paint_branch.h"
#include "qlabel.h"
#include "qlineedit.h"
#include "qpushbutton.h"
#include "qtextedit.h"
#include "setting_window.h"
#include "work_database.h"
#include "work_git.h"
#include <QMainWindow>
#include <QGraphicsScene>
#include <QTableWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QTextEdit* console_widget();
    void select_path();
    QWidget* cloning_widget();
    QWidget* push_widget();
    void help_dialog();
    QWidget* history_widget();
    QWidget* create_repo_widget();


public slots:
    void set_text_in_console(QString message);
    void set_style();
    void slot_setting(QVector<QString>);
private:
    void append_all_widgets();
    void get_txt();
    void set_language();
    void set_style_bool(QList<QString> list);
    void update_table();
    void select_item_from_table(QTableWidget* table);

private:
    QVector<QStringList> list_;
    QString style_btn;
    QGraphicsView* graphicsView;
    work_database *w_db;
    QString style_;
    Ui::MainWindow *ui;
    QStringList list;
    QTextEdit* cons;
    QString path_cd;
    QPushButton* btn_clone;
    QPushButton* btn_ls;
    QPushButton* btn_push;
    QPushButton* btn_push_old;
    QPushButton* btn_path;
    QPushButton* btn_clear;
    QPushButton *btn_delete_repo;
    QLineEdit* commit_new;
    QLineEdit* commit_old;
    work_git* wg;
    QLineEdit* token_line;
    QLineEdit* branch_line;
    QDialog* dialog;
    int count = 1;
    QStringList text;
    QGraphicsScene* scene;
    QTableWidget* table;
    QLabel* current_count_lbl;
    QTextEdit* description;
    bool checked = false;
    bool priv = true;
    QList<QObject*> vec_;
    QList<QWidget*>all_widgets;
    QString tabWidget_Style;
    QString tabl_style;
    QString menu_style;
    QVector<QString> load_from_txt;
    QStringList russ_lang = {"Ссылка на проект GitHab","Имя репозитория *","Описание","0 / 350","Выберите видимость *",
                            "Добавить README","Токен:","ветка:","коммит:","коммит:",
                            "Клонировать","Проверить каталог","Выбрать путь","Очистить консоль",
                            "Удалить репоз.","Создать репоз.",
                            "Новый репоз.","Отправить репоз.",
                            "Отправить","Создать","История"};

    QStringList engl_lang = {"Link to GitHab project","Repository name *","Description","0 / 350","Choose visibility *",
                             "Add README","Token:","branch:","commit:","commit:",
                             "Clone","Сheck directory","Choose a path","Clear console",
                             "Delete repo","Create repository",
                             "Push new repo","Push repo",
                             "Push","Create","History"};

    void token_push();

    bool lang_engl;
    bool them_dark;
    bool hide_win = false;

    QVector<QPushButton*> btn_list;

    QAction* setting;
    QAction* helper;
};
#endif // MAINWINDOW_H
