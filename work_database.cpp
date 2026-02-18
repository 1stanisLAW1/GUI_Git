#include "work_database.h"
#include "qdebug.h"

#include "qdatetime.h"
#include "qfileinfo.h"
#include "qthread.h"
#include <QCoreApplication>
#include <QMessageBox>
#include <QString>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

work_database::work_database(QObject *parent)
    : QObject{parent}
{
    QString executable_path = QCoreApplication::applicationDirPath();
    db_path = executable_path + "/../rest_files/db_history_push.db";

    // Проверяем существует ли файл
    QFileInfo dbFile(db_path);
    if (!dbFile.exists()) {
        emit message_signal("DB file not found: "+db_path+"(Файл базы данных не найден)");
    }

}

void work_database::add_data(QString url, QString branch, QString comm)
{
    if (!DB_Connect.isOpen()) {
        qDebug() << "DB not open, trying to open...";
        if (!DB_Connect.open()) {
            qDebug() << "Still cannot open DB:" << DB_Connect.lastError().text();
            emit message_signal("The database is not open!(БД не открыта)");
            return;
        }
    }

    QSqlQuery query(DB_Connect);
    QString date = QDate::currentDate().toString("dd.MM.yyyy");


    QString sql = "INSERT INTO data_push (URL_R, branch, comm, data) "
                  "VALUES ('" + url + "', '" + branch + "', '" + comm + "', '" + date + "')";
    if (query.exec(sql)) {
        emit message_signal("Data saved(Данные сохранены)");
    } else {
        qDebug() << "Query error:" << query.lastError().text();
        emit message_signal("Error: " + query.lastError().text());
    }
}

void work_database::initDatabase()
{
    QString connectionName = QString("thread_connection_%1").arg((qintptr)QThread::currentThreadId());

    DB_Connect = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    DB_Connect.setDatabaseName(db_path);

    qDebug() << "Opening DB in thread:" << QThread::currentThreadId();

    if (!DB_Connect.open()) {
        qDebug() << "Failed to open DB:" << DB_Connect.lastError().text();
        emit message_signal("Database opening error: " + DB_Connect.lastError().text()+"Ошибка открытия базы данных");
    } else {
        qDebug() << "DB opened successfully";
        emit message_signal("Database connected(БД подключена)");
    }
}

QVector<QStringList> work_database::loadTable(QString quer)
{
    QVector<QStringList> resultList;
    QSqlQuery query(DB_Connect);

    if(!query.prepare(quer)) {
        QMessageBox::warning(0, "Ошибка", "Не удалось подготовить запрос: " + query.lastError().text());
        qDebug()<<"Не удалось подготовить запрос: "<<query.lastError().text();
        return resultList;
    }

    if(!query.exec()) {
        QMessageBox::warning(0, "Ошибка", "Не удалось выполнить запрос: " + query.lastError().text());
        qDebug()<<"Не удалось выполнить запрос: "<<query.lastError().text();
        return resultList;
    }

    while(query.next()) {
        QStringList row;
        for(int i = 0; i < query.record().count(); ++i) {
            row.append(query.value(i).toString());
        }
        resultList.append(row);
    }

    if(resultList.isEmpty()) {
        qDebug() << "Запрос не вернул результатов";
    }
    return resultList;
}

