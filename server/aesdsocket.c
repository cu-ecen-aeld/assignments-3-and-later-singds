#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

#define OUT_FNAME "/var/tmp/aesdsocketdata"

bool closeAll = false;

void signalHandler(int signal)
{
    if (signal == SIGINT || signal == SIGTERM)
    {
        closeAll = true;
    }
}

int main(int argc, char *argv[])
{
    int socketfd;
    int socketCurr;
    struct addrinfo addrinfo = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
        .ai_protocol = 0,
        .ai_flags = AI_PASSIVE,
    };
    struct addrinfo *retAddrInfo;
    struct sockaddr_in addrClient;
    socklen_t addrClientLen;
    int ret;
    int so_reuseaddr = 1;
    struct sigaction sigintAct = {
        .sa_handler = signalHandler,
    };
    struct sigaction sigtermAct = {
        .sa_handler = signalHandler,
    };
    bool deamonMode = false;

    if (argc >= 2) {
        if (!strcmp(argv[1],"-d")) {
            deamonMode = true;
        }
    }

    sigaction(SIGINT, &sigintAct, NULL);
    sigaction(SIGTERM, &sigtermAct, NULL);

    openlog(NULL, 0, LOG_USER);
    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("socket error\n");
        exit(1);
    }
    setsockopt(socketfd, IPPROTO_TCP, SO_REUSEADDR, &so_reuseaddr, sizeof(int));
    if (getaddrinfo(NULL, "9000", &addrinfo, &retAddrInfo) != 0) {
        printf("getaddrinfo error\n");
        exit(1);
    }
    if (bind(socketfd, retAddrInfo->ai_addr, retAddrInfo->ai_addrlen) != 0) {
        printf("bind error\n");
        exit(1);
    }
    freeaddrinfo(retAddrInfo);
    if (fork() != 0) {
        // I'm the parent
        // exit without errors
        exit(0);
    }

    if (listen(socketfd, 5) != 0) {
        printf("listen error\n");
        exit(1);
    }
    printf("socket listen done\n");



    while (!closeAll)
    {
        addrClientLen = sizeof(struct sockaddr_in);
        socketCurr = accept(socketfd, (void *)&addrClient, &addrClientLen);
        if (socketCurr < 0) {
            if (closeAll)
                continue;
            printf("accept error\n");
            exit(1);
        } else {
            printf("accepted\n");
            char clientAddr[20];
            FILE *f;
            char *buff = NULL;
            int buffSz, buffLen;
            int rcvCnt;
            
            snprintf(clientAddr, sizeof(clientAddr), "%d.%d.%d.%d",
                (addrClient.sin_addr.s_addr >> 0 ) & 0xff,
                (addrClient.sin_addr.s_addr >> 8 ) & 0xff,
                (addrClient.sin_addr.s_addr >> 16) & 0xff,
                (addrClient.sin_addr.s_addr >> 24) & 0xff
            );
            syslog(LOG_DEBUG, "Accepted connection from %s", clientAddr);


            while(!closeAll)
            {
                if (buff == NULL) {
                    buffLen = 0;
                    buffSz = 0;
                }
                if (buffLen == buffSz) {
                    if (buffSz == 0) {
                        buffSz = 100;
                    } else {
                        buffSz *= 2;
                    }
                    buff = realloc(buff, buffSz);
                }
                
                rcvCnt = recv(socketCurr, buff + buffLen, 1, 0);
                if (rcvCnt == -1 || rcvCnt == 0) {
                    printf("recv %d error: %s\n", rcvCnt, strerror(errno));
                    syslog(LOG_DEBUG, "Closed connection from %s", clientAddr);
                    break;
                } else if (rcvCnt == 1) {
                    buffLen++;
                    if (buff[buffLen - 1] == '\n') {
                        int c;

                        printf("packet recieved\n");
                        write(1, buff, buffLen);

                        f = fopen(OUT_FNAME, "ab");
                        if (f == NULL) {
                            printf("fopen error\n");
                            exit(1);
                        }
                        fwrite(buff, 1, buffLen, f);
                        fclose(f);

                        free(buff);
                        buff = NULL;

                        f = fopen(OUT_FNAME, "rb");
                        if (f == NULL) {
                            printf("fopen error\n");
                            exit(1);
                        }
                        while (1) {
                            char cc;
                            c = fgetc(f);
                            if (c == EOF)
                                break;
                            cc = c;
                            send(socketCurr, &cc, 1, 0);
                        }
                        fclose(f);
                    }
                }
            }
            if (buff) {
                free(buff);
                buff = NULL;
            }
            close(socketCurr);
        }
    }
    close(socketfd);
    remove(OUT_FNAME);
    printf("closing all\n");
    syslog(LOG_DEBUG, "Caught signal, exiting");
}
