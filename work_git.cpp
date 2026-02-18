#include "work_git.h"
#include <git2.h>
#include <git2/remote.h>
#include <git2/credential.h>
#include "qdebug.h"
#include "qlineedit.h"
#include "qprocess.h"
#include <QInputDialog>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

#include <QDir>


work_git::work_git(QObject *parent)
    : QObject{parent}
{
}

void work_git::clone_repo(QStringList list)
{
    if (list.size() < 3) {
        emit message_signal("Недостаточно параметров. Нужны: URL, путь, ветка(Not enough parameters. Required: URL, path, branch)");
        return;
    }

    git_libgit2_init();

    QString url = list.at(0);
    QString targetPath = list.at(1);
    QString branchName = list.at(2);

    int last_Slash_Pos = url.lastIndexOf('/');
    QString last_part = url.mid(last_Slash_Pos + 1);

    if (last_part.endsWith(".git")) {
        last_part.chop(4);
    }

    QString fullPath = targetPath + last_part + "/";

    emit message_signal("Начинаем процесс клонирования(Starting the cloning process)...");
    emit message_signal(QString("Ветка(Branch): %1").arg(branchName));

    git_repository *repo = NULL;
    git_clone_options clone_opts;
    git_clone_options_init(&clone_opts, GIT_CLONE_OPTIONS_VERSION);

    if (!branchName.isEmpty() && branchName.toLower() != "master") {
        git_checkout_options checkout_opts;
        git_checkout_options_init(&checkout_opts, GIT_CHECKOUT_OPTIONS_VERSION);
        checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;

        clone_opts.checkout_opts = checkout_opts;

        git_fetch_options fetch_opts;
        git_fetch_options_init(&fetch_opts, GIT_FETCH_OPTIONS_VERSION);
        clone_opts.fetch_opts = fetch_opts;

        int error = git_clone(&repo, url.toUtf8().constData(),
                              fullPath.toUtf8().constData(), &clone_opts);

        if (error != 0) {
            const git_error *e = git_error_last();
            QString str = QString("Ошибка клонирования(Cloning error) %1/%2: %3")
                              .arg(error).arg(e->klass).arg(e->message);
            emit message_signal("Ошибка клонирования(Cloning error) - ✗");
            emit message_signal(str);
            git_libgit2_shutdown();
            return;
        }

        emit message_signal("Переключаемся на указанную ветку(Switching to the specified branch)...");

        git_reference *remote_branch = nullptr;
        QString remote_ref = QString("origin/%1").arg(branchName);

        error = git_branch_lookup(&remote_branch, repo,
                                  remote_ref.toUtf8().constData(),
                                  GIT_BRANCH_REMOTE);

        if (error == 0) {
            git_reference *local_branch = nullptr;

            git_commit *target_commit = nullptr;
            git_object *target_obj = nullptr;

            error = git_reference_peel(&target_obj, remote_branch, GIT_OBJECT_COMMIT);
            if (error == 0) {
                target_commit = (git_commit*)target_obj;

                error = git_branch_create(&local_branch, repo,
                                          branchName.toUtf8().constData(),
                                          target_commit, 0);

                if (error == 0) {
                    git_checkout_options checkout_opts_local;
                    git_checkout_options_init(&checkout_opts_local,
                                              GIT_CHECKOUT_OPTIONS_VERSION);
                    checkout_opts_local.checkout_strategy = GIT_CHECKOUT_SAFE;

                    error = git_checkout_tree(repo,
                                              (const git_object*)target_commit,
                                              &checkout_opts_local);

                    if (error == 0) {
                        // Устанавливаем HEAD на новую ветку
                        git_repository_set_head(repo,
                                                QString("refs/heads/%1")
                                                    .arg(branchName)
                                                    .toUtf8().constData());

                        emit message_signal(QString("Переключено на ветку(Switched to branch) %1 - ✓").arg(branchName));
                    }

                    git_reference_free(local_branch);
                }

                git_object_free(target_obj);
            }

            git_reference_free(remote_branch);
        } else {
            emit message_signal(QString("Ветка(Branch) %1 не найдена в удаленном репозитории(not found in the remote repository)").arg(branchName));
            emit message_signal("Будет использована ветка по умолчанию(The default branch will be used) master*");
        }

    } else {
        int error = git_clone(&repo, url.toUtf8().constData(),
                              fullPath.toUtf8().constData(), &clone_opts);

        if (error != 0) {
            const git_error *e = git_error_last();
            QString str = QString("Ошибка клонирования(Cloning error) %1/%2: %3")
                              .arg(error).arg(e->klass).arg(e->message);
            emit message_signal("Ошибка клонирования(Cloning error) - ✗");
            emit message_signal(str);
            git_libgit2_shutdown();
            return;
        }
    }

    git_reference *head = nullptr;
    int error = git_repository_head(&head, repo);

    if (error == 0) {
        const char *branch_name;
        git_branch_name(&branch_name, head);
        emit message_signal(QString("Текущая ветка(Current branch): %1").arg(branch_name));
        git_reference_free(head);
    }

    if (repo) git_repository_free(repo);

    emit message_signal("Клонирование (Cloning completed) - ✓");
    git_libgit2_shutdown();
}

void work_git::check_direct(QString path)
{
    if(path == ""){
        emit message_signal("Choose a path");
        return;
    }

    QDir dir(path);

    // Получаем все элементы (и файлы, и папки)
    QStringList entries = dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);

    for(QString &entry : entries){
        // Проверяем, является ли элемент папкой
        QFileInfo fileInfo(dir.absoluteFilePath(entry));
        if(fileInfo.isDir()){
            emit message_signal("📁 " + entry);
        } else {
            emit message_signal("📄 " + entry);
        }
    }
}

void work_git::check_push(QStringList list, int num)
{
    emit message_signal("Data verification(Проверка данных)...");
    git_libgit2_init();

    QString remoteUrl = list.at(0);
    QString repoPath = QDir::toNativeSeparators(list.at(1));
    QString commitMsg = list.size() > 2 ? list.at(2) : "Initial commit";

    QString branchName = list.at(4);
    QByteArray branchNameBytes = branchName.toUtf8();
    QByteArray refspecBytes = QString("refs/heads/%1:refs/heads/%1")
                                  .arg(list.at(4))
                                  .toUtf8();

    const char* refspec = refspecBytes.constData();
    const char *branch_name = branchNameBytes.constData();



    QString token = list.at(3);

    QDir dir(repoPath);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            emit message_signal("Error: Failed to create directory(Ошибка: не удалось создать каталог)");
            git_libgit2_shutdown();
            return;
        }
    }

    if (remoteUrl.isEmpty()||repoPath.isEmpty()||commitMsg.isEmpty()||branchName.isEmpty()||refspecBytes.isEmpty()) {
        emit message_signal("Error: All fields must be filled out.(Ошибка: Все поля должны быть заполнены)");
        git_libgit2_shutdown();
        return;
    }

    QString base = "https://github.com/";

    int startPos = base.length();
    int endPos = remoteUrl.indexOf('/', startPos);

    if (endPos == -1) {endPos = remoteUrl.length();}

    QString name = remoteUrl.mid(startPos, endPos - startPos);
    if(num == 0){
        push_project(remoteUrl,repoPath,commitMsg,token,name,refspec,branch_name);
    }else{
        push_project_in_repo(remoteUrl,repoPath,commitMsg,token,name,refspec,branch_name);
    }
}

void work_git::push_project(QString remoteUrl, QString repoPath, QString commitMsg, QString token, QString name, const char *refspec, const char *branch_name)
{
    // Initializing a new repository
    git_repository_init_options opts;
    git_repository_init_init_options(&opts, GIT_REPOSITORY_INIT_OPTIONS_VERSION);
    opts.flags = GIT_REPOSITORY_INIT_MKPATH;

    int error = git_repository_init_ext(&repo, repoPath.toUtf8().constData(), &opts);

    if (error != GIT_OK || !repo) {
        const git_error* e = git_error_last();
        emit message_signal(QString("Initialization error: %1")
                                .arg(e ? e->message : "Unknown error"));
        git_libgit2_shutdown();
        return;
    }

    if ((error = git_signature_now(&sig, "Your Name", "your.email@example.com")) != GIT_OK) {
        emit message_signal("Error creating signature");
        git_repository_free(repo);
        git_libgit2_shutdown();
        return;
    }

    // Adding all files
    if ((error = git_repository_index(&index, repo)) != GIT_OK ||
        (error = git_index_add_all(index, nullptr, 0, nullptr, nullptr)) != GIT_OK ||
        (error = git_index_write_tree(&tree_id, index)) != GIT_OK ||
        (error = git_tree_lookup(&tree, repo, &tree_id)) != GIT_OK) {
        const git_error* e = git_error_last();
        emit message_signal(QString("Error preparing the commit: %1")
                                .arg(e ? e->message : "Unknown error"));
        git_index_free(index);
        git_signature_free(sig);
        git_repository_free(repo);
        git_libgit2_shutdown();
        return;
    }
    git_index_free(index);

    // Creating a commit
    error = git_commit_create_v(
        &commit_id, repo, "HEAD", sig, sig,
        nullptr, commitMsg.toUtf8().constData(), tree,
        0, nullptr);

    if (error != GIT_OK) {
        const git_error* e = git_error_last();
        emit message_signal(QString("Error creating commit: %1")
                                .arg(e ? e->message : "Unknown error"));
    }

    // Setting up a remote repository
    error = git_remote_create(&remote, repo, "origin", remoteUrl.toUtf8().constData());

    if (error != GIT_OK) {
        const git_error* e = git_error_last();
        emit message_signal(QString("Configuration error remote: %1")
                                .arg(e ? e->message : "Unknown error"));
        git_tree_free(tree);
        git_signature_free(sig);
        git_repository_free(repo);
        git_libgit2_shutdown();
        return;
    }
    if (token.isEmpty()) {
        emit message_signal("Authentication cancelled(Аутентификация отменена)");
        git_tree_free(tree);
        git_signature_free(sig);
        git_repository_free(repo);
        git_libgit2_shutdown();
        return;
    }

    // Setting up URL with token
    if (remoteUrl.startsWith("https://")) {
        remoteUrl.insert(8,name+":"+token + "@");
    }

    // Updating the URL of the remote repository
    error = git_remote_set_url(repo, "origin", remoteUrl.toUtf8().constData());
    if (error != GIT_OK) {
        const git_error* e = git_error_last();
        emit message_signal(QString("Failed to set auth URL: %1").arg(e ? e->message : "Unknown"));
        git_remote_free(remote);
        git_tree_free(tree);
        git_signature_free(sig);
        git_repository_free(repo);
        git_libgit2_shutdown();
        return;
    }

    // We are getting the updated remote
    git_remote_free(remote);
    error = git_remote_lookup(&remote, repo, "origin");
    if (error != GIT_OK) {}


    error = git_repository_head(&head, repo);
    if (error == 0) {
        error = git_commit_lookup(&parent_commit, repo, git_reference_target(head));
    }

    //  Creating a branch
    git_reference *branch = NULL;
    if (error == 0) {
        error = git_branch_create(&branch, repo, branch_name, parent_commit, 0);
    }

    if (error == 0) {
        printf("Branch(Ветка) %s successfully created(успешно создано)\n", branch_name);
    } else {
        const git_error *e = git_error_last();
        printf("Error creating branch(Ошибка при создании ветки): %s\n", e->message);
    }

    // push
    git_push_options push_opts;
    git_push_options_init(&push_opts, GIT_PUSH_OPTIONS_VERSION);

    git_strarray refspecs = { (char**)&refspec, 1 };

    error = git_remote_push(remote, &refspecs, &push_opts);
    if (error != GIT_OK) {
        const git_error* e = git_error_last();
        emit message_signal(QString("Push failed: %1").arg(e ? e->message : "Unknown error"));
    } else {
        emit message_signal(QString("Successfully pushed to(Успешно отправлено в) %1 - ✓").arg(remoteUrl));
    }

    clear_resours();
    git_libgit2_shutdown();
}

void work_git::push_project_in_repo(QString remoteUrl, QString repoPath, QString commitMsg, QString token, QString name, const char *refspec, const char *branch_name)
{
    // Opening the existing repository
    int error = git_repository_open(&repo, repoPath.toUtf8().constData());
    if (error != GIT_OK) {
        const git_error* e = git_error_last();
        emit message_signal(QString("Error opening the repository(Ошибка при открытии репозитория): %1")
                                .arg(e ? e->message : "Unknown error"));
        git_libgit2_shutdown();
        return;
    }

    // Check if there are changes to commit
    git_status_options status_opts = GIT_STATUS_OPTIONS_INIT;
    status_opts.show = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
    status_opts.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED;

    git_status_list *status = NULL;
    error = git_status_list_new(&status, repo, &status_opts);
    if (error != GIT_OK) {
        const git_error* e = git_error_last();
        emit message_signal(QString("Error checking status(Ошибка проверки статуса): %1")
                                .arg(e ? e->message : "Unknown error"));
        git_repository_free(repo);
        git_libgit2_shutdown();
        return;
    }

    size_t entrycount = git_status_list_entrycount(status);
    if (entrycount == 0) {
        emit message_signal("No changes to commit(Нет изменений для фиксации)");
        git_status_list_free(status);
        git_repository_free(repo);
        git_libgit2_shutdown();
        return;
    }
    git_status_list_free(status);

    if ((error = git_signature_now(&sig, "Your Name", "your.email@example.com")) != GIT_OK) {
        emit message_signal("Error creating signature(Ошибка при создании подписи)");
        git_repository_free(repo);
        git_libgit2_shutdown();
        return;
    }

    // Adding all files
    if ((error = git_repository_index(&index, repo)) != GIT_OK ||
        (error = git_index_add_all(index, nullptr, 0, nullptr, nullptr)) != GIT_OK ||
        (error = git_index_write(index)) != GIT_OK ||
        (error = git_index_write_tree(&tree_id, index)) != GIT_OK ||
        (error = git_tree_lookup(&tree, repo, &tree_id)) != GIT_OK) {
        const git_error* e = git_error_last();
        emit message_signal(QString("Error preparing the commit(Ошибка при получении родительского коммита): %1")
                                .arg(e ? e->message : "Unknown error"));
        if (index) git_index_free(index);
        git_signature_free(sig);
        git_repository_free(repo);
        git_libgit2_shutdown();
        return;
    }
    git_index_free(index);

    // Getting the parent commit
    if ((error = git_repository_head(&head, repo)) != GIT_OK ||
        (error = git_commit_lookup(&parent_commit, repo, git_reference_target(head))) != GIT_OK) {
        const git_error* e = git_error_last();
        emit message_signal(QString("Error getting parent commit(Ошибка при получении родительского коммита): %1")
                                .arg(e ? e->message : "Unknown error"));
        git_tree_free(tree);
        git_signature_free(sig);
        git_repository_free(repo);
        git_libgit2_shutdown();
        return;
    }

    // Created commit
    error = git_commit_create_v(&commit_id, repo, "HEAD", sig, sig,nullptr, commitMsg.toUtf8().constData(), tree,1, parent_commit);

    if (error != GIT_OK) {
        const git_error* e = git_error_last();
        emit message_signal(QString("Error creating commit(Ошибка при создании коммита): %1")
                                .arg(e ? e->message : "Unknown error"));
        git_tree_free(tree);
        git_signature_free(sig);
        git_repository_free(repo);
        git_libgit2_shutdown();
        return;
    }

    // Setting up URL with token
    if (token.isEmpty()) {
        emit message_signal("Authentication cancelled(Аутентификация отменена)");
        git_tree_free(tree);
        git_signature_free(sig);
        git_repository_free(repo);
        git_libgit2_shutdown();
        return;
    }

    if (remoteUrl.startsWith("https://")) {
        remoteUrl.insert(8,name+ ":" + token + "@");
    }

    // We get or create a remote
    error = git_remote_lookup(&remote, repo, "origin");
    if (error != GIT_OK) {
        error = git_remote_create(&remote, repo, "origin", remoteUrl.toUtf8().constData());
        if (error != GIT_OK) {
            const git_error* e = git_error_last();
            emit message_signal(QString("Remote configuration error(Ошибка удалённой настройки): %1")
                                    .arg(e ? e->message : "Unknown error"));
            git_tree_free(tree);
            git_signature_free(sig);
            git_repository_free(repo);
            git_libgit2_shutdown();
            return;
        }
    }

    // Updating the URL of the remote repository
    error = git_remote_set_url(repo, "origin", remoteUrl.toUtf8().constData());
    if (error != GIT_OK) {
        const git_error* e = git_error_last();
        emit message_signal(QString("Failed to set auth URL(Не удалось установить URL аутентификации): %1").arg(e ? e->message : "Unknown"));
        git_remote_free(remote);
        git_tree_free(tree);
        git_signature_free(sig);
        git_repository_free(repo);
        git_libgit2_shutdown();
        return;
    }

    // Checking the existence of the branch
    git_reference* local_branch_ref = nullptr;
    QString local_branch_name = QString("refs/heads/%1").arg(branch_name);
    bool local_branch_exists = (git_reference_lookup(&local_branch_ref, repo, local_branch_name.toUtf8().constData()) == GIT_OK);

    // If there is no local branch - we create it.
    if (!local_branch_exists) {
        git_reference* head_ref = nullptr;
        if (git_repository_head(&head_ref, repo) == GIT_OK) {
            const git_oid* head_oid = git_reference_target(head_ref);
            git_commit* commit = nullptr;
            if (git_commit_lookup(&commit, repo, head_oid) == GIT_OK) {
                if (git_branch_create(&local_branch_ref, repo,
                                      branch_name,
                                      commit,
                                      0) != GIT_OK) {
                    const git_error* e = git_error_last();
                    emit message_signal(QString("Failed to create local branch(Не удалось создать локальную ветку:): %1").arg(e->message));
                }
                git_commit_free(commit);
            }
            git_reference_free(head_ref);
        }
    }

    // Switching to the required branch
    if (local_branch_exists || local_branch_ref) {
        git_reference* branch_to_checkout = local_branch_exists ? local_branch_ref : nullptr;
        if (!branch_to_checkout) {
            git_reference_lookup(&branch_to_checkout, repo, local_branch_name.toUtf8().constData());
        }

        if (branch_to_checkout) {
            git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;
            checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;
            if (git_repository_set_head(repo, git_reference_name(branch_to_checkout)) != GIT_OK ||
                git_checkout_head(repo, &checkout_opts) != GIT_OK) {
                const git_error* e = git_error_last();
                emit message_signal(QString("Failed to checkout branch(Не удалось переключиться на ветку): %1").arg(e->message));
            }

            if (!local_branch_exists) {
                git_reference_free(branch_to_checkout);
            }
        }
    }

    if (local_branch_ref && !local_branch_exists) {
        git_reference_free(local_branch_ref);
    }

    // Checking the existence of the branch on the remote server
    bool remote_branch_exists = false;
    git_reference* remote_branch_ref = nullptr;
    QString remote_branch_name = QString("refs/remotes/origin/%1").arg(branch_name);
    if (git_reference_lookup(&remote_branch_ref, repo, remote_branch_name.toUtf8().constData()) == GIT_OK) {
        remote_branch_exists = true;
        git_reference_free(remote_branch_ref);
    }

    // Push settings considering the existence of a branch
    QString refspec_str = QString("+refs/heads/%1:refs/heads/%1").arg(branch_name);

    // We are performing a push
    git_push_options push_opts;
    git_push_options_init(&push_opts, GIT_PUSH_OPTIONS_VERSION);

    CredentialsPayload credentials = {name, token};
    push_opts.callbacks.payload = &credentials;

    push_opts.callbacks.credentials = [](git_cred **out, const char *url, const char *username_from_url,
                                         unsigned int allowed_types, void *payload) -> int {
        CredentialsPayload *creds = static_cast<CredentialsPayload*>(payload);
        return git_cred_userpass_plaintext_new(out,
                                               creds->username.toUtf8().constData(),
                                               creds->token.toUtf8().constData());
    };

    QByteArray refspecBytes = refspec_str.toUtf8();
    const char* refspec_cstr = refspecBytes.constData();
    git_strarray refspecs = { (char**)&refspec_cstr, 1 };

    emit message_signal("The process of pushing(Не удалось выполнить push)...");

    int push_error = git_remote_push(remote, &refspecs, &push_opts);
    if (push_error != GIT_OK) {
        const git_error* e = git_error_last();
        emit message_signal(QString("Push failed(Не удалось выполнить push): %1").arg(e ? e->message : "Unknown error"));
    } else {
        emit message_signal(QString("Successfully force-pushed to branch(Успешно выполнен принудительный push в ветку) %1 - ✓").arg(branch_name));
    }

    clear_resours();
    git_libgit2_shutdown();
}

void work_git::clear_resours()
{
    git_tree_free(tree);
    git_signature_free(sig);
    git_remote_free(remote);
    if (parent_commit) git_commit_free(parent_commit);
    if (head) git_reference_free(head);
    git_repository_free(repo);
}
void work_git::create_repositori(const QString &token, const QString &name, const QString &description, bool private_, bool readme_)
{
    m_manager = new QNetworkAccessManager(this);

    QJsonObject json;
    json["name"] = name;
    json["description"] = description;
    json["private"] = private_;
    json["auto_init"] = readme_;

    QNetworkRequest request(QUrl("https://api.github.com/user/repos"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());
    request.setRawHeader("User-Agent", "Qt-GitHub-Client");
    request.setRawHeader("Accept", "application/vnd.github.v3+json");

    QNetworkReply *reply = m_manager->post(request, QJsonDocument(json).toJson());

    connect(reply, &QNetworkReply::finished, [this, reply, name]() {
        handle_response(reply, name);
    });
}

void work_git::delete_repo(const QString &token, const QString &url_repo)
{
    QString base = "https://github.com/";

    int startPos = base.length();
    int endPos = url_repo.indexOf('/', startPos);

    QString owner = url_repo.mid(startPos, endPos - startPos);

    int start_pos_ = base.length()+owner.length();
    int endPos_ = url_repo.indexOf('.', start_pos_);

    QString repo_name = url_repo.mid(start_pos_+1, endPos_);

    if (repo_name.endsWith(".git")) {
        repo_name = repo_name.left(repo_name.length() - 4);
    }

    m_manager = new QNetworkAccessManager(this);

    QUrl url(QString("https://api.github.com/repos/%1/%2").arg(owner, repo_name));


    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization",
                         QString("token %1").arg(token).toUtf8());
    request.setRawHeader("User-Agent", "Qt-GitHub-Client");
    request.setRawHeader("Accept", "application/vnd.github.v3+json");

    QNetworkReply *reply = m_manager->deleteResource(request);

    connect(reply, &QNetworkReply::finished, [this, reply, repo_name]() {
        handle_delete_response(reply, repo_name);
    });
}

void work_git::handle_delete_response(QNetworkReply *reply, const QString &repoName)
{
    if (reply->error() == QNetworkReply::NoError) {
        emit message_signal("Repository(Репозиторий) " + repoName + " deleted successfully(удалено успешно)");
    } else {
        emit message_signal("Error deleting repository(Ошибка удаления репозитория): " + reply->errorString());
    }
    reply->deleteLater();
}

void work_git::handle_response(QNetworkReply *reply, const QString &repoName)
{   
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument response = QJsonDocument::fromJson(reply->readAll());
        QJsonObject json = response.object();

        QString cloneUrl = json["clone_url"].toString();
        QString sshUrl = json["ssh_url"].toString();
        QString fullName = json["full_name"].toString();

        emit message_signal("Repository created successfully(Репозиторий успешно создан) - ✓");
        emit message_signal("HTTPS URL: " + cloneUrl);
    } else {
        QString errorMsg = QString("Error: %1 - %2")
        .arg(reply->error())
            .arg(reply->errorString());

        QByteArray responseData = reply->readAll();
        if (!responseData.isEmpty()) {
            QJsonDocument errorDoc = QJsonDocument::fromJson(responseData);
            if (errorDoc.isObject()) {
                QJsonObject errorObj = errorDoc.object();
                if (errorObj.contains("message")) {
                    errorMsg += "\nGitHub: " + errorObj["message"].toString();
                }
            }
        }
        emit message_signal(errorMsg);
    }

    reply->deleteLater();
}
