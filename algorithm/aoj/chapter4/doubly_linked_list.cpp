#include <bits/stdc++.h>
using namespace std;

// メモ
// ポインタはアドレスしか持てない
// *をつけるとその先のモノを表す

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
