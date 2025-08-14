#include "error_handling.h"
#include <git2.h>
#include <git2/remote.h>
#include <git2/credential.h>

Error_Handling::Error_Handling(QObject *parent)
    : QObject{parent}
{

}

bool Error_Handling::error_check(int error)
{
    git_repository* repo = nullptr;
    if (error != GIT_OK || !repo) {
        const git_error* e = git_error_last();
        git_libgit2_shutdown();
        return false;
    }
    return true;
}
