#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <fstream>
#include <iostream>
#include <queue>
#include <stack>
#include <string>
#include <unordered_map>

#include "board.h"

using namespace std;
struct DB_BFS_node {
    Board b;
    int cost;
};
int main() {
    double start = (double)clock();


    unordered_map<string, int> hash_table;  // 開一個hash來存db
    Board start_board;                      //起始盤面b1 存 1 2 3 4 6 7 8 的pattern
    start_board[0][0] = 0;
    start_board[0][1] = 0;
    start_board[0][2] = 0;
    start_board[0][3] = 0;
    start_board[0][4] = 0;
    start_board[1][0] = 0;
    start_board[1][1] = 0;
    start_board[1][2] = 0;
    start_board[1][3] = -1;
    start_board[1][4] = 0;
    start_board[2][0] = -1;
    start_board[2][1] = 0;
    start_board[2][2] = -1;
    start_board[2][3] = 0;
    start_board[2][4] = 0;
    start_board[3][0] = 16;
    start_board[3][1] = 17;
    start_board[3][2] = 18;
    start_board[3][3] = 0;
    start_board[3][4] = 0;

    // DB建構策略 從結束版面回推 bfs所有可能路徑
    queue<DB_BFS_node> bfs_queue;
    struct DB_BFS_node start_node {
        start_board, 0
    };
    // start_node.b = start_board;
    // start_node.cost = 0;
    bfs_queue.push(start_node);

    while (!bfs_queue.empty()) {  // bfs loop到所有狀態走完
        // cout << "size = " << bfs_queue.size() << '\n';
        struct DB_BFS_node now = bfs_queue.front();
        bfs_queue.pop();
        Board b = now.b;

        // 檢查這個版面有沒有來過
        string board_hashkey;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 5; j++) {
                string temp = to_string(b[i][j]);
                board_hashkey.append(temp);
                board_hashkey.append("/");
            }
        }
        if (hash_table.count(board_hashkey)) {  // 曾經來過 所以直接結束掉
            // cout<<"來過"<<'\n';
            continue;
        }
        hash_table[board_hashkey] = now.cost;

        // 找下一個點丟進去
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 5; j++) {
                if (b[i][j] <= 0) continue;
                int position = 5 * i + j;
                if (i < 3 && b[i + 1][j] == 0) {
                    struct DB_BFS_node next = now;
                    next.b.slide(position, Board::Action::down);
                    next.cost++;
                    bfs_queue.push(next);
                }
                if (0 < i && b[i - 1][j] == 0) {
                    struct DB_BFS_node next = now;
                    next.b.slide(position, Board::Action::up);
                    next.cost++;
                    bfs_queue.push(next);
                }
                if (j < 4 && b[i][j + 1] == 0) {
                    struct DB_BFS_node next = now;
                    next.b.slide(position, Board::Action::right);
                    next.cost++;
                    bfs_queue.push(next);
                }
                if (0 < j && b[i][j - 1] == 0) {
                    struct DB_BFS_node next = now;
                    next.b.slide(position, Board::Action::left);
                    next.cost++;
                    bfs_queue.push(next);
                }
            }
        }
    }
    // 開檔案
    ofstream out_data;
    out_data.open("hash_table_b4.txt");
    if (!out_data) {
        cerr << "cannot open hash_table_b1.txt " << '\n';
        return 0;
    }

    // 把hash寫進檔案
    for (auto i : hash_table) {
        out_data << i.first << i.second << '\n';
    }

    // 關檔案
    out_data.close();
    double end = (double)clock();
    // cout<< (end-start) / (double)CLOCKS_PER_SEC;
    return 0;
}