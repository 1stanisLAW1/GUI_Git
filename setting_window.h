#ifndef SETTING_WINDOW_H
#define SETTING_WINDOW_H

#include "qcombobox.h"
#include "qlabel.h"
#include "qpushbutton.h"
#include <QMainWindow>

namespace Ui {
class setting_window;
}
class setting_window : public QMainWindow
{
    Q_OBJECT

public:
    explicit setting_window(QWidget *parent = nullptr);
    ~setting_window();
     void theme();
   // void save_config();
  //  QVector<QString> load_setting();
  //  void set_style(QVector<QString>);
    void set_style(bool theme_dark,bool lang_engl);
    void updateBackgroundButtonText();
signals:
    void signal_save(QVector<QString>);
private:
    void style();
    void update_lang();
    void update_combo_box();
    QWidget *lang_widget();
    Ui::setting_window *ui;
    QLabel* lbl;
    QPushButton* background_btn;
    QComboBox* languege_box;
    QVector<QString>setting_value;
    QVector<QString> value;
    //QList<QString>style_;
    QPushButton* save_btn;
    bool lang_engl;
    bool them_dark;

};

#endif // SETTING_WINDOW_H
