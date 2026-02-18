#ifndef WORK_FILE_H
#define WORK_FILE_H

#include <QString>
#include <QList>

class work_file
{
public:
    work_file();
    QVector<QString> load_setting();
    void save_config(QVector<QString>);
    void save_token(QString token);
    QString load_token();
    void test();
private:
    QList<QString> name_value = {"theme","languege"};
    QString lang;
    QString encription_token(QString);
    QString antiencription_token(QString);
};

#endif // WORK_FILE_H
