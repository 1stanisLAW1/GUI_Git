#include "work_git.h"
#include <git2.h>
#include <git2/remote.h>
#include <git2/credential.h>
#include "qdebug.h"
#include "qlineedit.h"
#include <QInputDialog>
#include <QTimer>

#include <QDir>


work_git::work_git(QObject *parent)
    : QObject{parent}
{
   // qDebug() << "Libgit2 version:" << LIBGIT2_VER_MAJOR << LIBGIT2_VER_MINOR << LIBGIT2_VER_REVISION;
}

void work_git::clone_repo(QStringList list)
{
    git_libgit2_init();
    QString url = list.at(0);
    int last_Slash_Pos = url.lastIndexOf('/');  // Находим последний слеш
    QString last_part = url.mid(last_Slash_Pos+1);

    QString path = list.at(1)+last_part+"/";

    git_repository *repo = NULL;
    git_clone_options clone_opts;
    git_clone_options_init(&clone_opts, GIT_CLONE_OPTIONS_VERSION);
    emit message_signal("the process of cloning...");;

    int error = git_clone(&repo, url.toUtf8().constData(), path.toUtf8().constData(), &clone_opts);
    if (error != 0) {
        const git_error *e = giterr_last();
        QString str = QString("Error %1/%2: %3\n").arg(error).arg(e->klass).arg(e->message);
        emit message_signal("Error cloning - ✗");
        emit message_signal(str);
    }else{
        emit message_signal("cloning - ✓");
    }

    git_libgit2_shutdown();
}

void work_git::check_direct(QString path)
{
    if(path == ""){
        emit message_signal("Choose a path");
        return;
    }
    QDir dir(path);
    QStringList files = dir.entryList(QDir::Files|QDir::NoDotAndDotDot);

    for(QString &file :files){
        emit message_signal(file);
    }
}

void work_git::check_push(QStringList list, int num)
{
    emit message_signal("Data verification...");
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
            emit message_signal("Error: Failed to create directory");
            git_libgit2_shutdown();
            return;
        }
    }

    if (remoteUrl.isEmpty()||repoPath.isEmpty()||commitMsg.isEmpty()||branchName.isEmpty()||refspecBytes.isEmpty()) {
        emit message_signal("Error: All fields must be filled out.");
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
            emit message_signal("Authentication cancelled");
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
            printf("Branch %s successfully created\n", branch_name);
        } else {
            const git_error *e = git_error_last();
            printf("Error creating branch: %s\n", e->message);
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
            emit message_signal(QString("Successfully pushed to %1 - ✓").arg(remoteUrl));
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
        emit message_signal(QString("Error opening the repository: %1")
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
        emit message_signal(QString("Error checking status: %1")
                                .arg(e ? e->message : "Unknown error"));
        git_repository_free(repo);
        git_libgit2_shutdown();
        return;
    }

    size_t entrycount = git_status_list_entrycount(status);
    if (entrycount == 0) {
        emit message_signal("No changes to commit");
        git_status_list_free(status);
        git_repository_free(repo);
        git_libgit2_shutdown();
        return;
    }
    git_status_list_free(status);

    if ((error = git_signature_now(&sig, "Your Name", "your.email@example.com")) != GIT_OK) {
        emit message_signal("Error creating signature");
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
        emit message_signal(QString("Error preparing the commit: %1")
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
        emit message_signal(QString("Error getting parent commit: %1")
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
        emit message_signal(QString("Error creating commit: %1")
                                .arg(e ? e->message : "Unknown error"));
        git_tree_free(tree);
        git_signature_free(sig);
        git_repository_free(repo);
        git_libgit2_shutdown();
        return;
    }

    // Setting up URL with token
    if (token.isEmpty()) {
        emit message_signal("Authentication cancelled");
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
            emit message_signal(QString("Remote configuration error: %1")
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
        emit message_signal(QString("Failed to set auth URL: %1").arg(e ? e->message : "Unknown"));
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
                    emit message_signal(QString("Failed to create local branch: %1").arg(e->message));
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
                emit message_signal(QString("Failed to checkout branch: %1").arg(e->message));
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
    QString refspec_str = remote_branch_exists
                              ? QString("+refs/heads/%1:refs/heads/%1").arg(branch_name)  // Forced push
                              : QString("refs/heads/%1:refs/heads/%1").arg(branch_name);  //  Regular push

    // We are performing a push
    git_push_options push_opts;
    git_push_options_init(&push_opts, GIT_PUSH_OPTIONS_VERSION);

    push_opts.callbacks.credentials = [](git_cred **out, const char *url, const char *username_from_url,
                                         unsigned int allowed_types, void *payload) -> int {
        QString *token = static_cast<QString*>(payload);
        QString name = *static_cast<QString*>(payload);
        return git_cred_userpass_plaintext_new(out, name.toUtf8().constData(), token->toUtf8().constData());
    };
    push_opts.callbacks.payload = &token;
    push_opts.callbacks.payload = &name;

    QByteArray refspecBytes = refspec_str.toUtf8();
    git_strarray refspecs = { (char**)&refspec, 1 };

    emit message_signal("The process of pushing...");

    int push_error = git_remote_push(remote, &refspecs, &push_opts);
    if (push_error != GIT_OK) {
        const git_error* e = git_error_last();
        emit message_signal(QString("Push failed: %1").arg(e ? e->message : "Unknown error"));

        // Let's try to do a pull before the repeated push.
        if (remote_branch_exists) {
            emit message_signal("Attempting to pull changes first...");

            git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;
            fetch_opts.callbacks.credentials = push_opts.callbacks.credentials;
            fetch_opts.callbacks.payload = push_opts.callbacks.payload;

            if (git_remote_fetch(remote, NULL, &fetch_opts, "fetch") == GIT_OK) {
                // After a successful fetch, we try to push again.
                push_error = git_remote_push(remote, &refspecs, &push_opts);
                if (push_error != GIT_OK) {
                    const git_error* e = git_error_last();
                    emit message_signal(QString("Push still failed after fetch: %1").arg(e->message));
                } else {
                    emit message_signal("Successfully pushed after fetch - ✓");
                }
            }
        }
    } else {
        emit message_signal(QString("Successfully pushed to branch %1 - ✓").arg(branch_name));
    }

    // Resource Cleanup
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
