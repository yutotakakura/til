/* 
連結リスト

以下の命令を受け付ける双方向連結リストを実装する。
insert x : 連結リストの先頭にキーxを持つ要素を継ぎ足す
delete x : キーxを持つ最初の要素を連結リストから削除する。そのような要素が存在しない場合は、何もしない。
deleteFirst : 連結リストの先頭の要素を削除する
deleteLast : 連結リストの末尾の要素を削除する

入力
n
command1
command2
command3
...
commandn

出力
全ての命令が終了した後の、連結リスト内のキーを順番に出力する。キーは1つの空白文字で区切る。

制約
命令数 <= 2,000,000
delete x の回数 <= 20
0 <= キーの値 <= 10^9
命令の過程でのリストの要素数 <= 10^6
dekete, deleteFirst, deleteLastが与えられるとき、リストには1つ以上の要素が存在する
*/
    
#include <bits/stdc++.h>
using namespace std;

/* 
メモ
ポインタはアドレスしか持てない
*をつけるとその先のモノを表す
 */

struct Node {
    int key;
    Node *next, *prev;
};

Node *nil;

Node* listSearch(int key) {
    Node *cur = nil->next; // 番兵の次の要素から辿る
    while (cur != nil && cur->key != key) {
        cur = cur->next;
   }
    return cur;
}

void init() {
    nil = (Node *)malloc(sizeof(Node));
    nil->next = nil;
    nil->prev = nil;
}

void printList() {
    Node *cur = nil->next;
    int isf = 0;
    while (1) {
        if (cur == nil) break;
        if (isf++ > 0) printf(" ");
        printf("%d", cur->key);
        cur = cur->next;
    }
    printf("\n");
}

void deleteNode(Node *t) {
    if (t == nil) return; // tが番兵のときは処理しない
    t->prev->next = t->next;
    t->next->prev = t->prev;
    free(t);
}

void deleteFirst() {
    deleteNode(nil->next);
}

void deleteLast() {
    deleteNode(nil->prev);
}

void deleteKey(int key) {
    // 検索したノードを削除
    deleteNode(listSearch(key));
}

void insert(int key) {
    Node *x = (Node *)malloc(sizeof(Node));
    x->key = key;
    // 番兵の直後に要素を追加
    x->next = nil->next;
    nil->next->prev = x;
    nil->next = x;
    x->prev = nil;
}

int main() {
    int key, n, i;
    int size = 0;
    char com[20];
    int np = 0, nd = 0;
    scanf("%d", &n);
    init();
    for(i = 0; i < n; i++) {
        scanf("%s%d", com, &key);
        // ToDo : 続きの実装
    }
}
