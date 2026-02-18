#include "work_file.h"
#include "qapplication.h"
#include "qdir.h"
#include "qmessagebox.h"
#include <QList>

work_file::work_file() {

}

QVector<QString> work_file::load_setting()
{
    QVector<QString> value;

    QMessageBox msg;

    QString executetablePath = QCoreApplication::applicationDirPath();
   // QString filePath = executetablePath + "/../../../rest_files/configure/setting_load.txt";
    QString filePath = executetablePath + "/../rest_files/configure/setting_load.txt"; // for realise
    QFile file(filePath);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {

        QTextStream stream(&file);
        QStringList result;

        while (!stream.atEnd()) {
            QString line = stream.readLine().trimmed();
            if (line.isEmpty()) continue;

            if(name_value.size() == 0) {
                file.close();
            }

            for(int i = 0; i < name_value.size(); i++) {
                QString pattern = QString("%1 = ").arg(name_value.at(i));
                if (line.startsWith(pattern)) {
                    QString value_ = line.mid(pattern.length()).trimmed();
                    value.append(value_);
                    if (!value.isEmpty()) {
                        result.append(value);
                    }
                }
            }
        }
    }else{
        msg.setWindowTitle("Error!");
        msg.setText("Не удалось открыть файл для записи");
        msg.setMinimumSize(100,50);
        msg.show();
        msg.exec();
    }

    lang = value.at(1);

    return value;
}

void work_file::save_config(QVector<QString>setting_value)
{

    QMessageBox msg;

    QString executetablePath = QCoreApplication::applicationDirPath();
    //QString filePath = executetablePath + "/../../../rest_files/configure/setting_load.txt";
    QString filePath = executetablePath + "/../rest_files/configure/setting_load.txt"; // for realise
    QFile file(filePath);

    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << "";
        file.close();
    } else {
        qDebug() << "Ошибка очистки лог-файла:" << file.errorString();
    }
    file.close();

    if(file.open(QIODevice::Append | QIODevice::Text)){
        QTextStream stream_text(&file);
        if(name_value.size() == setting_value.size()){
            for(int i = 0;i<setting_value.size();i++){
                stream_text<<name_value.at(i)<<" = "<<setting_value.at(i)+"\n";
            }
            if (stream_text.status() != QTextStream::Ok) {
                msg.setWindowTitle("Error!");
                msg.setText("Ошибка записи данных: "+filePath+" "+QString::number(stream_text.status()));
                msg.setMinimumSize(100,50);
                msg.show();
                msg.exec();
            }

            file.close();

            if (file.error() != QFile::NoError) {
                msg.setWindowTitle("Error!");
                msg.setText("Ошибка закрытия файла");
                msg.setMinimumSize(100,50);
                msg.show();
                msg.exec();
            }

            msg.setWindowTitle(" ");

            if(lang == "Russian"){
                msg.setText("Сохранение завершено");
            }else{
                msg.setText("Saving completed");
            }
            msg.setMinimumSize(100,50);
            msg.show();
            msg.exec();

        }else{
            qDebug()<<"Разные размеры контейнеров "<<name_value.size()<<" "<<setting_value.size();
        }
    }else{
        msg.setWindowTitle("Error!");
        msg.setText("Не удалось открыть файл для записи");
        msg.setMinimumSize(100,50);
        msg.show();
        msg.exec();
    }
}

void work_file::save_token(QString token)
{
    QMessageBox msg;

    QString executetablePath = QCoreApplication::applicationDirPath();
    //QString filePath = executetablePath + "/../../../rest_files/configure/save_token.txt";
    QString filePath = executetablePath + "/../rest_files/configure/save_token.txt"; // for realise
    QFile file(filePath);

    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << "";
        file.close();
    } else {
        qDebug() << "Ошибка очистки лог-файла:" << file.errorString();
    }
    file.close();

    if(file.open(QIODevice::Append | QIODevice::Text)){
        QTextStream stream_text(&file);
                stream_text<<encription_token(token);
            if (stream_text.status() != QTextStream::Ok) {
                msg.setWindowTitle("Error!");
                msg.setText("Ошибка записи данных: "+filePath+" "+QString::number(stream_text.status()));
                msg.setMinimumSize(100,50);
                msg.show();
                msg.exec();
            }

            file.close();

            if (file.error() != QFile::NoError) {
                msg.setWindowTitle("Error!");
                msg.setText("Ошибка закрытия файла");
                msg.setMinimumSize(100,50);
                msg.show();
                msg.exec();
            }

            msg.setWindowTitle(" ");

            if(lang == "Russian"){
                msg.setText("Сохранение завершено");
            }else{
                msg.setText("Saving completed");
            }
            msg.setMinimumSize(100,50);
            msg.show();
            msg.exec();

        }
        else{
        msg.setWindowTitle("Error!");
        msg.setText("Не удалось открыть файл для записи");
        msg.setMinimumSize(100,50);
        msg.show();
        msg.exec();
    }
}

QString work_file::load_token()
{
    QString tok;

    QMessageBox msg;

    QString executetablePath = QCoreApplication::applicationDirPath();
    //QString filePath = executetablePath + "/../../../rest_files/configure/save_token.txt";
    QString filePath = executetablePath + "/../rest_files/configure/save_token.txt"; // for realise
    QFile file(filePath);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {

        QTextStream stream(&file);
        QStringList result;

        while (!stream.atEnd()) {
            tok = stream.readLine().trimmed();
        }
    }else{
        msg.setWindowTitle("Error!");
        msg.setText("Не удалось открыть файл для записи");
        msg.setMinimumSize(100,50);
        msg.show();
        msg.exec();
    }

    return antiencription_token(tok);
}

QString work_file::encription_token(QString token)
{
    QString encript_token = "";
    for (int i = 0; i<token.length(); i++) {
        QChar char_ = token.at(i);
        encript_token+= QChar(char_.unicode() + 3);
    }
    return encript_token;
}

QString work_file::antiencription_token(QString token)
{
    QString encript_token = "";
    for (int i = 0; i<token.length(); i++) {
        QChar char_ = token.at(i);
        encript_token+= QChar(char_.unicode() - 3);
    }
    return encript_token;
}
