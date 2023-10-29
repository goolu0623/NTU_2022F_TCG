#include <iostream>
#include <unordered_map>
#include <vector>
using std::cerr;
using std::pair;
using std::unordered_map;
using std::vector;
#define RED 0   // 紅方左上
#define BLUE 1  // 藍方右下
#define BOARD_SIZE 5
#define PIECE_NUM 6
#define COMMAND_NUM 7

/*
ng search 概念
// for (int i = 0; i < move_count; i++) {
//     // do move
//     double now_val = ngsearch(board, dep, limit, (-1) * beta, (-1) * alpha);
//     if (now_val > max_val) {
//         now_val = max_val;
//     }
//     // undo move
// }
// return highest value move here

*/

class Nega_Board {
   public:
    struct move {
        int orgr;
        int orgc;
        int tarr;
        int tarc;
        int move_num;  // 0表示沒子被吃 剩下表示被吃子編號
        int eat_num;
    };
    struct piece {  // 棋子結構 編號0是無棋 1~6 藍 7~12 紅
        int num;
        int row;
        int col;
        int piece_distance;  // 距離目標角的歐式距離
        double piece_value;  // power(N, piece_distance);
        bool exist;
    };

    vector<move> history_move;
    // bool available_move[3];
    int ng_board[BOARD_SIZE][BOARD_SIZE];  // 存盤面
    piece piece_list[2 * PIECE_NUM + 1];  // 存雙方全部子 0放空 這樣比較好對齊數字編號
    int red_exist_cnt;
    int blue_exist_cnt;
    double N=2;
    int K1 = 50, K2 = 20, K3 = 1;
    int search_cnt = 0;
    void do_move(struct move move) {
        move.eat_num = ng_board[move.tarr][move.tarc];  // 更新會被吃的子 (如果沒有就是0)
        piece_list[move.move_num].row = move.tarr,
        piece_list[move.move_num].col = move.tarc;  // 更新piece list
        if (move.tarr > move.orgr || move.tarc > move.orgc) {
            // 代表現在是紅方 要往右下進攻
            piece_list[move.move_num].piece_distance = std::min(move.tarr, move.tarc);
            piece_list[move.move_num].piece_value =
                std::pow(N, piece_list[move.move_num].piece_distance);
        } else {
            // 代表現在是藍方 要往左上進攻
            piece_list[move.move_num].piece_distance = 4 - std::max(move.tarr, move.tarc);
            piece_list[move.move_num].piece_value =
                std::pow(N, piece_list[move.move_num].piece_distance);
        }
        ng_board[move.tarr][move.tarc] = ng_board[move.orgr][move.orgc];  // 更新棋盤target位置
        ng_board[move.orgr][move.orgc] = 0;  // 更新棋盤原先位置
        if (move.eat_num > 0) {              // 代表有子被吃
            piece_list[move.eat_num].exist = false;
            // fprintf(stderr, "now move from (%d,%d) to (%d,%d)\n", move.orgr, move.orgc, move.tarr,
            //         move.tarc);
            // fprintf(stderr, "eatnum = %d\n", move.eat_num);
            // cerr<<"--------eateateateateateat---"<<move.eat_num<<std::endl;
            move.eat_num > 6 ? red_exist_cnt-- : blue_exist_cnt--;
        }
        history_move.emplace_back(move);
    }
    void undo_move() {
        struct move move = history_move[history_move.size() - 1];
        history_move.erase(history_move.end() - 1);
        ng_board[move.orgr][move.orgc] = ng_board[move.tarr][move.tarc];
        ng_board[move.tarr][move.tarc] = move.eat_num;
        if (move.tarr > move.orgr || move.tarc > move.orgc) {
            // 代表現在是紅方 要往右下進攻
            piece_list[move.move_num].piece_distance = std::min(move.orgr, move.orgc);
            piece_list[move.move_num].piece_value =
                std::pow(N, piece_list[move.move_num].piece_distance);
        } else {
            // 代表現在是藍方 要往左上進攻
            piece_list[move.move_num].piece_distance = 4 - std::max(move.orgr, move.orgc);
            piece_list[move.move_num].piece_value =
                std::pow(N, piece_list[move.move_num].piece_distance);
            // int temp = 4 - piece_list[move.move_num].piece_distance;
            // fprintf(stderr, "piece distance = %d, 4 - piecedistance = %d\n",
            //         piece_list[move.move_num].piece_distance,
            //         4 - piece_list[move.move_num].piece_distance);
            // fprintf(stderr, "pow(N,4-p) = %f v.s pow(N,(4-p)) = %f \n",
            //         std::pow(N, 4-piece_list[move.move_num].piece_distance), std::pow(N, temp));
        }
        if (move.eat_num != 0) {  // 代表有東西被吃
            piece_list[move.eat_num].exist = true;
            move.eat_num > 6 ? red_exist_cnt++ : blue_exist_cnt++;
        }
        // fprintf(stderr, "now undo move from (%d,%d) to (%d,%d)\n", move.orgr, move.orgc, move.tarr,
        //         move.tarc);
        // fprintf(stderr, "eatnum = %d\n", move.eat_num);
    }
};

class Nega_Node {
   public:
    // int hash_code;
    int side;  // 這場你的顏色
    int turn;  // 現在輪誰進攻
    int dice;  // 這輪的骰子
    Nega_Board::move best_move;
    Nega_Board *board;
    vector<Nega_Board::move> available_move;  // 0表示可往水平 1表示可往斜向 2表示可往垂直
    double ng_search(int depth, int limit, double alpha, double beta) {  // 演算法主程式
        this->board->search_cnt++;
        // fprintf(stderr, "start search dep = %d, lim = %d\n", depth, limit);
        // for(int i=0;i<BOARD_SIZE;i++){
        //     fprintf(stderr, "----------%d  %d  %d  %d  %d
        //     -----\n",board->ng_board[i][0],board->ng_board[i][1],board->ng_board[i][2],board->ng_board[i][3],board->ng_board[i][4]);
        // }

        // fprintf(stderr, "start search: \n");
        int end_state = state();
        if (end_state != 0) {
            // fprintf(stderr, "end state! \n");
        }
        if (end_state != 0) {  // 終盤 1藍勝 2紅勝 0還沒結束, header的RED==0, BLUE==1
            if (this->turn == RED) {
                if (end_state == 2)
                    return DBL_MAX;
                else
                    return DBL_MIN;
            } else if (this->turn == BLUE) {
                if (end_state == 1)
                    return DBL_MAX;
                else
                    return DBL_MIN;
            }
        }
        if (depth == limit) {  // 深度達標
            // fprintf(stderr, "depth limit! \n");
            return evaluate();
        }
        find_available_move();
        double local_max = alpha;
        // fprintf(stderr, "local_max alpha = %f\n", alpha);
        if (available_move.size() == 0) {
            // fprintf(stderr, "no available_move\n");
        } else {
            Nega_Board::piece now_piece = board->piece_list[dice];
            Nega_Board::move next_move = available_move[0];

            Nega_Node next_nega;
            next_nega.board = this->board;
            turn == RED ? next_nega.turn = BLUE : next_nega.turn = RED;
            next_nega.side = side;

            this->board->do_move(next_move);
            next_nega.dice = 1;
            local_max = -1.0 * next_nega.ng_search(depth + 1, limit, (-1.0) * beta, (-1.0) * alpha);
            best_move = next_move;
            this->board->undo_move();
        }
        for (int i = 0; i < available_move.size(); i++) {
            Nega_Board::piece now_piece = board->piece_list[dice];
            Nega_Board::move next_move = available_move[i];

            Nega_Node next_nega;
            next_nega.board = this->board;
            turn == RED ? next_nega.turn = BLUE : next_nega.turn = RED;
            next_nega.side = side;

            this->board->do_move(next_move);
            // next_nega.dice = 1;
            // local_max = -1.0 * next_nega.ng_search(depth + 1, limit, (-1.0) * beta, (-1.0) *
            // alpha); best_move = next_move;
            for (int next_dice = 1; next_dice <= 6; next_dice++) {
                next_nega.dice = next_dice;
                double temp =
                    -1.0 * next_nega.ng_search(depth + 1, limit, (-1.0) * beta, (-1.0) * alpha);
                if (temp > local_max) {
                    // fprintf(stderr, "new local\n", local_max);
                    local_max = temp;
                    best_move = next_move;
                }
                // fprintf(stderr, "temp = %f\n", temp);
            }
            this->board->undo_move();
        }
        // fprintf(stderr, "local_max = %f\n", local_max);
        return local_max;
    };

    int find_available_move() {
        // if (this->turn == BLUE) {
        //     cerr << "blue" << std::endl;
        // } else if (this->turn == RED) {
        //     cerr << "red" << std::endl;
        // }

        // fprintf(stderr, "this->dice = %d\n", this->dice);
        int temp = dice;  // 紀錄一下dice 等下可以還回去
        // if (this->turn == BLUE) {
        //     if (this->board->piece_list[dice].exist == true) {
        //         fprintf(stderr, "this->dice exist\n");
        //     }
        // } else {
        //     if (this->board->piece_list[dice + 6].exist == true) {
        //         fprintf(stderr, "this->dice exist\n");
        //     }
        // }
        if (turn == BLUE) {
            if (this->board->piece_list[dice].exist == false) {
                // cerr << "this->dice not exist " << std::endl;
                for (int i = dice + 1; i <= 6; i++) {
                    if (this->board->piece_list[i].exist == true) {
                        dice = i;
                        break;
                    }
                }
                // cerr << "dice upper =  " << dice << std::endl;
                if (dice > temp) {
                    Nega_Board::piece now_piece = board->piece_list[dice];
                    Nega_Board::move next_move;
                    next_move.orgr = now_piece.row, next_move.orgc = now_piece.col;
                    next_move.move_num = dice;
                    if (now_piece.col > 0) {
                        // 往左
                        next_move.tarr = now_piece.row, next_move.tarc = now_piece.col - 1;
                        if (now_piece.row > 0) {
                            // 往左上
                            next_move.tarr--;
                            available_move.emplace_back(next_move);
                            // 往上
                            next_move.tarc++;
                            available_move.emplace_back(next_move);
                            next_move.tarr++, next_move.tarc--;
                        }
                        available_move.emplace_back(next_move);
                    } else if (now_piece.row > 0) {
                        // 往上
                        next_move.tarr = now_piece.row - 1, next_move.tarc = now_piece.col;
                        available_move.emplace_back(next_move);
                    }
                }
                for (int i = dice - 1; i >= 0; i--) {
                    if (this->board->piece_list[i].exist == true) {
                        dice = i;
                        break;
                    }
                }
                // cerr << "dice below =  " << dice << std::endl;
                if (dice < temp) {
                    Nega_Board::piece now_piece = board->piece_list[dice];
                    Nega_Board::move next_move;
                    next_move.orgr = now_piece.row, next_move.orgc = now_piece.col;
                    next_move.move_num = dice;
                    if (now_piece.col > 0) {
                        // 往左
                        next_move.tarr = now_piece.row, next_move.tarc = now_piece.col - 1;

                        if (now_piece.row > 0) {
                            // 往左上
                            next_move.tarr--;
                            available_move.emplace_back(next_move);
                            // 往上
                            next_move.tarc++;
                            available_move.emplace_back(next_move);
                            next_move.tarr++, next_move.tarc--;
                        }
                        available_move.emplace_back(next_move);
                    } else if (now_piece.row > 0) {
                        // 往上
                        next_move.tarr = now_piece.row - 1, next_move.tarc = now_piece.col;
                        available_move.emplace_back(next_move);
                    }
                }
            } else {
                Nega_Board::piece now_piece = board->piece_list[dice];
                Nega_Board::move next_move;
                next_move.orgr = now_piece.row, next_move.orgc = now_piece.col;
                next_move.move_num = dice;
                if (now_piece.col > 0) {
                    // 往左
                    next_move.tarr = now_piece.row, next_move.tarc = now_piece.col - 1;
                    if (now_piece.row > 0) {
                        // 往左上
                        next_move.tarr--;
                        available_move.emplace_back(next_move);
                        // 往上
                        next_move.tarc++;
                        available_move.emplace_back(next_move);
                        next_move.tarr++, next_move.tarc--;
                    }
                    available_move.emplace_back(next_move);
                } else if (now_piece.row > 0) {
                    // 往上
                    next_move.tarr = now_piece.row - 1, next_move.tarc = now_piece.col;
                    available_move.emplace_back(next_move);
                }
            }
        } else if (turn == RED) {
            if (this->board->piece_list[dice + 6].exist == false) {
                for (int i = dice + 6 + 1; i <= 12; i++) {
                    if (this->board->piece_list[i].exist == true) {
                        dice = i - 6;
                        break;
                    }
                }
                if (dice > temp) {
                    Nega_Board::piece now_piece = board->piece_list[dice + 6];
                    Nega_Board::move next_move;
                    next_move.orgr = now_piece.row, next_move.orgc = now_piece.col;
                    next_move.move_num = dice + 6;
                    if (now_piece.col < BOARD_SIZE - 1) {
                        // 往右
                        next_move.tarr = now_piece.row, next_move.tarc = now_piece.col + 1;

                        if (now_piece.row < BOARD_SIZE - 1) {
                            // 往右下
                            next_move.tarr++;
                            available_move.emplace_back(next_move);
                            // 往下
                            next_move.tarc--;
                            available_move.emplace_back(next_move);
                            next_move.tarr--, next_move.tarc++;
                        }

                        available_move.emplace_back(next_move);
                    } else if (now_piece.row < BOARD_SIZE - 1) {
                        // 往下
                        next_move.tarr = now_piece.row + 1, next_move.tarc = now_piece.col;
                        available_move.emplace_back(next_move);
                    }
                }
                for (int i = dice + 6 - 1; i >= 7; i--) {
                    if (this->board->piece_list[i].exist == true) {
                        dice = i - 6;
                        break;
                    }
                }
                if (dice < temp) {
                    Nega_Board::piece now_piece = board->piece_list[dice + 6];
                    Nega_Board::move next_move;
                    next_move.orgr = now_piece.row, next_move.orgc = now_piece.col;
                    next_move.move_num = dice + 6;
                    if (now_piece.col < BOARD_SIZE - 1) {
                        // 往右
                        next_move.tarr = now_piece.row, next_move.tarc = now_piece.col + 1;
                        if (now_piece.row < BOARD_SIZE - 1) {
                            // 往右下
                            next_move.tarr++;
                            available_move.emplace_back(next_move);
                            // 往下
                            next_move.tarc--;
                            available_move.emplace_back(next_move);
                            next_move.tarr--, next_move.tarc++;
                        }

                        available_move.emplace_back(next_move);
                    } else if (now_piece.row < BOARD_SIZE - 1) {
                        // 往下
                        next_move.tarr = now_piece.row + 1, next_move.tarc = now_piece.col;
                        available_move.emplace_back(next_move);
                    }
                }
            } else {
                Nega_Board::piece now_piece = board->piece_list[dice + 6];
                Nega_Board::move next_move;
                next_move.orgr = now_piece.row, next_move.orgc = now_piece.col;
                next_move.move_num = dice + 6;
                if (now_piece.col < BOARD_SIZE - 1) {
                    // 往右
                    next_move.tarr = now_piece.row, next_move.tarc = now_piece.col + 1;
                    if (now_piece.row < BOARD_SIZE - 1) {
                        // 往右下
                        next_move.tarr++;
                        available_move.emplace_back(next_move);
                        // 往下
                        next_move.tarc--;
                        available_move.emplace_back(next_move);
                        next_move.tarr--, next_move.tarc++;
                    }

                    available_move.emplace_back(next_move);
                } else if (now_piece.row < BOARD_SIZE - 1) {
                    // 往下
                    next_move.tarr = now_piece.row + 1, next_move.tarc = now_piece.col;
                    available_move.emplace_back(next_move);
                }
            }
            dice = temp;
        }
        return 0;
    };

   private:
    // 審局	void update_available(char *move); // 更新可用步
    // 只要拿掉舊的塞入新的就好 這樣更新起來比較省 (最多也就6顆棋走3個方向
    // 直接開最大Array 18種走步)
    double evaluate() {
        // 參考paper An offensive and defensive expect minimax algorithm in EinStein Würfelt Nicht!
        // https://sci-hub.se/10.1109/ccdc.2015.7161846
        // 拆分三個績效factor 1. attack 2. block 3. threaten
        // 四個係數 1.corner distance 2.piece probability 3. piece value 4.weight
        double attack_score = 0, block_score = 0;
        double red_attack_score = 0, blue_attack_score = 0;
        // fprintf(stderr, "start evaluate: \n");
        for (int i = 7; i <= 12; i++) {  // 紅攻擊
            double probability = 1.0;
            if (this->board->piece_list[i].exist) {
                for (int j = i - 1; j >= 7; j--) {
                    if (this->board->piece_list[j].exist) {
                        break;
                    } else {
                        probability += 1.0;
                    }
                }
                for (int j = i + 1; j <= 12; j++) {
                    if (this->board->piece_list[j].exist) {
                        break;
                    } else {
                        probability += 1.0;
                    }
                }
                probability /= 6;
                // fprintf(stderr, "red pro = %f, piece value i = %d, v =
                // %f\n",probability,i,this->board->piece_list[i].piece_value);
                red_attack_score += probability * this->board->piece_list[i].piece_value;
            }
        }
        // red_attack_score /= 6;

        for (int i = 1; i <= 6; i++) {  // 藍攻擊
            double probability = 1.0;
            if (this->board->piece_list[i].exist) {
                for (int j = i - 1; j >= 1; j--) {
                    if (this->board->piece_list[j].exist) {
                        break;
                    } else {
                        probability += 1.0;
                    }
                }
                for (int j = i + 1; j <= 6; j++) {
                    if (this->board->piece_list[j].exist) {
                        break;
                    } else {
                        probability += 1.0;
                    }
                }
                probability /= 6;
                // fprintf(stderr, "blue pro = %f, piece value i = %d, v =
                // %f\n",probability,i,this->board->piece_list[i].piece_value);
                blue_attack_score += probability * this->board->piece_list[i].piece_value;
            }
        }
        // blue_attack_score /= 6;

        double threaten_score = 0;
        if (this->turn == RED) {
            for (int i = 7; i <= 12; i++) {
                if (!this->board->piece_list[i].exist) {
                    continue;
                }
                double prob = 1;
                for (int j = i - 1; j >= 7; j++) {
                    if (this->board->piece_list[j].exist) {
                        break;
                    } else {
                        prob += 1;
                    }
                }
                for (int j = i + 1; j <= 12; j++) {
                    if (this->board->piece_list[j].exist) {
                        break;
                    } else {
                        prob += 1;
                    }
                }
                int now_row = this->board->piece_list[i].row;
                int now_col = this->board->piece_list[i].col;
                if (now_row + 1 < BOARD_SIZE) {
                    if (now_col + 1 < BOARD_SIZE) {
                        if (this->board->ng_board[now_row + 1][now_col + 1] > 0 &&
                            this->board->ng_board[now_row + 1][now_col + 1] <= 6) {
                            int enemy_num = this->board->ng_board[now_row + 1][now_col + 1];
                            threaten_score += prob * this->board->piece_list[enemy_num].piece_value;
                        }
                    }
                    if (this->board->ng_board[now_row + 1][now_col] > 0 &&
                        this->board->ng_board[now_row + 1][now_col] <= 6) {
                        int enemy_num = this->board->ng_board[now_row + 1][now_col];
                        threaten_score += prob * this->board->piece_list[enemy_num].piece_value;
                    }
                } else if (now_col + 1 < BOARD_SIZE) {
                    if (this->board->ng_board[now_row][now_col + 1] > 0 &&
                        this->board->ng_board[now_row][now_col + 1] <= 6) {
                        int enemy_num = this->board->ng_board[now_row][now_col + 1];
                        threaten_score += prob * this->board->piece_list[enemy_num].piece_value;
                    }
                }
            }
        } else {  // turn == BLUE
            for (int i = 1; i <= 6; i++) {
                if (!this->board->piece_list[i].exist) {
                    continue;
                }
                double prob = 1;
                for (int j = i - 1; j >= 1; j++) {
                    if (this->board->piece_list[j].exist) {
                        break;
                    } else {
                        prob += 1;
                    }
                }
                for (int j = i + 1; j <= 6; j++) {
                    if (this->board->piece_list[j].exist) {
                        break;
                    } else {
                        prob += 1;
                    }
                }
                int now_row = this->board->piece_list[i].row;
                int now_col = this->board->piece_list[i].col;
                if (now_row - 1 >= 0) {
                    if (now_col - 1 >= 0) {
                        if (this->board->ng_board[now_row - 1][now_col - 1] > 6 &&
                            this->board->ng_board[now_row - 1][now_col - 1] <= 12) {
                            int enemy_num = this->board->ng_board[now_row - 1][now_col - 1];
                            threaten_score += prob * this->board->piece_list[enemy_num].piece_value;
                        }
                    }
                    if (this->board->ng_board[now_row - 1][now_col] > 6 &&
                        this->board->ng_board[now_row - 1][now_col] <= 12) {
                        int enemy_num = this->board->ng_board[now_row - 1][now_col];
                        threaten_score += prob * this->board->piece_list[enemy_num].piece_value;
                    }
                } else if (now_col - 1 >= 0) {
                    if (this->board->ng_board[now_row][now_col - 1] > 6 &&
                        this->board->ng_board[now_row][now_col - 1] <= 12) {
                        int enemy_num = this->board->ng_board[now_row][now_col - 1];
                        threaten_score += prob * this->board->piece_list[enemy_num].piece_value;
                    }
                }
            }
        }
        threaten_score /= 6;
        // fprintf(stderr, "evaluate red_a = %f, blue_a = %f, threaten = %f\n", red_attack_score,
        //         blue_attack_score, threaten_score);
        if (this->turn == BLUE) {
            return this->board->K1 * blue_attack_score - this->board->K2 * red_attack_score +
                   this->board->K3 * threaten_score;
        }
        if (this->turn == RED) {
            return this->board->K1 * red_attack_score - this->board->K2 * blue_attack_score +
                   this->board->K3 * threaten_score;
        }
        return 0;
    };
    int state() {  // 盤面狀態(結束沒)
        if (this->board->ng_board[0][0] > 0 && this->board->ng_board[0][0] < 7) {
            // fprintf(stderr, "blue reach: \n");
            return 1;  // 藍勝
        }
        if (this->board->red_exist_cnt == 0) {
            return 1;
        }
        if (this->board->ng_board[BOARD_SIZE - 1][BOARD_SIZE - 1] > 6) {
            // fprintf(stderr, "red reach: \n");
            // cerr<<"reach value = "
            // <<this->board->ng_board[BOARD_SIZE-1][BOARD_SIZE-1]<<std::endl; for(int
            // i=0;i<BOARD_SIZE;i++){
            //     fprintf(stderr, "----------%d  %d  %d  %d  %d
            //     -----\n",board->ng_board[i][0],board->ng_board[i][1],board->ng_board[i][2],board->ng_board[i][3],board->ng_board[i][4]);
            // }
            return 2;  // 紅勝
        }
        if (this->board->blue_exist_cnt == 0) {
            return 2;
        }
        return 0;
    };
};
