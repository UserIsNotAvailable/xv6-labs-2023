#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define PING "ping"
#define PONG "pong"
#define LEN 5

int main(void) {
    int pipefd[2];
    if (0 > pipe(pipefd)) {
        fprintf(2, "pipe failed\n");
        exit(1);
    }

    char buf[LEN];
    int pid = fork();

    if (-1 == pid) {
        fprintf(2, "fork failed\n");
        exit(1);
    } else if (0 == pid) {
        if (LEN != write(pipefd[1], PING, LEN)) {
            fprintf(2, "child write failed\n");
            exit(1);
        }

        if (LEN != read(pipefd[0], buf, LEN)) {
            fprintf(2, "child read failed\n");
            exit(1);
        }
        printf("%d: received %s\n", getpid(), buf);


        if (0 > close(pipefd[0])) {
            fprintf(2, "child close pipefd[0] failed\n");
            exit(1);
        }

        if (0 > close(pipefd[1])) {
            fprintf(2, "child close pipefd[1] failed\n");
            exit(1);
        }
    } else {
        if (LEN != read(pipefd[0], buf, LEN)) {
            fprintf(2, "parent read failed\n");
            exit(1);
        }
        printf("%d: received %s\n", getpid(), buf);
        if (LEN != write(pipefd[1], PONG, LEN)) {
            fprintf(2, "parent write failed\n");
            exit(1);
        }

        if (0 > close(pipefd[0])) {
            fprintf(2, "parent close pipefd[0] failed\n");
            exit(1);
        }

        if (0 > close(pipefd[1])) {
            fprintf(2, "parent close pipefd[1] failed\n");
            exit(1);
        }
        if (0 > wait(0)) {
            fprintf(2, "parent wait failed\n");
            exit(1);
        }
    }

    exit(0);
}
