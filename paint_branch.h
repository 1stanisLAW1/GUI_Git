#ifndef PAINT_BRANCH_H
#define PAINT_BRANCH_H

#include "qgraphicsview.h"
#include <QObject>

class paint_branch : public QObject
{
    Q_OBJECT
public:
    explicit paint_branch(QObject *parent = nullptr);
    void test();
    QGraphicsView* paint(QList<QStringList> list);

signals:
};

#endif // PAINT_BRANCH_H
