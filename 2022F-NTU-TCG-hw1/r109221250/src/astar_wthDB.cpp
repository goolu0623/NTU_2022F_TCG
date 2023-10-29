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

int cal_cost(Board b);
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

unordered_map<string, int> my_hash;

int main() {
    // double start = (double)clock(), first_find;
    struct timespec start_time, ans_time, end_time;
    clock_gettime(CLOCK_REALTIME, &start_time);

    // 把db heuristics全讀近來
    ifstream b1, b2, b3, b4;
    b1.open("hash_table_b1.txt");
    b2.open("hash_table_b2.txt");
    b3.open("hash_table_b3.txt");
    b4.open("hash_table_b4.txt");
    // if (!b1 || !b2 || !b3 || !b4) {
    //     cerr << "file open error" << '\n';
    // }

    while (b1) {
        string myline;
        getline(b1, myline);
        string key, temp;
        key.assign(myline, 0, 43);
        int value;
        if (myline.size() == 44) {
            temp.assign(myline, 43, 1);
            value = stoi(temp);
        } else if (myline.size() == 45) {
            temp.assign(myline, 43, 2);
            value = stoi(temp);
        } else {
            continue;
        }
        my_hash[key] = value;
        // cout << value << endl;
    }

    while (b2) {
        string myline;
        getline(b2, myline);
        string key, temp;
        key.assign(myline, 0, 46);

        int value;
        if (myline.size() == 47) {
            temp.assign(myline, 46, 1);
            value = stoi(temp);
        } else if (myline.size() == 48) {
            temp.assign(myline, 46, 2);
            value = stoi(temp);
        } else {
            continue;
        }
        my_hash[key] = value;
        // cout << value << endl;
    }
    while (b3) {
        string myline;
        getline(b3, myline);
        string key, temp;
        key.assign(myline, 0, 44);

        int value;
        if (myline.size() == 45) {
            temp.assign(myline, 44, 1);
            value = stoi(temp);
        } else if (myline.size() == 46) {
            temp.assign(myline, 44, 2);
            value = stoi(temp);
        } else {
            continue;
        }
        my_hash[key] = value;
        // cout << value << endl;
    }
    while (b4) {
        string myline;
        getline(b4, myline);
        string key, temp;
        key.assign(myline, 0, 46);

        int value;
        if (myline.size() == 47) {
            temp.assign(myline, 46, 1);
            value = stoi(temp);
        } else if (myline.size() == 48) {
            temp.assign(myline, 46, 2);
            value = stoi(temp);
        } else {
            continue;
        }
        my_hash[key] = value;
        // cout << value << endl;
    }
    b1.close();
    b2.close();
    b3.close();
    b4.close();

    // 開始吃input做成要處理的版面
    int n, m;
    cin >> n >> m;
    Board question;
    queue<pair<int, Board::Action>> ANS;

    // for (int i = 0; i < n; i++)
    //     for (int j = 0; j < m; j++) cin >> question[i][j];

    cin >> question;
    // init
    priority_queue<as_node, vector<as_node>, cmp> pq;  // 給a*以cost最低優先
    unordered_map<string, int> question_hash;
    // 把初始盤面塞進priorty queue
    struct as_node start_node {
        0, cal_cost(question), question
    };
    pq.push(start_node);

    while (!pq.empty()) {
        // 先看時間 快到限時就cout答案然後結束
        // double now = (double)clock();

        clock_gettime(CLOCK_REALTIME, &end_time);
        if ((double)(end_time.tv_sec + end_time.tv_nsec * 1e-9) -
                (double)(start_time.tv_sec + start_time.tv_nsec * 1e-9) >
            9.5) {
            int ans_size = ANS.size();
            cout << ans_size << '\n';
            for (int i = 0; i < ans_size; i++) {
                pair<int, Board::Action> temp = ANS.front();
                cout << temp.first << " " << temp.second << '\n';
                ANS.pop();
            }
            // cout << "cpu find time = " << (first_find - start) / (double)CLOCKS_PER_SEC << '\n';
            // cout << "cpu end time = "<< (now-start)/ (double)CLOCKS_PER_SEC << '\n';
            // cout << "wall clock find time = "
            //      << (double)(ans_time.tv_sec + ans_time.tv_nsec * 1e-9) -
            //             (double)(start_time.tv_sec + start_time.tv_nsec * 1e-9)<<'\n';
            // cout << "wall clock end time = "
            //      << (double)(end_time.tv_sec + end_time.tv_nsec * 1e-9) -
            //             (double)(start_time.tv_sec + start_time.tv_nsec * 1e-9)<<'\n';

            return 0;
        }

        // 取一個點出來處理

        struct as_node now_node;
        now_node = pq.top();
        pq.pop();

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

            if (question_hash.count(board_hashkey) != 0 &&
                question_hash[board_hashkey] <= now_node.prev_cost + now_node.est_cost)
                continue;
            else
                question_hash[board_hashkey] = now_node.prev_cost + now_node.est_cost;
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
                            next_node.est_cost = cal_cost(next_node.b);
                            // next_node.est_cost +=
                            // cal_cost_update(now_node.b, now_position + 1, Board::Action::left);
                            pq.push(next_node);
                        }
                        if (i > 0 && now_board[i - 1][j] > 0) {  //代表上側有方塊 且非-1或0
                            struct as_node next_node = now_node;
                            next_node.prev_cost++;
                            next_node.b.slide(now_position - 5, Board::Action::down);
                            next_node.move.push(make_pair(now_position - 5, Board::Action::down));
                            next_node.est_cost = cal_cost(next_node.b);
                            // next_node.est_cost +=
                            // cal_cost_update(now_node.b, now_position - 5, Board::Action::down);
                            pq.push(next_node);
                        }
                        if (i < 3 && now_board[i + 1][j] > 0) {  //代表下側有方塊 且非-1
                            struct as_node next_node = now_node;
                            next_node.prev_cost++;
                            next_node.b.slide(now_position + 5, Board::Action::up);
                            next_node.move.push(make_pair(now_position + 5, Board::Action::up));
                            next_node.est_cost = cal_cost(next_node.b);
                            // next_node.est_cost +=
                            // cal_cost_update(now_node.b, now_position + 5, Board::Action::up);
                            pq.push(next_node);
                        }
                        if (j > 0 && now_board[i][j - 1] > 0) {  //代表左側有方塊 且非-1
                            struct as_node next_node = now_node;
                            next_node.prev_cost++;
                            next_node.b.slide(now_position - 1, Board::Action::right);
                            next_node.move.push(make_pair(now_position - 1, Board::Action::right));
                            next_node.est_cost = cal_cost(next_node.b);
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

int cal_cost(Board b) {
    string b1, b2, b3, b4;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 5; j++) {
            if (b[i][j] > 0) {
                if (b[i][j] == 1 || b[i][j] == 2 || b[i][j] == 3 || b[i][j] == 4) {
                    b1.append(to_string(b[i][j]));
                    b1.append("/");
                    b2.append("0/");
                    b3.append("0/");
                    b4.append("0/");
                } else if (b[i][j] == 5 || b[i][j] == 10 || b[i][j] == 14 || b[i][j] == 15) {
                    b2.append(to_string(b[i][j]));
                    b1.append("0/");
                    b2.append("/");
                    b3.append("0/");
                    b4.append("0/");
                } else if (b[i][j] == 6 || b[i][j] == 7 || b[i][j] == 8 || b[i][j] == 12) {
                    b3.append(to_string(b[i][j]));
                    b1.append("0/");
                    b2.append("0/");
                    b3.append("/");
                    b4.append("0/");
                } else if (b[i][j] == 16 || b[i][j] == 17 || b[i][j] == 18) {
                    b4.append(to_string(b[i][j]));
                    b1.append("0/");
                    b2.append("0/");
                    b3.append("0/");
                    b4.append("/");
                }
            } else {
                b1.append(to_string(b[i][j]));
                b1.append("/");
                b2.append(to_string(b[i][j]));
                b2.append("/");
                b3.append(to_string(b[i][j]));
                b3.append("/");
                b4.append(to_string(b[i][j]));
                b4.append("/");
            }
        }
    }
    // if (my_hash.count(b1) == 0 || my_hash.count(b2) == 0 || my_hash.count(b3) == 0 ||
    //     my_hash.count(b4) == 0) {
    //     cerr << "hash error" << endl;
    //     if (my_hash.count(b1) == 0) cerr << "b1 = " << b1 << '\n';
    //     if (my_hash.count(b2) == 0) cerr << "b2 = " << b2 << '\n';
    //     if (my_hash.count(b3) == 0) cerr << "b3 = " << b3 << '\n';
    //     if (my_hash.count(b4) == 0) cerr << "b4 = " << b4 << '\n';
    // }
    int sum = my_hash[b1] + my_hash[b2] + my_hash[b3] + my_hash[b4];
    return sum;
}