#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "qlineedit.h"
#include "qpushbutton.h"
#include "qtextedit.h"
#include "work_git.h"
#include <QMainWindow>

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
public slots:
    void set_text_in_console(QString message);


private:
    Ui::MainWindow *ui;
    QStringList list;
    QTextEdit* cons;
    QString path_cd;
    QPushButton* btn_clone;
    QPushButton* btn_ls;
    QPushButton* btn_push;
    QPushButton* btn_push_old;
    QLineEdit* commit_new;
    QLineEdit* commit_old;
    work_git* wg;
    QLineEdit* token_line;
    QLineEdit* branch_line;
};
#endif // MAINWINDOW_H
