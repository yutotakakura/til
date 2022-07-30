/*

【スタック】

逆ポーランド記法で与えられた数式の計算結果を、一行で出力する。
入力は一つの空白を持って与えられる。

【制約】
2 <= 式に含まれるオペランドの数 <= 100
1 <= 式に含まれる演算子の数 <= 99
演算子は+, -, *のみ。一つのオペランドは10^6以下の正の整数
-1 * 10^9 <= 計算途中の値 <= 10^9

【入力例】
1 2 + 3 4 - *

【出力例】
-3

【計算量】
pop も push も O(1)

*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int top, S[1000];

void push(int x) {
    // topを加算してからその位置へ挿入
    S[++top] = x;
}

int pop() {
    top--;
    // topが指していた要素を返す
    return S[top+1];
}

int main() {
    int a, b;
    top = 0;
    char s[100];
    while(scanf("%s", s) != EOF) {
        if (S[0] == '+') {
            a = pop();
            b = pop();
            push(a + b);
        } else if (S[0] == '-') {
            b = pop();
            a = pop();
            push(a - b);
        } else if (S[0] == '*') {
            a = pop();
            b = pop();
            push(a * b);
        } else {
            push(atoi(s));
        }
    }

    printf("%d\n", pop());

    return 0;
}