#include "paint_branch.h"
#include "qdebug.h"
#include "qgraphicsitem.h"

paint_branch::paint_branch(QObject *parent)
    : QObject{parent}
{

}

QGraphicsView *paint_branch::paint(QList<QStringList> list)
{
    QGraphicsScene* scene = new QGraphicsScene();

    scene->setSceneRect(0, 0, 1500, 1000);

    QGraphicsView* gr_view = new QGraphicsView(scene);

    gr_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    gr_view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    gr_view->setStyleSheet(
        "QScrollBar:vertical {"
        "    border: 1px solid #999999;"
        "    background: #f0f0f0;"
        "    width: 15px;"
        "    margin: 0px 0px 0px 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: #666666;"
        "    min-height: 20px;"
        "    border-radius: 5px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background: #333333;"
        "}"
        "QScrollBar:horizontal {"
        "    border: 1px solid #999999;"
        "    background: #f0f0f0;"
        "    height: 15px;"
        "    margin: 0px 0px 0px 0px;"
        "}"
        "QScrollBar::handle:horizontal {"
        "    background: #666666;"
        "    min-width: 20px;"
        "    border-radius: 5px;"
        "}"
        "QScrollBar::handle:horizontal:hover {"
        "    background: #333333;"
        "}"
        );

    QMap<QString, QList<QStringList>> branches;

    for(const QStringList& commit : list) {
        QString url = commit[0];
        branches[url].append(commit);
    }

    int startX = 100;
    int startY = 100;
    int branchSpacing = 300;
    int commitSpacing = 80;

    int maxCommits = 0;
    for(auto it = branches.begin(); it != branches.end(); ++it) {
        if(it.value().size() > maxCommits) {
            maxCommits = it.value().size();
        }
    }

    int requiredWidth = startX + branches.size() * branchSpacing + 200;
    int requiredHeight = startY + maxCommits * commitSpacing + 200;
    scene->setSceneRect(0, 0, requiredWidth, requiredHeight);


    int branchIndex = 0;
    for(auto it = branches.begin(); it != branches.end(); ++it) {
        QString url = it.key();
        QList<QStringList> commits = it.value();

        int branchX = startX + branchIndex * branchSpacing;

        int branchHeight = commits.size() * commitSpacing;
        QGraphicsLineItem* branchLine = new QGraphicsLineItem(
            branchX, startY, branchX, startY + branchHeight
            );

        QPen pen(Qt::blue);
        pen.setWidth(2);
        branchLine->setPen(pen);
        scene->addItem(branchLine);

        if(branchIndex > 0) {
            QGraphicsLineItem* connectLine = new QGraphicsLineItem(
                startX, startY + 30,
                branchX, startY + 30
                );
            QPen connectPen(Qt::gray);
            connectPen.setStyle(Qt::DashLine);
            connectLine->setPen(connectPen);
            scene->addItem(connectLine);
        }

        for(int i = 0; i < commits.size(); i++) {
            int commitY = startY + i * commitSpacing;

            QGraphicsEllipseItem* commitCircle = new QGraphicsEllipseItem(
                branchX - 12, commitY - 12, 24, 24
                );

            if(commits[i][2].contains("First commit")) {
                commitCircle->setBrush(QBrush(Qt::green));
            } else {
                commitCircle->setBrush(QBrush(QColor(100, 200, 255)));
            }

            scene->addItem(commitCircle);

            QString commitInfo = QString("Branch: %1\nMsg: %2\nDate: %3")
                                     .arg(commits[i][1])
                                     .arg(commits[i][2].left(15) + (commits[i][2].length() > 15 ? "..." : ""))  // commit message
                                     .arg(commits[i][3]);

            QGraphicsTextItem* textItem = new QGraphicsTextItem(commitInfo);
            textItem->setPos(branchX + 25, commitY - 25);
            textItem->setScale(0.8);

            textItem->setDefaultTextColor(Qt::black);
            QFont font = textItem->font();
            font.setPointSize(9);
            textItem->setFont(font);

            scene->addItem(textItem);

            if(i == 0) {
                QGraphicsTextItem* urlText = new QGraphicsTextItem(
                    url.length() > 30 ? url.left(27) + "..." : url
                    );
                urlText->setPos(branchX - 60, startY - 40);
                urlText->setDefaultTextColor(Qt::darkBlue);
                QFont urlFont;
                urlFont.setBold(true);
                urlFont.setPointSize(10);
                urlText->setFont(urlFont);
                scene->addItem(urlText);

                QGraphicsEllipseItem* startMarker = new QGraphicsEllipseItem(
                    branchX - 5, startY - 5, 10, 10
                    );
                startMarker->setBrush(QBrush(Qt::red));
                scene->addItem(startMarker);
            }
        }

        branchIndex++;
    }

    QGraphicsTextItem* title = new QGraphicsTextItem("Git Branches Visualization");
    title->setPos(requiredWidth/2 - 150, 20);
    QFont titleFont;
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setDefaultTextColor(Qt::darkBlue);
    scene->addItem(title);

    QGraphicsTextItem* legend = new QGraphicsTextItem(
        "Legend:\n"
        "● Green - First commit\n"
        "● Blue - Regular commit\n"
        "● Red - Branch start\n"
        "--- - Branch connection"
        );
    legend->setPos(requiredWidth - 250, requiredHeight - 150);
    legend->setDefaultTextColor(Qt::black);
    QFont legendFont;
    legendFont.setPointSize(10);
    legend->setFont(legendFont);
    scene->addItem(legend);

    return gr_view;
}
