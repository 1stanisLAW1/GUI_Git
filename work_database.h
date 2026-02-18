#ifndef WORK_DATABASE_H
#define WORK_DATABASE_H

#include "qsqldatabase.h"
#include <QObject>

class work_database : public QObject
{
    Q_OBJECT
public:
    explicit work_database(QObject *parent = nullptr);
public slots:
    void add_data(QString url,QString branch,QString commit);
    void initDatabase();
    QVector<QStringList>loadTable(QString quer);
private:
    QSqlDatabase DB_Connect;
    QString db_path;

signals:
    void message_signal(QString message);
};

#endif // WORK_DATABASE_H
