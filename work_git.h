#ifndef WORK_GIT_H
#define WORK_GIT_H

#include "qnetworkaccessmanager.h"
#include <git2.h>
#include <QObject>

class work_git : public QObject
{
    Q_OBJECT
public:
    explicit work_git(QObject *parent = nullptr);
    void clone_repo(QStringList list);
    void check_direct(QString path);
    void check_push(QStringList list,int num);
    void push_project(QString remoteUrl,QString repoPath,QString commitMsg,QString token, QString name,const char* refspec,const char *branch_name);
    void push_project_in_repo(QString remoteUrl,QString repoPath,QString commitMsg,QString token, QString name,const char* refspec,const char *branch_name);
    void clear_resours();
    void create_repositori(const QString &token,const QString &name,const QString &description,const QString &private_,bool readme_);
signals:
    void message_signal(QString message);
    void tok_set();
private:
    void handleResponse(QNetworkReply *reply, const QString &repoName);
    QString tok;
    bool pause = true;
    git_repository* repo = nullptr;
    git_index* index = nullptr;
    git_oid tree_id, commit_id;
    git_tree* tree = nullptr;
    git_signature* sig = nullptr;
    git_remote* remote = nullptr;
    git_reference *head = NULL;
    git_commit *parent_commit = NULL;
    git_reference* local_branch_ref = nullptr;
    QNetworkAccessManager *m_manager;
};

#endif // WORK_GIT_H
