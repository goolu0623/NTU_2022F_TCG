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
int cal_cost_all(Board b);
int cal_cost_update(Board b, int target, Board::Action your_slide);

class as_node {
   public:
    int prev_cost;
    int est_cost;
    Board b;
    queue<pair<int, Board::Action>> move;
    // string name;
};

class cmp {
   public:
    bool operator()(as_node a, as_node b) {
        return a.prev_cost + a.est_cost > b.prev_cost + b.est_cost;
    }
};

// bool operator()(as_node a, as_node b) {
//     return a.prev_cost + a.est_cost < b.prev_cost + b.est_cost;
// }
int main() {
    // double start = (double)clock();
    struct timespec start_time, ans_time, end_time;
    clock_gettime(CLOCK_REALTIME, &start_time);


    //測試用
    // struct as_node aaa, bbb, ccc;
    // aaa.est_cost = 10;
    // aaa.prev_cost = 0;
    // aaa.name = "aaa";
    // bbb.est_cost = 3;
    // bbb.prev_cost = 0;
    // bbb.name = "bbb";
    // ccc.est_cost = 8;
    // ccc.prev_cost = 0;
    // ccc.name = "ccc";
    // priority_queue<as_node, vector<as_node>, cmp> pq2;
    // pq2.push(aaa);
    // pq2.push(bbb);
    // pq2.push(ccc);
    // cout << pq2.top().name;

    // return 0;

    // 先吃inpurt做成版面
    int n, m;
    cin >> n >> m;
    Board question;
    queue<pair<int, Board::Action>> ANS;

    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++) cin >> question[i][j];

    // 計時用
    // double start = (double)clock();
    // double first_find;

    // cout << "start = " << setprecision(10) << start / (double)CLOCKS_PER_SEC << '\n';
    // init
    priority_queue<as_node, vector<as_node>, cmp> pq;  // 給a*以cost最低優先
    unordered_map<string, int> my_hash;
    // cout<<"hash 亂出書" <<my_hash["123"]<<'\n';
    // 把初始盤面塞進priorty queue
    struct as_node start_node {
        0, cal_cost_all(question), question
    };
    pq.push(start_node);

    //
    while (!pq.empty()) {
        // 先看時間過多久 如果快到了就cout答案然後結束
        // double now = (double)clock();
        clock_gettime(CLOCK_REALTIME, &end_time);
        if ((double)(end_time.tv_sec + end_time.tv_nsec * 1e-9) -
                        (double)(start_time.tv_sec + start_time.tv_nsec * 1e-9) > 9.5) {
            int ans_size = ANS.size();
            cout << ans_size << '\n';
            for (int i = 0; i < ans_size; i++) {
                pair<int, Board::Action> temp = ANS.front();
                cout << temp.first << " " << temp.second << '\n';
                ANS.pop();
            }
            // cout << "find time = " << (first_find - start) / (double)CLOCKS_PER_SEC << '\n';
            // cout << "wall clock find time = "
            //      << (double)(ans_time.tv_sec + ans_time.tv_nsec * 1e-9) -
            //             (double)(start_time.tv_sec + start_time.tv_nsec * 1e-9)<<'\n';
            return 0;
        }

        // 取一個點出來處理

        struct as_node now_node;
        now_node = pq.top();
        pq.pop();

        // cerr << now_node.b;
        //檢查現在是不是答案
        if (now_node.b.finish()) {
            if (ANS.size() == 0) {
                // cerr<<"第一組答案"<<'\n';
                ANS = now_node.move;
                // first_find = (double)clock();
                // clock_gettime(CLOCK_REALTIME, &ans_time);
            } else if (ANS.size() > now_node.move.size()) {
                // cerr<<"刷新答案"<<'\n';
                ANS = now_node.move;
                // first_find = (double)clock();
                // clock_gettime(CLOCK_REALTIME, &ans_time);
            } else {
                // cerr << "找到一個答案size = " << now_node.move.size() << '\n';
            }

        } else {
            // 先檢查這個board有沒有進來過 很暴力的直接用版面接成string 當hash key檢查
            string board_hashkey;
            Board now_board = now_node.b;
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 5; j++) {
                    string temp = to_string(now_board[i][j]);
                    board_hashkey.append(temp);
                    board_hashkey.append(" ");
                }
            }

            if (my_hash.count(board_hashkey) != 0 &&
                my_hash[board_hashkey] <= now_node.prev_cost + now_node.est_cost)
                continue;
            else
                my_hash[board_hashkey] = now_node.prev_cost + now_node.est_cost;
            //因為現在不是答案的 繼續掃盤面
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 5; j++) {
                    if (now_board[i][j] == 0) {  // 找到可以被滑動的地方
                        int now_position = 5 * i + j;

                        if (j < 4 && now_board[i][j + 1] > 0) {  //代表右側有方塊 且非-1
                            struct as_node next_node = now_node;
                            next_node.prev_cost++;
                            next_node.b.slide(now_position + 1, Board::Action::left);
                            next_node.move.push(make_pair(now_position + 1, Board::Action::left));
                            next_node.est_cost = cal_cost_all(next_node.b);
                            // next_node.est_cost +=
                                // cal_cost_update(now_node.b, now_position + 1, Board::Action::left);
                            pq.push(next_node);
                        }
                        if (i > 0 && now_board[i - 1][j] > 0) {  //代表上側有方塊 且非-1或0
                            struct as_node next_node = now_node;
                            next_node.prev_cost++;
                            next_node.b.slide(now_position - 5, Board::Action::down);
                            next_node.move.push(make_pair(now_position - 5, Board::Action::down));
                            next_node.est_cost = cal_cost_all(next_node.b);
                            // next_node.est_cost +=
                                // cal_cost_update(now_node.b, now_position - 5, Board::Action::down);
                            pq.push(next_node);
                        }
                        if (i < 3 && now_board[i + 1][j] > 0) {  //代表下側有方塊 且非-1
                            struct as_node next_node = now_node;
                            next_node.prev_cost++;
                            next_node.b.slide(now_position + 5, Board::Action::up);
                            next_node.move.push(make_pair(now_position + 5, Board::Action::up));
                            next_node.est_cost = cal_cost_all(next_node.b);
                            // next_node.est_cost +=
                                // cal_cost_update(now_node.b, now_position + 5, Board::Action::up);
                            pq.push(next_node);
                        }
                        if (j > 0 && now_board[i][j - 1] > 0) {  //代表左側有方塊 且非-1
                            struct as_node next_node = now_node;
                            next_node.prev_cost++;
                            next_node.b.slide(now_position - 1, Board::Action::right);
                            next_node.move.push(make_pair(now_position - 1, Board::Action::right));
                            next_node.est_cost = cal_cost_all(next_node.b);
                            // next_node.est_cost +=
                                // cal_cost_update(now_node.b, now_position - 1, Board::Action::right);
                            pq.push(next_node);
                        }
                    }
                }
            }
        }
    }

    return 0;
}

int cal_cost_all(Board b) {
    int cost = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 5; j++) {
            // cerr<<"i= "<<i <<" ,j= "<<j<<'\n';
            // cerr<<"b[i][j]= "<<b[i][j]<<'\n';
            if (5 * i + j + 1 != b[i][j] && b[i][j] > 0) {
                // cost += abs((5 * i + j + 1) - (b[i][j]));  // manhattan distance
                int ii = (b[i][j] - 1) / 5;
                int jj = (b[i][j] - 1) % 5;
                cost += (abs(ii - i) + abs(jj - j));
            }
        }
    }
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 5; j++) {
            for (int k = j + 1; k < 5; k++) {
                int first = b[i][j], second = b[i][k];
                if (i * 5 + 6 > first && first > second && second > i * 5) {
                    cost += 2;  // linear conflict
                }
            }
        }
    }
    return cost;
}

int cal_cost_update(Board b, int target, Board::Action your_slide) {
    int original_manhattan = 0;
    int new_manhattan = 0;
    int original_linear_conflict = 0;
    int new_linear_conflict = 0;
    int sum = 0;
    int i = target / 5;
    int j = target % 5;
    int ii = (b[i][j] - 1) / 5;
    int jj = (b[j][j] - 1) % 5;
    int new_i = i;
    int new_j = j;
    original_manhattan = abs(ii - i) + abs(jj - j);
    if (your_slide == Board::Action::up)
        new_i -= 1;
    else if (your_slide == Board::Action::down)
        new_i += 1;
    else if (your_slide == Board::Action::left)
        new_j -= 1;
    else if (your_slide == Board::Action::right)
        new_j += 1;
    new_manhattan = abs(ii - new_i) + abs(jj - new_j);
    if (your_slide == Board::Action::left || your_slide == Board::Action::right) {
        sum = -original_manhattan + new_manhattan;
    } else {
        if (i == ii) {
            for (int m = 0; m < 5; m++) {
                if (m == j) continue;
                int first, second;
                if (m < j) {
                    first = b[i][m];
                    second = b[i][j];
                } else {
                    first = b[i][j];
                    second = b[i][m];
                }
                if (i * 5 + 6 > first && first > second && second > i * 5) {
                    original_linear_conflict += 2;
                }
            }
        }
        if (new_i == ii) {
            for (int m = 0; m < 5; m++) {
                if (m == new_j) continue;
                int first, second;
                if (m < new_j) {
                    first = b[i][m];
                    second = b[i][new_j];
                } else {
                    first = b[i][new_j];
                    second = b[i][m];
                }
                if (i * 5 + 6 > first && first > second && second > i * 5) {
                    new_linear_conflict += 2;
                }
            }
        }

        sum = -original_manhattan + new_manhattan - original_linear_conflict + new_linear_conflict;
    }
    return sum;
}