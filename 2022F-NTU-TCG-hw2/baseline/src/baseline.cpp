// Copyright (C) 2019 Yueh-Ting Chen (eopXD)
/*! \file greedy.cpp
        \brief baseline agents
         -D GREEDY, greedy movement evaluated by eval() below
         -D CONSERVATIVE, conservative strategy, eats opponent, or do hor/vert,
          if none of the above move exists, do random
         -D RANDOM, do random move from move_list()
         Add log: -D PLAYER1/PLAYER2
        \author Yueh-Ting Chen (eopXD)
        \course Theory of Computer Game (TCG)
        \modified by kjchan (2022)
*/

#include <time.h>

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <utility>
#include <vector>

#include "einstein.hpp"

char start;
char init[2][NUM_CUBE + 1] = {};
BOARD_GUI *b, tmp_b;
bool myturn;
inline void flip_bit(bool &_) { _ = !_; }
char num, dir;
std::fstream flog;
void logger(std::string logfile) {
    flog.open(logfile, std::fstream::out);
    if (!flog.is_open()) {
        throw std::runtime_error("error opening file\n");
    }
}
using PII = std::pair<int, int>;
using VII = std::vector<PII>;
using UII = std::unordered_map<int, int>;
#ifdef OLD_GREEDY
#define EAT 5.0      // threatened
#define ATTACK 1.0   // my minimum piece distance to goal
#define DEFENSE 1.0  // opponent's piece to goad

#define DRAW 50
#define WIN 100

double eval(BOARD_GUI *eval_b) {  // evaluating a board
    if (eval_b->winner() != Color::OTHER) {
        if (eval_b->winner() == eval_b->turn()) {  // lose
            return (-WIN);
        } else if (eval_b->winner() == Color::NO_ONE) {  // draw
            return (DRAW);
        } else {
            return (WIN);
        }
    }

    Color opponent = eval_b->turn();

    double threatened = 0.0;
    double distance[2] = {0.0, 0.0};

    auto ml = eval_b->move_list();
    for (auto &move : ml) {
        if (eval_b->yummy(move) == 1) {
            ++threatened;
        }
    }
    for (int ply = 0; ply < 2; ++ply) {
        for (int num = 0; num < NUM_CUBE; ++num) {
            PII pos = eval_b->find(static_cast<Color>(ply), num);
            if (pos.first == -100) {
                continue;
            }
            // change
            PII corner =
                (ply == 0) ? std::make_pair(BOARD_SZ - 2, BOARD_SZ - 1) : std::make_pair(0, 0);
            double coefficient = (ply == enum2int(opponent)) ? DEFENSE : ATTACK;
            distance[ply] += pow(NUM_CUBE - num, coefficient) *
                             (abs(pos.first - corner.first) + abs(pos.second - corner.second));
        }
    }
    double res = 0;
    res -= threatened * EAT;
    res -= DEFENSE * distance[enum2int(opponent)];
    res += ATTACK * distance[!enum2int(opponent)];
    return (res);
}
#endif

#ifdef HEURISTIC
double eval(BOARD_GUI *eval_b) {
    Color turn = eval_b->turn();
    PII p[NUM_CUBE * 2];
    double dis[2] = {0.0, 0.0};
    for (int ply = 0; ply < 2; ++ply) {
        for (int num = 0; num < NUM_CUBE; ++num) {
            PII pos = eval_b->find(static_cast<Color>(ply), num);
            p[ply * 6 + num] = pos;
            if (pos.first == -100) {
                continue;
            }
            double coefficient = (ply == enum2int(turn)) ? 1 : -1;
            dis[ply] += (coefficient * pow(NUM_CUBE - num, 2));
        }
    }

    double res = 0;
    res += dis[enum2int(turn)];
    res += dis[!enum2int(turn)];

    int opponent = (enum2int(turn) == 0) ? 1 : 0;
    if (opponent == 0) {
        int n = 0;
        for (int i = NUM_CUBE; i < 2 * NUM_CUBE; i++) {
            if (p[i].first != -100 && p[i].first > n) n = p[i].first;
        }
        // std::cout << n << '\n';
        for (int i = 0; i < NUM_CUBE; i++) {
            if (p[i].first != -100 && p[i].first > n) res -= pow(NUM_CUBE - i, 3);
        }
    } else if (opponent == 1) {
        int n = BOARD_SZ - 1;
        for (int i = 0; i < NUM_CUBE; i++) {
            if (p[i].first != -100 && p[i].first < n) n = p[i].second;
        }
        // std::cout << n << '\n';
        for (int i = NUM_CUBE; i < 2 * NUM_CUBE; i++) {
            if (p[i].first != -100 && p[i].first < n) res -= pow(2 * NUM_CUBE - i, 3);
        }
    }

    return (-1) * res;
}

double search(BOARD_GUI *eval_b, int dep, int limit, int alpha, int beta) {
    if (dep >= limit) return eval(eval_b);

    auto ml = b->move_list();
    if (ml.size() == 0) return eval(eval_b);
    double val = alpha;
    for (auto &move : ml) {
        b->do_move(move);
        double tmp = search(b, dep + 1, limit, (-1) * beta, (-1) * val);
        if (tmp > val) {
            val = tmp;
        }
        b->undo_move();
        if (val >= beta) {
            val = beta;
            break;
        }
    }
    return (-1) * val;
}

#endif

// #ifdef MCTS
/*
每一輪任選n次
每步 任選活著的, 任選可走的三向(斜步可能分數可以高一點)
走n層深以後 評估分數(?)

累加統計值
back prop

*/

// 我會被當紅色方test
// 所以我的move都往右或下比較快
class mcts_board_red {
   public:
    struct piece {
        int name;
        int pos;
        piece() {
            name = -1;
            pos = -1;
        }
    };
    UII enemy_piece, my_piece;
    VII enemy_available_move, my_available_move;
    struct piece enemy_corner;
    struct piece my_corner;
    int color;
    // constructor
    mcts_board_red(UII enemy, UII my) {
        enemy_piece = enemy;
        my_piece = my;
        enemy_available_move = available_move(0);

        my_available_move = available_move(1);
        // flog << "available move" << std::endl;
        // for (auto element : my_available_move) {
        //     flog << element.first << " " << element.second << std::endl;
        // }
        // flog << "available move end" << std::endl;
    };
    mcts_board_red() = default;

    // function
    // side: 0:enemy side, 1:my side
    void move(int side, int name, int pos) {
        if (side == 0) {
            enemy_piece[name] = pos;
            if (pos == 0) {
                enemy_corner.name = name;
                enemy_corner.pos = pos;
            }
        } else if (side == 1) {
            my_piece[name] = pos;
            if (pos == 41) {
                my_corner.name = name;
                my_corner.pos = pos;
            }
        }
        return;
    }
    void check_captured(int side, int name, int pos) {
        // flog<<"start check"<<std::endl;
        if (side == 1) {  // 檢查對面有沒有人被我踩掉
            for (auto find_piece : enemy_piece) {
                if (find_piece.second == pos) {
                    // flog<<"enemy be eat"<<find_piece.first<<" "<<find_piece.second<<std::endl;
                    // flog<<"now my name= "<<name<<", pos="<<pos<<std::endl;
                    VII::iterator it = enemy_available_move.begin();
                    for (; it != enemy_available_move.end(); it++) {
                        if ((*it).first == find_piece.first) {
                            // flog<<"erase from available"<<(*it).first<<"
                            // "<<(*it).second<<std::endl;
                            enemy_available_move.erase(it);
                            it--;
                        }
                    }
                    // flog<<"end of eat"<<std::endl;
                    // flog<<"erase from piece = "<<find_piece.first<<std::endl;
                    enemy_piece.erase(find_piece.first);
                    break;
                }
            }
        } else if (side == 0) {  // 檢查我有沒有人被對面踩掉
            for (auto find_piece : my_piece) {
                if (find_piece.second == pos) {
                    // flog<<"me be eat"<<std::endl;
                    VII::iterator it = my_available_move.begin();
                    for (; it != my_available_move.end(); it++) {
                        if ((*it).first == find_piece.first) {
                            // flog<<(*it).first<<std::endl;
                            // flog<<(*it).second<<std::endl;
                            my_available_move.erase(it);
                            it--;
                        }
                    }
                    // flog<<"end of eat"<<std::endl;
                    // flog<<"find_piece.first = "<<find_piece.first<<std::endl;
                    my_piece.erase(find_piece.first);
                    break;
                }
            }
        }
        // flog<<"end of check"<<std::endl;
        return;
    }
    int state() {
        // 0 = not over
        // 1 = me wins, 2 = enemy wins, 3 = draw
        if (enemy_piece.size() == 0) {
            return (1);
        }
        if (my_piece.size() == 0) {
            return (2);
        }
        // 判定角落
        if (my_corner.name != -1 && enemy_corner.name != -1) {
            if (my_corner.name < enemy_corner.name) {
                return (1);
            } else if (my_corner.name > enemy_corner.name) {
                return (2);
            } else {
                return (3);
            }
        }
        if (enemy_available_move.size() == 0 && my_available_move.size() == 0) {
            // flog << "no use" << std::endl;
            return (3);
            
        }
        return (0);
    }
    VII available_move(int side) {
        VII result;
        if (side == 1) {  // my side
            // 我是紅方 要往右下走
            // 只看往右 往下 往右下 有沒有爆格
            // 往右看%
            // 往下看數字就好了
            // 往右下看mod+數字
            for (auto find_piece : my_piece) {
                int pos = find_piece.second;
                if (pos % 7 < 6 && is_occupy(side, num, pos, 0) == false) {  // 能往右
                    result.emplace_back(find_piece.first, find_piece.second + 1);
                }
                if (pos < 35 && is_occupy(side, num, pos, 1) == false) {  // 能往下
                    result.emplace_back(find_piece.first, find_piece.second + 7);
                }
                if (pos % 7 < 6 && pos < 35 && is_occupy(side, num, pos, 2) == false) {  // 能往右下
                    result.emplace_back(find_piece.first, find_piece.second + 8);
                }
            }
        } else if (side == 0) {  // enemy side
            // 敵是紅方 要往左上走
            // 只看往左 往上 往左上 有沒有爆格
            // 往左看%
            // 往上看數字就好了
            // 往左上看mod+數字
            for (auto find_piece : enemy_piece) {
                int pos = find_piece.second;
                if (pos % 7 > 0 && is_occupy(side, num, pos, 0) == false) {  // 能往左
                    result.emplace_back(find_piece.first, find_piece.second - 1);
                }
                if (pos > 6 && is_occupy(side, num, pos, 1) == false) {  // 能往上
                    result.emplace_back(find_piece.first, find_piece.second - 7);
                }
                if (pos % 7 > 0 && pos > 6 && is_occupy(side, num, pos, 2) == false) {  // 能往左上
                    result.emplace_back(find_piece.first, find_piece.second - 8);
                }
            }
        }
        return result;
    }
    void update_available_move(int side, int num) {
        if (side == 1) {  // my side
            // 先清掉舊的
            VII::iterator it = my_available_move.begin();
            for (; it != my_available_move.end(); it++) {
                if ((*it).first == num) {
                    my_available_move.erase(it);
                    it--;
                }
            }

            // 加入新的
            // 我是紅方 要往右下走
            // 只看往右 往下 往右下 有沒有爆格
            // 往右看%
            // 往下看數字就好了
            // 往右下看mod+數字
            int pos = my_piece[num];
            if (pos % 7 < 6 && is_occupy(side, num, pos, 0) == false) {  // 能往右
                my_available_move.emplace_back(num, pos + 1);
            }
            if (pos < 35 && is_occupy(side, num, pos, 1) == false) {  // 能往下
                my_available_move.emplace_back(num, pos + 7);
            }
            if (pos % 7 < 6 && pos < 35 && is_occupy(side, num, pos, 2) == false) {  // 能往右下
                my_available_move.emplace_back(num, pos + 8);
            }
        } else if (side == 0) {  // enemy side
            VII::iterator it = enemy_available_move.begin();
            for (; it != enemy_available_move.end(); it++) {
                if ((*it).first == num) {
                    enemy_available_move.erase(it);
                    it--;
                }
            }
            // 敵是紅方 要往左上走
            // 只看往左 往上 往左上 有沒有爆格
            // 往左看%
            // 往上看數字就好了
            // 往左上看mod+數字
            int pos = enemy_piece[num];
            if (pos % 7 > 0 && is_occupy(side, num, pos, 0) == false) {  // 能往左
                enemy_available_move.emplace_back(num, pos - 1);
            }
            if (pos > 6 && is_occupy(side, num, pos, 1) == false) {  // 能往上
                enemy_available_move.emplace_back(num, pos - 7);
            }
            if (pos % 7 > 0 && pos > 6 && is_occupy(side, num, pos, 2) == false) {  // 能往左上
                enemy_available_move.emplace_back(num, pos - 8);
            }
        }
    }

    bool is_occupy(int side, int num, int pos, int dir) {  // 0左右, 1上下, 2斜向
        if (side == 1) {                                   // my side
            for (auto find_piece : my_piece) {
                if (dir == 0) {
                    if (find_piece.second == (pos + 1)) {
                        return true;
                    }
                } else if (dir == 1) {
                    if (find_piece.second == (pos + 7)) {
                        return true;
                    }
                } else if (dir == 2) {
                    if (find_piece.second == (pos + 8)) {
                        return true;
                    }
                }
            }
        } else if (side == 0) {  // enemy side
            for (auto find_piece : enemy_piece) {
                if (dir == 0) {
                    if (find_piece.second == (pos - 1)) {
                        return true;
                    }
                } else if (dir == 1) {
                    if (find_piece.second == (pos - 7)) {
                        return true;
                    }
                } else if (dir == 2) {
                    if (find_piece.second == (pos - 8)) {
                        return true;
                    }
                }
            }
        }
        return false;
    }
};

class mcts_tree_node_red {
   public:
    // 結構類
    mcts_tree_node_red *parent, *now, *root;
    mcts_board_red now_board;
    PII last_move;  // if parent==root, last_move=(-1,-1)

    std::vector<std::pair<PII, mcts_tree_node_red *>> child_list;
    // 屬性類
    int win_cnt, lose_cnt, draw_cnt, total_cnt;

    // constructor
    mcts_tree_node_red() = default;
    mcts_tree_node_red(mcts_board_red board, PII move) {
        win_cnt = 0, lose_cnt = 0, draw_cnt = 0, total_cnt = 0;
        now_board = board;
        last_move = move;
    };

    // void init_ptr(mcts_tree_node *a, mcts_tree_node *b, mcts_tree_node *c) {
    //     parent = a;
    //     now = b;
    //     root = c;
    // }
    PII simulate(int pick_rounds) {
        /*
        1. select
                先採random select
                之後再採用
        2. expand
        3. rollout
        4. backprop
        */
        struct timespec start_time, end_time;
        clock_gettime(CLOCK_REALTIME, &start_time);
        int temp_cnt = 0;
        while (temp_cnt < pick_rounds) {
            clock_gettime(CLOCK_REALTIME, &end_time);
            if ((double)(end_time.tv_sec + end_time.tv_nsec * 1e-9) -
                    (double)(start_time.tv_sec + start_time.tv_nsec * 1e-9) >
                9.5) {
                flog << "pick_rounds = " << temp_cnt << std::endl;
                break;
            }
            temp_cnt++;
            // flog<<"start select_expand"<<std::endl;
            mcts_tree_node_red *choose_child;
            if (temp_cnt < 3000) {
                choose_child = select_expand_with_all(temp_cnt);
            } else {
                choose_child = select_expand_with_ucb();
            }
            if (choose_child == nullptr) {
                return std::make_pair(15, 15);
            }
            // flog<<"start rollout------------------------------------"<<std::endl;
            int result = rollout(choose_child);
            // flog<<"start back_prop"<<std::endl;
            back_prop(choose_child, result);
        }
        double win_rate = -1;
        PII final_select_move = std::make_pair(-1, -1);
        // child_list結構長這樣 std::vector<std::pair<PII, mcts_tree_node *>> child_list;
        for (int i = 0; i < child_list.size(); i++) {
            double temp_win_rate =
                (double)child_list[i].second->win_cnt / child_list[i].second->total_cnt;
            if (win_rate < temp_win_rate) {
                win_rate = temp_win_rate;
                final_select_move = child_list[i].first;
            }
            // flog<<"wincnt= "<<child_list[i].second->win_cnt<<std::endl;
            // flog<<"totalcnt= "<<child_list[i].second->total_cnt<<std::endl;
        }
        // flog << child_list.size() << std::endl;
        // flog << "win rate=" << win_rate << std::endl;
        return final_select_move;
    }

    mcts_tree_node_red *select_expand_with_all(int count) {
        // random select
        VII available_move = now_board.available_move(1);
        if (available_move.size() == 0) {
            return nullptr;  // 告訴他現在沒有available move了
        }
        PII chose = available_move.at(count % available_move.size());

        // child_list結構長這樣 std::vector<std::pair<PII, mcts_tree_node *>> child_list;

        // 找看看現在選的這個步有沒有走過
        for (int i = 0; i < child_list.size(); i++) {
            PII child_move = child_list[i].first;
            mcts_tree_node_red *child_ptr = child_list[i].second;
            if (chose == child_move) {
                return child_ptr;
            }
        }
        // 因為沒走過所以要expand他的樹出來
        mcts_board_red temp_board = now_board;
        temp_board.move(1, chose.first, chose.second);
        temp_board.check_captured(1, chose.first, chose.second);
        temp_board.update_available_move(1, chose.first);
        mcts_tree_node_red *next_child;
        next_child = new mcts_tree_node_red(temp_board, chose);
        child_list.emplace_back(std::make_pair(chose, next_child));

        return next_child;
    }
    mcts_tree_node_red *select_expand_with_ucb() {
        // random select
        VII available_move = now_board.available_move(1);
        if (available_move.size() == 0) {
            return nullptr;  // 告訴他現在沒有available move了
        }
        // 只挑勝率最高的瘋狂模擬
        double max_UCB = -999;
        mcts_tree_node_red *next_child;
        int total_gamecnt = 0;
        int para_c = 10;
        for (int i = 0; i < child_list.size(); i++) {
            total_gamecnt += child_list[i].second->total_cnt;
        }
        for (int i = 0; i < child_list.size(); i++) {
            double temp_win_rate =
                (double)child_list[i].second->win_cnt / child_list[i].second->total_cnt;
            double temp_UCB =
                temp_win_rate +
                para_c * std::pow((std::log(total_gamecnt) / child_list[i].second->total_cnt), 0.5);
            if (max_UCB < temp_UCB) {
                max_UCB = temp_UCB;
                next_child = child_list[i].second;
            }
        }
        // child_list結構長這樣 std::vector<std::pair<PII, mcts_tree_node *>> child_list;
        return next_child;
    }

    // void expand() { return; }
    int rollout(mcts_tree_node_red *choose_child) {
        // 這邊開始roll out
        // 任取n層都不用建立mcts node
        // 只要維護盤面valid 並且維護state
        mcts_board_red roll_board = choose_child->now_board;
        int turn = 0;                      // 0 enemy side, 1 my side
        while (roll_board.state() == 0) {  // 0 not over 1 = me wins, 2 = enemy wins, 3 = draw
            // rand chose
            // do move
            // check captured
            // update
            // flog<<"change turn to "<<turn<<std::endl;
            if (turn == 0) {  // enemy turn
                if (!roll_board.enemy_available_move.empty()) {
                    PII chose = roll_board.enemy_available_move.at(
                        rand() % roll_board.enemy_available_move.size());
                    // flog<<chose.first<<" "<<chose.second<<std::endl;
                    // flog<<"start move "<<std::endl;
                    roll_board.move(0, chose.first, chose.second);
                    // flog<<"start check "<<std::endl;
                    roll_board.check_captured(0, chose.first, chose.second);
                    // flog<<"start update "<<std::endl;
                    roll_board.update_available_move(0, chose.first);
                }

                turn = 1;
            } else {  // my turn
                if (!roll_board.my_available_move.empty()) {
                    PII chose = roll_board.my_available_move.at(
                        rand() % roll_board.my_available_move.size());
                    // flog<<chose.first<<" "<<chose.second<<std::endl;
                    // flog<<"start move "<<std::endl;
                    roll_board.move(1, chose.first, chose.second);
                    // flog<<"start check "<<std::endl;
                    roll_board.check_captured(1, chose.first, chose.second);
                    // flog<<"start update "<<std::endl;
                    roll_board.update_available_move(1, chose.first);
                }
                turn = 0;
            }
        }
        // flog<< "result= "<<roll_board.state()<<std::endl;
        return roll_board.state();
    }
    void back_prop(mcts_tree_node_red *choose_child, int result) {
        if (result == 1) {
            choose_child->win_cnt++;
        } else if (result == 2) {
            choose_child->lose_cnt++;
        } else if (result == 3) {
            choose_child->draw_cnt++;
        }
        choose_child->total_cnt++;
        return;
    }
    int eval_mcts() { return 0; }
};

class mcts_board_blue {
   public:
    struct piece {
        int name;
        int pos;
        piece() {
            name = -1;
            pos = -1;
        }
    };
    UII enemy_piece, my_piece;
    VII enemy_available_move, my_available_move;
    struct piece enemy_corner;
    struct piece my_corner;
    int color;
    // constructor
    mcts_board_blue(UII enemy, UII my) {
        enemy_piece = enemy;
        my_piece = my;
        enemy_available_move = available_move(1);

        my_available_move = available_move(0);
        // flog << "available move" << std::endl;
        // for (auto element : my_available_move) {
        //     flog << element.first << " " << element.second << std::endl;
        // }
        // flog << "available move end" << std::endl;
    };
    mcts_board_blue() = default;

    // function
    // side: 1:enemy side, 0:my side
    void move(int side, int name, int pos) {
        if (side == 1) {
            enemy_piece[name] = pos;
            if (pos == 41) {
                enemy_corner.name = name;
                enemy_corner.pos = pos;
            }
        } else if (side == 0) {
            my_piece[name] = pos;
            if (pos == 0) {
                my_corner.name = name;
                my_corner.pos = pos;
            }
        }
        return;
    }
    void check_captured(int side, int name, int pos) {
        // flog<<"start check"<<std::endl;
        if (side == 0) {  // 檢查對面有沒有人被我踩掉
            for (auto find_piece : enemy_piece) {
                if (find_piece.second == pos) {
                    // flog<<"enemy be eat"<<find_piece.first<<" "<<find_piece.second<<std::endl;
                    // flog<<"now my name= "<<name<<", pos="<<pos<<std::endl;
                    VII::iterator it = enemy_available_move.begin();
                    for (; it != enemy_available_move.end(); it++) {
                        if ((*it).first == find_piece.first) {
                            // flog<<"erase from available"<<(*it).first<<"
                            // "<<(*it).second<<std::endl;
                            enemy_available_move.erase(it);
                            it--;
                        }
                    }
                    // flog<<"end of eat"<<std::endl;
                    // flog<<"erase from piece = "<<find_piece.first<<std::endl;
                    enemy_piece.erase(find_piece.first);
                    break;
                }
            }
        } else if (side == 1) {  // 檢查我有沒有人被對面踩掉
            for (auto find_piece : my_piece) {
                if (find_piece.second == pos) {
                    // flog<<"me be eat"<<std::endl;
                    VII::iterator it = my_available_move.begin();
                    for (; it != my_available_move.end(); it++) {
                        if ((*it).first == find_piece.first) {
                            // flog<<(*it).first<<std::endl;
                            // flog<<(*it).second<<std::endl;
                            my_available_move.erase(it);
                            it--;
                        }
                    }
                    // flog<<"end of eat"<<std::endl;
                    // flog<<"find_piece.first = "<<find_piece.first<<std::endl;
                    my_piece.erase(find_piece.first);
                    break;
                }
            }
        }
        // flog<<"end of check"<<std::endl;
        return;
    }
    int state() {
        // 0 = not over
        // 1 = me wins, 2 = enemy wins, 3 = draw
        if (enemy_piece.size() == 0) {
            return (1);
        }
        if (my_piece.size() == 0) {
            return (2);
        }
        // 判定角落
        if (my_corner.name != -1 && enemy_corner.name != -1) {
            if (my_corner.name < enemy_corner.name) {
                return (1);
            } else if (my_corner.name > enemy_corner.name) {
                return (2);
            } else {
                return (3);
            }
        }
        if (enemy_available_move.size() == 0 && my_available_move.size() == 0) {
            // flog << "no use" << std::endl;
            return (3);
        }
        return (0);
    }
    VII available_move(int side) {
        VII result;
        if (side == 1) {  // enemy side
            // 敵是紅方 要往右下走
            // 只看往右 往下 往右下 有沒有爆格
            // 往右看%
            // 往下看數字就好了
            // 往右下看mod+數字
            for (auto find_piece : enemy_piece) {
                // flog<<"find_piece "<<find_piece.first<<" "<<find_piece.second<<std::endl;
                int pos = find_piece.second;
                if (pos % 7 < 6 && is_occupy(side, num, pos, 0) == false) {  // 能往右
                    result.emplace_back(find_piece.first, find_piece.second + 1);
                }
                if (pos < 35 && is_occupy(side, num, pos, 1) == false) {  // 能往下
                    result.emplace_back(find_piece.first, find_piece.second + 7);
                }
                if (pos % 7 < 6 && pos < 35 && is_occupy(side, num, pos, 2) == false) {  // 能往右下
                    result.emplace_back(find_piece.first, find_piece.second + 8);
                }
            }
        } else if (side == 0) {  // my side
            // 我是藍方 要往左上走
            // 只看往左 往上 往左上 有沒有爆格
            // 往左看%
            // 往上看數字就好了
            // 往左上看mod+數字
            for (auto find_piece : my_piece) {
                // flog<<"find_piece "<<find_piece.first<<" "<<find_piece.second<<std::endl;
                int pos = find_piece.second;
                if (pos % 7 > 0 && is_occupy(side, num, pos, 0) == false) {  // 能往左
                    result.emplace_back(find_piece.first, find_piece.second - 1);
                }
                if (pos > 6 && is_occupy(side, num, pos, 1) == false) {  // 能往上
                    result.emplace_back(find_piece.first, find_piece.second - 7);
                }
                if (pos % 7 > 0 && pos > 6 && is_occupy(side, num, pos, 2) == false) {  // 能往左上
                    result.emplace_back(find_piece.first, find_piece.second - 8);
                }
            }
        }
        return result;
    }
    void update_available_move(int side, int num) {
        if (side == 0) {  // my side
            // 先清掉舊的
            VII::iterator it = my_available_move.begin();
            for (; it != my_available_move.end(); it++) {
                if ((*it).first == num) {
                    my_available_move.erase(it);
                    it--;
                }
            }

            // 加入新的
            // 我是藍方 要往左上走
            // 只看往左 往上 往左上 有沒有爆格
            // 往左看%
            // 往上看數字就好了
            // 往左上看mod+數字
            int pos = my_piece[num];
            if (pos % 7 > 0 && is_occupy(side, num, pos, 0) == false) {  // 能往左
                my_available_move.emplace_back(num, pos - 1);
            }
            if (pos > 6 && is_occupy(side, num, pos, 1) == false) {  // 能往上
                my_available_move.emplace_back(num, pos - 7);
            }
            if (pos % 7 > 0 && pos > 6 && is_occupy(side, num, pos, 2) == false) {  // 能往左上
                my_available_move.emplace_back(num, pos - 8);
            }
        } else if (side == 1) {  // enemy side
            VII::iterator it = enemy_available_move.begin();
            for (; it != enemy_available_move.end(); it++) {
                if ((*it).first == num) {
                    enemy_available_move.erase(it);
                    it--;
                }
            }
            // 敵是紅方 要往右下走
            // 只看往右 往下 往右下 有沒有爆格
            // 往右看%
            // 往下看數字就好了
            // 往右下看mod+數字
            int pos = enemy_piece[num];
            if (pos % 7 < 6 && is_occupy(side, num, pos, 0) == false) {  // 能往右
                enemy_available_move.emplace_back(num, pos + 1);
            }
            if (pos < 35 && is_occupy(side, num, pos, 1) == false) {  // 能往下
                enemy_available_move.emplace_back(num, pos + 7);
            }
            if (pos % 7 < 6 && pos < 35 && is_occupy(side, num, pos, 2) == false) {  // 能往右下
                enemy_available_move.emplace_back(num, pos + 8);
            }
        }
    }

    bool is_occupy(int side, int num, int pos, int dir) {  // 0左右, 1上下, 2斜向
        if (side == 1) {                                   // enemy side
            for (auto find_piece : enemy_piece) {
                if (dir == 0) {
                    if (find_piece.second == (pos + 1)) {
                        return true;
                    }
                } else if (dir == 1) {
                    if (find_piece.second == (pos + 7)) {
                        return true;
                    }
                } else if (dir == 2) {
                    if (find_piece.second == (pos + 8)) {
                        return true;
                    }
                }
            }
        } else if (side == 0) {  // my side
            for (auto find_piece : my_piece) {
                if (dir == 0) {
                    if (find_piece.second == (pos - 1)) {
                        return true;
                    }
                } else if (dir == 1) {
                    if (find_piece.second == (pos - 7)) {
                        return true;
                    }
                } else if (dir == 2) {
                    if (find_piece.second == (pos - 8)) {
                        return true;
                    }
                }
            }
        }
        return false;
    }
};

class mcts_tree_node_blue {
   public:
    // 結構類
    mcts_tree_node_blue *parent, *now, *root;
    mcts_board_blue now_board;
    PII last_move;  // if parent==root, last_move=(-1,-1)

    std::vector<std::pair<PII, mcts_tree_node_blue *>> child_list;
    // 屬性類
    int win_cnt, lose_cnt, draw_cnt, total_cnt;

    // constructor
    mcts_tree_node_blue() = default;
    mcts_tree_node_blue(mcts_board_blue board, PII move) {
        win_cnt = 0, lose_cnt = 0, draw_cnt = 0, total_cnt = 0;
        now_board = board;
        last_move = move;
    };

    // void init_ptr(mcts_tree_node *a, mcts_tree_node *b, mcts_tree_node *c) {
    //     parent = a;
    //     now = b;
    //     root = c;
    // }
    PII simulate(int pick_rounds) {
        /*
        1. select
                先採random select
                之後再採用
        2. expand
        3. rollout
        4. backprop
        */
        struct timespec start_time, end_time;
        clock_gettime(CLOCK_REALTIME, &start_time);
        int temp_cnt = 0;
        while (temp_cnt < pick_rounds) {
            clock_gettime(CLOCK_REALTIME, &end_time);
            if ((double)(end_time.tv_sec + end_time.tv_nsec * 1e-9) -
                    (double)(start_time.tv_sec + start_time.tv_nsec * 1e-9) >
                9.5) {
                flog << "pick_rounds = " << temp_cnt << std::endl;
                break;
            }
            temp_cnt++;
            // flog<<"start select_expand"<<std::endl;
            mcts_tree_node_blue *choose_child;
            if (temp_cnt < 3000) {
                choose_child = select_expand_with_all(temp_cnt);
            } else {
                choose_child = select_expand_with_ucb();
            }
            if (choose_child == nullptr) {
                return std::make_pair(15, 15);
            }
            // flog<<"start rollout------------------------------------"<<std::endl;
            int result = rollout(choose_child);
            // flog<<"start back_prop"<<std::endl;
            back_prop(choose_child, result);
        }
        double win_rate = -1;
        PII final_select_move = std::make_pair(-1, -1);
        // child_list結構長這樣 std::vector<std::pair<PII, mcts_tree_node *>> child_list;
        for (int i = 0; i < child_list.size(); i++) {
            double temp_win_rate =
                (double)child_list[i].second->win_cnt / child_list[i].second->total_cnt;
            if (win_rate < temp_win_rate) {
                win_rate = temp_win_rate;
                final_select_move = child_list[i].first;
            }
            // flog<<"wincnt= "<<child_list[i].second->win_cnt<<std::endl;
            // flog<<"totalcnt= "<<child_list[i].second->total_cnt<<std::endl;
        }
        // flog << child_list.size() << std::endl;
        // flog << "win rate=" << win_rate << std::endl;
        return final_select_move;
    }

    mcts_tree_node_blue *select_expand_with_all(int count) {
        // random select
        VII available_move = now_board.available_move(0);
        if (available_move.size() == 0) {
            return nullptr;  // 告訴他現在沒有available move了
        }
        PII chose = available_move.at(count % available_move.size());

        // child_list結構長這樣 std::vector<std::pair<PII, mcts_tree_node *>> child_list;

        // 找看看現在選的這個步有沒有走過
        for (int i = 0; i < child_list.size(); i++) {
            PII child_move = child_list[i].first;
            mcts_tree_node_blue *child_ptr = child_list[i].second;
            if (chose == child_move) {
                return child_ptr;
            }
        }
        // 因為沒走過所以要expand他的樹出來
        mcts_board_blue temp_board = now_board;
        temp_board.move(0, chose.first, chose.second);
        temp_board.check_captured(0, chose.first, chose.second);
        temp_board.update_available_move(0, chose.first);
        mcts_tree_node_blue *next_child;
        next_child = new mcts_tree_node_blue(temp_board, chose);
        child_list.emplace_back(std::make_pair(chose, next_child));

        return next_child;
    }
    mcts_tree_node_blue *select_expand_with_ucb() {
        // random select
        VII available_move = now_board.available_move(0);
        if (available_move.size() == 0) {
            return nullptr;  // 告訴他現在沒有available move了
        }
        // 只挑勝率最高的瘋狂模擬
        double max_UCB = -999;
        mcts_tree_node_blue *next_child;
        int total_gamecnt = 0;
        int para_c = 10;
        for (int i = 0; i < child_list.size(); i++) {
            total_gamecnt += child_list[i].second->total_cnt;
        }
        for (int i = 0; i < child_list.size(); i++) {
            double temp_win_rate =
                (double)child_list[i].second->win_cnt / child_list[i].second->total_cnt;
            double temp_UCB =
                temp_win_rate +
                para_c * std::pow((std::log(total_gamecnt) / child_list[i].second->total_cnt), 0.5);
            if (max_UCB < temp_UCB) {
                max_UCB = temp_UCB;
                next_child = child_list[i].second;
            }
        }
        // child_list結構長這樣 std::vector<std::pair<PII, mcts_tree_node *>> child_list;
        return next_child;
    }
    // void expand() { return; }
    int rollout(mcts_tree_node_blue *choose_child) {
        // 這邊開始roll out
        // 任取n層都不用建立mcts node
        // 只要維護盤面valid 並且維護state
        mcts_board_blue roll_board = choose_child->now_board;
        int turn = 1;                      // 1 enemy side, 0 my side
        while (roll_board.state() == 0) {  // 0 not over 1 = me wins, 2 = enemy wins, 3 = draw
            // rand chose
            // do move
            // check captured
            // update
            // flog<<"change turn to "<<turn<<std::endl;

            if (turn == 1) {  // enemy turn
                if (!roll_board.enemy_available_move.empty()) {
                    PII chose = roll_board.enemy_available_move.at(
                        rand() % roll_board.enemy_available_move.size());
                    // flog<<chose.first<<" "<<chose.second<<std::endl;
                    // flog<<"start move "<<std::endl;
                    roll_board.move(1, chose.first, chose.second);
                    // flog<<"start check "<<std::endl;
                    roll_board.check_captured(1, chose.first, chose.second);
                    // flog<<"start update "<<std::endl;
                    roll_board.update_available_move(1, chose.first);
                }

                turn = 0;
            } else {  // my turn
                if (!roll_board.my_available_move.empty()) {
                    PII chose = roll_board.my_available_move.at(
                        rand() % roll_board.my_available_move.size());
                    // flog<<chose.first<<" "<<chose.second<<std::endl;
                    // flog<<"start move "<<std::endl;
                    roll_board.move(0, chose.first, chose.second);
                    // flog<<"start check "<<std::endl;
                    roll_board.check_captured(0, chose.first, chose.second);
                    // flog<<"start update "<<std::endl;
                    roll_board.update_available_move(0, chose.first);
                }
                turn = 1;
            }
        }
        // flog<< "result= "<<roll_board.state()<<std::endl;
        return roll_board.state();
    }
    void back_prop(mcts_tree_node_blue *choose_child, int result) {
        if (result == 1) {
            choose_child->win_cnt++;
        } else if (result == 2) {
            choose_child->lose_cnt++;
        } else if (result == 3) {
            choose_child->draw_cnt++;
        }
        choose_child->total_cnt++;
        return;
    }
};
// void test_function() {
//     mcts_tree_node *fake_parent;
//     std::unordered_map<int, int> temp1, temp2;
//     mcts_board test(temp1, temp2);
//     // mcts_tree_node temp(fake_parent);
//     // flog << temp.win_cnt << temp.lose_cnt << temp.draw_cnt << temp.turn_cnt << std::endl;
// }

// #endif

int main() {
#ifdef HEURISTIC
    logger(".log.heuristic");
#endif
#ifdef GREEDY
    logger(".log.greedy");
#endif
#ifdef RANDOM
    logger(".log.random");
#endif
#ifdef MCTS
    logger(".log.pure_mcts");
#endif

    // auto timer = [](bool reset = false) -> double {
    //     static decltype(std::chrono::steady_clock::now()) tick, tock;
    //     if (reset) {
    //         tick = std::chrono::steady_clock::now();
    //         return (0);
    //     }
    //     tock = std::chrono::steady_clock::now();
    //     return (std::chrono::duration_cast<std::chrono::duration<double>>(tock - tick).count());
    // };

    srand(time(NULL));
    // test_function();
    do {
        /* get initial positions */
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < NUM_CUBE; ++j) {
                init[i][j] = getchar();
            }
        }
        init[0][NUM_CUBE] = init[1][NUM_CUBE] = '\0';
        start = getchar();

        flog << init[0] << " " << init[1] << std::endl;
        flog << start << std::endl;
        /* game start s*/
        b = new BOARD_GUI(init[0], init[1]);
        b->no_hl = 1;

        for (myturn = (start == 'f'); b->winner() == Color::OTHER; flip_bit(myturn)) {
            if (myturn) {
#ifdef HEURISTIC

                flog << "origin\n";
                flog << *(b);
                auto ml = b->move_list();
                PII m;
                double alpha = -1000000;
                double beta = 1000000;
                double val = alpha;
                for (auto &move : ml) {
                    b->do_move(move);
                    double tmp = search(b, 0, 5, (-1) * beta, (-1) * val);
                    if (tmp > val) {
                        m = move;
                        val = tmp;
                    }
                    b->undo_move();
                    if (val >= beta) {
                        break;
                    }
                }

                if (m.first == -1) {
                    m = ml[rand() % ml.size()];
                }
#endif
#ifdef GREEDY
                // flog << "greedy start" << std::endl;
                auto ml = b->move_list();
                // flog << "move list start" << std::endl;
                PII m = PII(-1, -1);
                for (auto &move : ml) {
                    if (b->yummy(move) == 1) {
                        m = move;
                        break;
                    }
                }
                // 表示前一步沒有找到可吃的
                if (m.first == -1) {
                    for (auto &move : ml) {
                        if (move.second < 2 and b->yummy(move) != -1) {
                            m = move;
                            break;
                        }
                    }
                }
                // 表示任走一步"非斜步" 並且不是自吃步 都沒有東西
                if (m.first == -1) {
                    m = ml[rand() % ml.size()];
                }
#endif
#ifdef RANDOM
                auto ml = b->move_list();
                auto m = ml.at(rand() % ml.size());
#endif
#ifdef MCTS
                UII r_piece, b_piece;
                for (int i = 0; i < 6; i++) {
                    int num = i;
                    PII b_find = b->find(Color::B, i);
                    PII r_find = b->find(Color::R, i);
                    int r_pos = r_find.first * 7 + r_find.second;
                    int b_pos = b_find.first * 7 + b_find.second;
                    if (r_pos <= 41 && r_pos >= 0) r_piece[num] = r_pos;
                    if (b_pos <= 41 && b_pos >= 0) b_piece[num] = b_pos;
                    // flog << "R, num=" << num << ", pos=" << r_pos << std::endl;
                    // flog << "B, num=" << num << ", pos=" << b_pos << std::endl;
                }
                // flog<<"detect board--------------" <<std::endl;
                // for (auto elements : enemy_piece) {
                //     flog << elements.first << " " << elements.second << std::endl;
                // }
                // flog <<"--------"<<std::endl;
                // for (auto elements : my_piece) {
                //     flog << elements.first << " " << elements.second << std::endl;
                // }
                // flog<<"detect board end----------" <<std::endl;
                PII m;
                if (b->turn() == Color::R) {
                    mcts_board_red now_turn_board(b_piece, r_piece);  // 先傳enemy再傳mine
                    mcts_tree_node_red now_turn_mcts(now_turn_board, std::make_pair(-1, -1));
                    PII my_move = now_turn_mcts.simulate(99900000);
                    int my_move_x = my_move.second / 7;
                    int my_move_y = my_move.second % 7;
                    int org_x = b->find(Color::R, my_move.first).first;
                    int org_y = b->find(Color::R, my_move.first).second;
                    int pos = 0;
                    if (my_move_x != org_x && my_move_y != org_y)
                        pos = 2;  // 2是斜向移動
                    else if (my_move_x != org_x)
                        pos = 0;  // 0是垂直移動
                    else
                        pos = 1;  // 1是水平移動
                    m = std::make_pair(my_move.first, pos);
                    if (my_move.first == 15 && my_move.second == 15) {
                        m = my_move;
                    }
                    // flog << "my choose = " << my_move.first << " " << my_move.second <<
                    // std::endl;
                } else {
                    mcts_board_blue now_turn_board(r_piece, b_piece);
                    mcts_tree_node_blue now_turn_mcts(now_turn_board, std::make_pair(-1, -1));
                    PII my_move = now_turn_mcts.simulate(99900000);
                    int my_move_x = my_move.second / 7;
                    int my_move_y = my_move.second % 7;
                    int org_x = b->find(Color::B, my_move.first).first;
                    int org_y = b->find(Color::B, my_move.first).second;
                    int pos = 0;
                    if (my_move_x != org_x && my_move_y != org_y)
                        pos = 2;  // 2是斜向移動
                    else if (my_move_x != org_x)
                        pos = 0;  // 0是垂直移動
                    else
                        pos = 1;  // 1是水平移動
                    m = std::make_pair(my_move.first, pos);
                    if (my_move.first == 15 && my_move.second == 15) {
                        m = my_move;
                    }
                    // flog << "my choose = " << my_move.first << " " << my_move.second <<
                    // std::endl; flog << "my choose = " << my_move.first <<" "<< my_move.second <<
                    // std::endl;
                }

// flog << "start mcts----------------------------------------" << std::endl;

// flog << "end mcts------------------------------------------" << std::endl;
// flog << "my choose = " << my_move.first <<" "<< my_move.second << std::endl;
#endif
                flog << myturn << b->send_move(m) << std::endl;
                b->do_move(m);
                std::cout << b->send_move(m) << std::flush;
            } else {
                num = getchar() - '0';
                dir = getchar() - '0';
                flog << myturn << (int)num << (int)dir << std::endl;
                if (num == 16) {
                    b->undo_move();
                    b->undo_move();
                    flip_bit(myturn);
                } else {
                    b->do_move(num, dir);
                }
            }
        }
        flog << "winner: " << b->winner() << std::endl;
        /* game end */
        delete b;
    } while (getchar() == 'y');

    return (0);
}
