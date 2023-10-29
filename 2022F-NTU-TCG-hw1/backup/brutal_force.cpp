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

#include "./scripts/src/board.h"
using namespace std;

stack<pair<int, Board::Action>> ANS;

unordered_map<string, bool> my_hash;

bool BFS(Board now_term, int depth) {
    if (depth >= 20) {
        return 0;
    }
    // 先檢查現在這個board有沒有進來過 很暴力的直接用版面接成string 當hash key檢查
    string term_hash;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 5; j++) {
            string temp = to_string(now_term[i][j]);
            term_hash.append(temp);
            term_hash.append(" ");
        }
    }
    
    if (my_hash[term_hash]) {
        cerr<<"term_hash = "<< term_hash;
        cerr << " hash found" << '\n';
        return 0;
    } else {
        cerr<<"term_hash = "<< term_hash;
        cerr << " not found" << '\n';
        my_hash[term_hash] = true;
    }

    queue<pair<int, Board::Action>> TODO_queue;
    // 先把版面上能動的條件看一輪 然後能動的通通塞進queue
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 5; j++) {
            if (now_term[i][j] == 0) {
                int now_position = 5 * i + j;
                // cerr << "now_position = " << now_position << '\n';
                // 0,1,2,3 == up, down, left, right; 根據board.h的定義
                if (j < 4 && now_term[i][j + 1] > 0) {  //代表右側有方塊 且非-1
                    TODO_queue.push(make_pair(now_position + 1, Board::Action::left));
                }
                if (i > 0 && now_term[i - 1][j] > 0) {  //代表上側有方塊 且非-1或0
                    TODO_queue.push(make_pair(now_position - 5, Board::Action::down));
                }
                if (i < 3 && now_term[i + 1][j] > 0) {  //代表下側有方塊 且非-1
                    TODO_queue.push(make_pair(now_position + 5, Board::Action::up));
                }
                if (j > 0 && now_term[i][j - 1] > 0) {  //代表左側有方塊 且非-1
                    TODO_queue.push(make_pair(now_position - 1, Board::Action::right));
                }
            }
        }
    }
    // 針對queue裡面能動作的全部跑一輪下去BFS
    int size_of_queue = TODO_queue.size();
    for (int i = 0; i < size_of_queue; i++) {
        pair<int, Board::Action> next_move = TODO_queue.front();
        TODO_queue.pop();
        Board next_term = now_term;
        bool find_ans = false;
        // cerr << "now test slide from " << next_move.first << " and move " << next_move.second
            //  << '\n';
        // cerr << now_term << '\n';
        next_term.slide(next_move.first, next_move.second);  //能動的滑看看
        if (next_term.finish()) {
            ANS.push(next_move);
            return 1;
        } else {
            find_ans = BFS(next_term, depth + 1);
        }

        if (find_ans) {
            ANS.push(next_move);
            return 1;
        }
    }

    return 0;
}

int main() {
    int n, m;
    cin >> n >> m;
    Board question;
    for (int i = 0; i < n; i++) {
        vector<int> temp_vec;
        for (int j = 0; j < m; j++) {
            int temp_int;
            cin >> question[i][j];
            // question[i][j] = temp_int;
        }
    }
    // cout<<1;
    bool find_ans = BFS(question, 0);
    if (find_ans) {
        int ans_size= ANS.size();
        cout << ans_size << '\n';

        for (int i = 0; i < ans_size; i++) {
            pair<int, Board::Action> now = ANS.top();
            ANS.pop();
            cout << now.first <<" "<< now.second<<'\n';
        }
    } else {
        cerr << "not found ANS" << '\n';
    }

    return 0;
}
