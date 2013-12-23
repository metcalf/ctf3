#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "git2.h"

#include "common.h"
#include "naive.h"

git_repository *repo = NULL;

char volatile stop = 0;
char volatile updateStop = 0;
char volatile updated = 0;

// Copied from libgit2 examples
void check_lg2(int error, const char *message, const char *extra)
{
    const git_error *lg2err;
    const char *lg2msg = "", *lg2spacer = "";

    if (!error)
        return;

    if ((lg2err = giterr_last()) != NULL && lg2err->message != NULL) {
        lg2msg = lg2err->message;
        lg2spacer = " - ";
    }

    if (extra)
        fprintf(stderr, "%s '%s' [%d]%s%s\n",
                message, extra, error, lg2spacer, lg2msg);
    else
        fprintf(stderr, "%s [%d]%s%s\n",
                message, error, lg2spacer, lg2msg);

    exit(1);
}

void reset_hard(){
    git_object *remote_commit;
    git_reference *remote_head;

    check_lg2(git_reference_lookup(&remote_head, repo, "refs/remotes/origin/master"),
              "Could not lookup master branch", NULL);

    check_lg2(git_reference_peel((git_object**)&remote_commit, remote_head, GIT_OBJ_COMMIT),
              "Could not peel remote commit", NULL);
    check_lg2(git_reset(repo, (git_object *)remote_commit, GIT_RESET_HARD),
              "Could not reset to remote head", NULL);

    git_object_free(remote_commit);
    git_reference_free(remote_head);
}

int prepare_index(git_index *index, char* msg){
    git_oid index_tree;
    git_tree *head_obj;
    git_reference *head;
    char tree_str[GIT_OID_HEXSZ+1], parent_str[GIT_OID_HEXSZ+1];

    reset_hard();

    // Get the head OID
    git_repository_head(&head, repo);
    git_reference_peel((git_object**)&head_obj, head, GIT_OBJ_COMMIT);
    git_oid_tostr(parent_str, GIT_OID_HEXSZ+1,
                   git_tree_id(head_obj));

    git_reference_free(head);
    git_object_free((git_object*)head_obj);

    // Write the coin
    // TODO: Use C for this?
    system("perl -i -pe 's/(user-nivak8lr: )(\\d+)/$1 . ($2+1)/e' LEDGER.txt");
    system("grep -q \"user-nivak8lr\" LEDGER.txt || echo \"user-nivak8lr: 1\" >> LEDGER.txt");

    // Update the index
    check_lg2(git_index_read(index, 0),
              "Could not re-read index from disk", NULL);

    check_lg2(git_index_add_bypath(index, "LEDGER.txt"),
              "Could not add to index", "LEDGER.txt");

    // Write the index and get the tree OID
    git_index_write_tree(&index_tree, index);
    git_oid_tostr(tree_str, GIT_OID_HEXSZ+1, &index_tree);

    snprintf(msg, COMMIT_LENGTH+1,
             "commit %d%c"
             "tree %s\n"
             "parent %s\n"
             "author Andrew Metcalf <andrew@stripe.com> %d +0000\n"
             "committer Andrew Metcalf <andrew@stripe.com> %d +0000\n"
             "\n"
             "Brute Force!"
             "\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01"
             "\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01"
             "\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01"
             "\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01",
             MSG_LENGTH, 0, tree_str, parent_str,
             (int)time(NULL), (int)time(NULL));

    return 0;
}

int check_updates(){
    git_reference *head, *remote_head;
    int retval;
    FILE *fp;

    fp = popen("git fetch -q 2>/dev/null", "r");

    if(pclose(fp) == -1){
        printf("Error closing fetch pipe");
        exit(1);
    }

    git_repository_head(&head, repo);

    check_lg2(git_reference_lookup(&remote_head, repo, "refs/remotes/origin/master"),
              "Could not lookup master branch", NULL);

    // Differing,
    if(git_reference_cmp(head, remote_head)){
        printf("Update detected\n");
        reset_hard();

        retval = 1;
    } else {
        retval = 0;
    }

    git_reference_free(head);
    git_reference_free(remote_head);

    return retval;
}

void* check_updates_worker(void* arg){
    while(!updateStop){
        if(check_updates() && !updateStop){
            updated = 1;
        }
    }
    pthread_exit(NULL);
}

void int_handler(int sig){
    // Not entirely threadsafe
    stop = 1;
    updated = 1;
}

int push_result(char* msg){
    git_odb *odb;
    git_oid oid;
    FILE *fp;
    char cmd[128], commit_str[GIT_OID_HEXSZ+1];

    check_lg2(git_repository_odb(&odb, repo),
              "Could not allocate odb poiner", NULL);

    check_lg2(git_odb_write(&oid, odb, &msg[11], MSG_LENGTH, GIT_OBJ_COMMIT),
              "Error writing commit", NULL);

    git_odb_free(odb);

    git_oid_tostr(commit_str, GIT_OID_HEXSZ+1, &oid);
    snprintf(cmd, 128, "git push origin %s:master", commit_str);

    printf("%s\n", cmd);

    return system(cmd);
}

void init_args(hash_args *args){
    FILE *fp;
    fp = fopen("difficulty.txt", "r");
    fscanf(fp, "%u", &args->difficulty);


    if(args->difficulty % 2){
        printf("Difficulty is not a multiple of 2: %u\n", args->difficulty);
        exit(1);
    } else {
        printf("Difficulty is %u\n", args->difficulty);
    }

    args->difficulty = args->difficulty / 2;
    args->msg = malloc(MSG_LENGTH+1);
    args->stop = &updated;
}

void init_git(git_index **index){
    git_threads_init();

    check_lg2(git_repository_open(&repo, "."),
              "No git repository", NULL);
    check_lg2(git_repository_index(index, repo),
              "Could not open repository index", NULL);
}

int main () {
    git_index *index = NULL;
    pthread_t updateThread, hashThread;
    int rc;
    void *status;
    hash_args args;

    init_args(&args);
    init_git(&index);

    check_updates();

    signal (SIGINT, int_handler);

    while(!stop){
        printf("Preparing index\n");

        prepare_index(index, args.msg);

        updateStop = 0;
        updated = 0;
        args.found = 0;

        printf("Starting update thread\n");
        rc = pthread_create(&updateThread, NULL, check_updates_worker, NULL);
        if (rc){
            printf("ERROR creating update thread %d\n", rc);
            exit(-1);
        }

        printf("Starting brute force thread\n");
        rc = pthread_create(&hashThread, NULL, force_hash, &args);
        if (rc){
            printf("ERROR creating hash thread %d\n", rc);
            stop = 1;
        } else {
            pthread_join(hashThread, &status);
        }

        updateStop = 1;

        if(!stop && !updated && args.found){
            if(push_result(args.msg)){
                printf("Found one but the push failed\n");
            } else {
                printf("Earned a coin!\n");
            }
        }

        pthread_join(updateThread, &status);
    }

    free(args.msg);

    git_index_free(index);
    git_repository_free(repo);

    git_threads_shutdown();

    pthread_exit(NULL);

    return 0;
}
