#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

char *CONDITIONS[] = {"布", "剪刀", "石头"};    // 对照

struct data {
    struct timeval time;
    int fgguss; // 2: 石头; 1: 剪刀; 0: 布
};

struct gamemsg {
    long int msgtype;
    struct data mydata;
};

struct startmsg {
    long int msgtype;
    int se; // se=1: 开始 se=0: 游戏结束
};

int main() {
    key_t key;
    int player_id;

    pid_t fpid;
    fpid = fork();
    if (fpid < 0) {
        fprintf(stderr, "Error: failed to create child process[player]\n");
        exit(EXIT_FAILURE);
    } else if (fpid == 0) {
        // 子进程 -> 玩家一
        key = ftok("/tmp", 77);
        player_id = msgget(key, IPC_CREAT | 0666);
        srand(1);
    } else {
        // 父进程 -> 玩家二
        key = ftok("/tmp", 88);
        player_id = msgget(key, IPC_CREAT | 0666);
        srand(2);
    }

    if (player_id == -1) {
        fprintf(stderr, "Error: failed to create a message queue[player]\n");
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    struct gamemsg gmsg;
    struct startmsg smsg;

    smsg.msgtype = 1;
    gmsg.msgtype = 2;

    while (1) {
        if (msgrcv(player_id, (void *)&smsg, sizeof(int), 1, 0) < 0) {
            fprintf(stderr, "Error: failed to receive message[player]\n");
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }
        
        if (smsg.se == 0) {
            break;
        }
        // 猜拳并将猜拳内容封装成消息发送
        struct data mydata;

        int finger = rand() % 3;
        gettimeofday(&mydata.time, 0);
        mydata.fgguss = finger;
        gmsg.mydata = mydata;
        
        // 各自的出拳
        if (fpid == 0)
            printf("玩家一：我出%s\n", CONDITIONS[finger]);
        else
            printf("玩家二：我出%s\n", CONDITIONS[finger]);

        if (msgsnd(player_id, (void *)&gmsg, sizeof(mydata), 0) < 0) {
            fprintf(stderr, "Error: failed to send message[player]\n");
            perror("msgsnd");
            exit(EXIT_FAILURE);
        }
    }

    if (msgctl(player_id, IPC_RMID, NULL) < 0) {
        fprintf(stderr, "Error: failed to detete a message queue[player]\n");
        perror("msgctl");
        exit(EXIT_FAILURE);
    }

    return 0;
}