#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

#include <QObject>

class Error_Handling : public QObject
{
    Q_OBJECT
public:
    explicit Error_Handling(QObject *parent = nullptr);
    bool error_check(int error);

signals:
};

#endif // ERROR_HANDLING_H
