#ifndef SETTING_WINDOW_H
#define SETTING_WINDOW_H

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

private:
    Ui::setting_window *ui;
    bool theme_dark = false;
    QPushButton* background_btn;
};

#endif // SETTING_WINDOW_H
