#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define MATCHES 200
#define SLEEPTIME 200

int gameboard[MATCHES]; // 每局的胜负情况
long int time_p1[MATCHES];   // 玩家一每局所用的时间
long int time_p2[MATCHES];   // 玩家二每局所用的时间

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

void Sleep(int ms)
{
	struct timeval delay;
	delay.tv_sec = 0;
	delay.tv_usec = ms * 1000;
	select(0, NULL, NULL, NULL, &delay);
}

// 用来判断哪一方赢得比赛
// 返回的数 >0: player1赢 =0: 平局 <0: player2赢
int judge(int ans1, int ans2) {
        if (ans1 == 2 && ans2 == 0)
            return -1;
        else if (ans1 == 0 && ans2 == 2)
            return 1;
        return ans1 - ans2;
}

// 计算平均时间
double get_average(long int arr[]) {
    long int ave = 0;
    for (int i = 0; i < MATCHES; ++i)
        ave += arr[i];
    return ave * 1.0 / MATCHES;
}

int main() {
    struct gamemsg gmsg1, gmsg2;
    struct startmsg smsg;
    key_t key1, key2;
    int p1msg_id, p2msg_id;

    key1 = ftok("/tmp", 77);
    key2 = ftok("/tmp", 88);
    p1msg_id = msgget(key1, IPC_CREAT | 0666);
    p2msg_id = msgget(key2, IPC_CREAT | 0666);

    if (p1msg_id == -1 || p2msg_id == -1) {
        fprintf(stderr, "Error: failed to create/connect message queue[judge]\n");
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    smsg.msgtype = 1;

    gmsg1.msgtype = 2;
    gmsg2.msgtype = 2;

    int i = 0;
    int p1_win = 0, p2_win = 0, tie = 0;
    while (i < MATCHES) {
        // 裁判发出开始口令
        smsg.se = 1;
        if (msgsnd(p1msg_id, (void *)&smsg, sizeof(int), 0) < 0) {
            fprintf(stderr, "Error: failed to send a message to player1[judge]\n");
            perror("msgsnd");
            exit(EXIT_FAILURE);
        }
        if (msgsnd(p2msg_id, (void *)&smsg, sizeof(int), 0) < 0) {
            fprintf(stderr, "Error: failed to send a message to player2[judge]\n");
            perror("msgsnd");
            exit(EXIT_FAILURE);
        }

        // 裁判接收两人的出拳情况
        struct data mydata;
        struct timeval endtime;
        if (msgrcv(p1msg_id, (void *)&gmsg1, sizeof(mydata), 2, 0) < 0) {
            fprintf(stderr, "Error: failed to receive a message from player1[judge]\n");
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }
        gettimeofday(&endtime, 0);
        int fgguss1 = gmsg1.mydata.fgguss;
        long int time1 = 1000000*(endtime.tv_sec - gmsg1.mydata.time.tv_sec) + (endtime.tv_usec - gmsg1.mydata.time.tv_usec);
        if (msgrcv(p2msg_id, (void *)&gmsg2, sizeof(mydata), 2, 0) < 0) {
            fprintf(stderr, "Error: failed to receive a message from player2[judge]\n");
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }
        gettimeofday(&endtime, 0);
        int fgguss2 = gmsg2.mydata.fgguss;
        long int time2 = 1000000*(endtime.tv_sec - gmsg2.mydata.time.tv_sec) + (endtime.tv_usec - gmsg2.mydata.time.tv_usec);
        
        // 储存数据
        int judgement = judge(fgguss1, fgguss2);
        gameboard[i] = judgement;
        time_p1[i] = time1;
        time_p2[i] = time2;

        if (judgement > 0)
            p1_win++;
        else if (judgement == 0)
            tie++;
        else
            p2_win++;
        

        // 打印本次出拳结果
        printf("=============== 第%d回合 ===============\n", i+1);
        printf("玩家一：%s\t玩家二：%s\n", CONDITIONS[fgguss1], CONDITIONS[fgguss2]);
        printf("玩家一用时：%-4ldus\t玩家二用时：%-4ldus\n", time1, time2);
        if (judgement > 0)
            printf("玩家一胜\n");
        else if (judgement == 0)
            printf("平局\n");
        else
            printf("玩家二胜\n");
        
        // 本次循环结束
        i++;
        Sleep(SLEEPTIME);
    }

    // 发出比赛结束的口令
    smsg.se = 0;
    if (msgsnd(p1msg_id, (void *)&smsg, sizeof(int), 0) < 0) {
        fprintf(stderr, "Error: failed to send stop message[judge]\n");
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }
    if (msgsnd(p2msg_id, (void *)&smsg, sizeof(int), 0) < 0) {
            fprintf(stderr, "Error: failed to send stop message[judge]\n");
            perror("msgsnd");
            exit(EXIT_FAILURE);
    }

    // 打印最终结果
    printf("#################### 比赛结束 ####################\n");
    printf("玩家一赢%5d局;\t玩家二赢%5d局;\t平局%5d局\n", p1_win, p2_win, tie);
    if (p1_win > p2_win)
        printf("玩家一赢得整场比赛\n");
    else if (p1_win == p2_win)
        printf("平局\n");
    else
        printf("玩家二赢得整场比赛\n");
    
    // 打印统计结果
    printf("\n玩家一的平均时间：%.2f us\t玩家二的平均时间：%.2f us\n", get_average(time_p1), get_average(time_p2));
    return 0;
}