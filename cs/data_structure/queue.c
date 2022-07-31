/*

【キュー】

名前nameと処理時間timeをもつn個のプロセスがある。
ラウンドロビンスケジュールをシミュレートせよ。
クオンタムをq[ms]とする。

【入力】
n q
name1 time1
name2 time2
name3 time3
...
namei timei

【出力】
プロセスが完了した順に、各プロセスの名前と終了時刻を空白で区切って一行に出力する。

【制約】
1 <= n <= 100,000
1 <= q <= 1,000
1 <= time <= 50,000
1 <= 文字列nameの長さ <= 10
1 <= timeの合計 <= 1,000,000

【計算量】
リングバッファを用いれば、enqueueもdequeueも O(1)

*/

#include<stdio.h>
#include<string.h>
#define LEN 100005

typedef struct pp {
    char name[100];
    int t;
} P;

P Q[LEN];
int head, tail, n;

void enqueue(P x) {
    Q[tail]  = x;
    tail = (tail + 1) % LEN;
}

P dequeue() {
    P x = Q[head];
    head = (head + 1) % LEN;
    return x;
}

int min(int a, int b) { return a < b ? a : b; } // 最小値を返す

int main() {
    int elaps = 0, c;
    int i, q;
    P u;
    scanf("%d %d", &n, &q);

    // 全てのプロセスをキューに順番に追加する
    for (i = 1; i <= n; i++) {
        scanf("%s", Q[i].name);
        scanf("%d", &Q[i].t);
    }
    head = 1; tail = n + 1;

    // シミュレーション
    while (head != tail) {
        u = dequeue();
        c  = min(q, u.t);
        u.t -= c;
        elaps += c;
        if (u.t > 0) enqueue(u);
        else {
            printf("%s %d\n", u.name, elaps);
        }
    }

    return 0;
}