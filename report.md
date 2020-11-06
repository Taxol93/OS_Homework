# 操作系统实验报告

## 实验步骤1的数据结构

### 1. 玩家猜拳消息

```c
char *CONDITIONS[] = {"布", "剪刀", "石头"};

struct data {
    struct timeval time;    // 存放出拳的时间
    int fgguss; // 2: 石头; 1: 剪刀; 0: 布
};

struct gamemsg {
    long int msgtype;   // 消息类型
    struct data mydata;     // 消息内容
};
```

- `gamemsg` 存放了消息。其中`msgtype`说明了消息的类型，`mydata`作为一个结构体则存放了消息的具体内容。
- `data`结构体中有一个时间变量`time`用来存放玩家出拳时的时间，以便后面判断出拳是否超时。另一个int型变量`fgguss`存放了玩家的具体出拳。规则如下：
  - 2 - 石头
  - 1 - 剪刀
  - 0 - 布
- `CONDITIONS`用来在输出时将数字转化成文字

### 2. 裁判的开始命令

```c
struct startmsg {
    long int msgtype;
    int se; // se=1: 开始 se=0: 游戏结束
};
```

- `startmsg`开始命令消息
  - `msgtype`存放了说明了消息类型
  - `se`是一个int型变量，1代表开始 0代表结束

## 实验步骤1的大小规则

### 1. 判断函数代码

```c
// 用来判断哪一方赢得比赛
// 返回的数 >0: player1赢; =0: 平局; <0: player2赢
int judge(int ans1, int ans2) {
        if (ans1 == 2 && ans2 == 0)
            return -1;
        else if (ans1 == 0 && ans2 == 2)
            return 1;
        return ans1 - ans2;
}
```

- 先判断特殊情况（石头，布）和（布，石头），返回相应的结果。其余利用数的特性，直接返回相减之后的结果
- 返回值说明：返回一个大于0的数-玩家一赢；返回一个等于0的数-玩家二赢；返回一个小于0的数-玩家二赢