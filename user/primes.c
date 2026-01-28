#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MIN 2
#define MAX 35

static void sieve(int in) {
    int p, n, fd[2], pid = 0;

    if (0 >= read(in, &p, sizeof(p))) return;
    printf("prime %d\n", p);

    while (0 < read(in, &n, sizeof(n))) {
        if (0 == n % p) continue;
        if (!pid && (pipe(fd), !(pid = fork()))) {
            close(fd[1]);
            sieve(fd[0]);
            exit(0);
        }
        if (pid) write(fd[1], &n, sizeof(n));
    }

    if (pid)
        (close(fd[1]), wait(0));
}

int main(void) {
    int fd[2];
    pipe(fd);

    if (!fork()) {
        close(fd[1]);
        sieve(fd[0]);
    } else {
        close(fd[0]);
        for (int i = MIN; i <= MAX; ++i)
            write(fd[1], &i, sizeof(i));
        close(fd[1]);
        wait(0);
    }
}
