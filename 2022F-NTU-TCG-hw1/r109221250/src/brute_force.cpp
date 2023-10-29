#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

#include "board.h"
using namespace std;
struct BFS_node {
    Board b;
    queue<pair<int, Board::Action>> move;
};

int main() {
    // 先吃inpurt做成版面
    int n, m;
    cin >> n >> m;
    Board question;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++) cin >> question[i][j];

    // 先給起始盤面
    struct BFS_node start_node;
    start_node.b = question;

    queue<BFS_node> bfs_queue;            // bfs的queue
    unordered_map<string, bool> my_hash;  //用來檢驗bfs盤面有沒有走過
    bfs_queue.push(start_node);           // bfs進入的第一個點

    while (!bfs_queue.empty()) {
        // 取一個bfs點出來處理
        struct BFS_node now_node = bfs_queue.front();
        bfs_queue.pop();

        // 先檢查現在這個board有沒有進來過 很暴力的直接用版面接成string 當hash key檢查
        string board_hashkey;
        Board now_board = now_node.b;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 5; j++) {
                string temp = to_string(now_board[i][j]);
                board_hashkey.append(temp);
                board_hashkey.append(" ");
            }
        }

        if (my_hash[board_hashkey])
            continue;
        else
            my_hash[board_hashkey] = true;

        if (now_board.finish()) {
            // 找到答案 把答案吐出來
            int cnt = 0;
            int stack_size = now_node.move.size();
            cout << stack_size << '\n';
            for (int i = 0; i < stack_size; i++) {
                pair<int, Board::Action> temp;
                temp = now_node.move.front();
                cout << temp.first;
                cout << " ";
                cout << temp.second;
                cout << "\n";
                now_node.move.pop();
            }
            break;
        } else {
            // 這個盤面未完成, 掃這個盤面的bfs點

            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 5; j++) {
                    if (now_board[i][j] == 0) {  // 找到可以被滑動的地方
                        int now_position = 5 * i + j;

                        if (j < 4 && now_board[i][j + 1] > 0) {  //代表右側有方塊 且非-1
                            struct BFS_node next_node{now_board,now_node.move};
                            next_node.b.slide(now_position + 1, Board::Action::left);
                            next_node.move.push(make_pair(now_position + 1, Board::Action::left));
                            bfs_queue.push(next_node);
                        }
                        if (i > 0 && now_board[i - 1][j] > 0) {  //代表上側有方塊 且非-1或0
                            struct BFS_node next_node{now_board,now_node.move};
                            next_node.b.slide(now_position - 5, Board::Action::down);
                            next_node.move.push(make_pair(now_position - 5, Board::Action::down));
                            bfs_queue.push(next_node);
                        }
                        if (i < 3 && now_board[i + 1][j] > 0) {  //代表下側有方塊 且非-1
                            struct BFS_node next_node{now_board,now_node.move};
                            next_node.b.slide(now_position + 5, Board::Action::up);
                            next_node.move.push(make_pair(now_position + 5, Board::Action::up));
                            bfs_queue.push(next_node);
                        }
                        if (j > 0 && now_board[i][j - 1] > 0) {  //代表左側有方塊 且非-1
                            struct BFS_node next_node{now_board,now_node.move};
                            next_node.b.slide(now_position - 1, Board::Action::right);
                            next_node.move.push(make_pair(now_position - 1, Board::Action::right));
                            bfs_queue.push(next_node);
                        }
                    }
                }
            }
        }
    }

    return 0;

    // 0. while queue還有東西
    // 1. 先操作這個步驟(還是我直接傳操作完的版面給他?)
    // 2. hash看看有沒有走過盤面 直接暴力串string當hash key檢查
    // 3. 盤面完成了?
    // 3.1 完成的話把當下的點 一路*prev回傳回去 然後因為bfs 所以是最佳解
    // 3.2 沒完成的話 掃bfs點
}
