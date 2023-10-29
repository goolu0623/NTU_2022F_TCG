#include "MyAI.h"

#include <string>

#include "float.h"
#include "ngscout.h"
using std::stoi;
using std::string;

MyAI::MyAI(void) {}

MyAI::~MyAI(void) {}

void MyAI::Name(const char* data[], char* response) { strcpy(response, "r10922150:AI"); }

void MyAI::Version(const char* data[], char* response) { strcpy(response, "1.0.0"); }

void MyAI::Time_setting(const char* data[], char* response) {
    this->red_time = stoi(data[1]);
    this->blue_time = stoi(data[1]);
    strcpy(response, "1");
}

void MyAI::Board_setting(const char* data[], char* response) {
    this->board_size = stoi(data[1]);
    this->red_piece_num = stoi(data[2]);
    this->blue_piece_num = stoi(data[2]);
    strcpy(response, "1");
}

void MyAI::Ini(const char* data[], char* response) {
    // set color
    if (!strcmp(data[1], "R")) {
        this->color = RED;
    } else if (!strcmp(data[1], "B")) {
        this->color = BLUE;
    }

    char position[15];
    this->Init_board_state(position);

    sprintf(response, "%c%c %c%c %c%c %c%c %c%c %c%c", position[0], position[1], position[2],
            position[3], position[4], position[5], position[6], position[7], position[8],
            position[9], position[10], position[11]);
}

void MyAI::Get(const char* data[], char* response) {
    // set color 你現在的顏色
    if (!strcmp(data[1], "R")) {
        this->color = RED;
    } else if (!strcmp(data[1], "B")) {
        this->color = BLUE;
    }

    // set dice & board
    this->dice = stoi(data[2]);  // 現在骰子骰到的隨機數字
    char position[25];
    // 前六藍棋 後六紅棋
    sprintf(position, "%s%s%s%s%s%s%s%s%s%s%s%s", data[3], data[4], data[5], data[6], data[7],
            data[8], data[9], data[10], data[11], data[12], data[13], data[14]);
    this->Set_board(position);

    // generate move
    char move[4];
    this->Generate_move(move);
    sprintf(response, "%c%c %c%c", move[0], move[1], move[2], move[3]);
}

void MyAI::Exit(const char* data[], char* response) { fprintf(stderr, "Bye~\n"); }

// *********************** AI FUNCTION *********************** //

void MyAI::Init_board_state(char* position) {
    int order[PIECE_NUM] = {0, 1, 2, 3, 4, 5};
    string red_init_position = "A1B1C1A2B2A3";
    string blue_init_position = "E3D4E4C5D5E5";

    // assign the initial positions of your pieces in random order
    for (int i = 0; i < PIECE_NUM; i++) {
        int j = rand() % (PIECE_NUM - i) + i;
        int tmp = order[i];
        order[i] = order[j];
        order[j] = tmp;
    }

    for (int i = 0; i < PIECE_NUM; i++) {
        if (this->color == RED) {
            position[order[i] * 2] = red_init_position[i * 2];
            position[order[i] * 2 + 1] = red_init_position[i * 2 + 1];
        } else if (this->color == BLUE) {
            position[order[i] * 2] = blue_init_position[i * 2];
            position[order[i] * 2 + 1] = blue_init_position[i * 2 + 1];
        }
    }
}

void MyAI::Set_board(char* position)
{
	memset(this->board, 0, sizeof(this->board));
	memset(this->blue_exist, 1, sizeof(this->blue_exist));
	memset(this->red_exist, 1, sizeof(this->red_exist));
	this->blue_piece_num = PIECE_NUM;
	this->red_piece_num = PIECE_NUM;

	int lost_piece_num = 0;
	for(int i = 0; i < PIECE_NUM * 2; i++)
	{
		int index = i * 2 - lost_piece_num;

		// the piece does not exist
		while(position[index] == '0')
		{
			index = i * 2 - lost_piece_num + 1;
			lost_piece_num++;
			// blue
			if(i < PIECE_NUM) 
			{
				this->blue_piece_num --;
				this->blue_exist[i] = 0;
			}
			// red
			else 
			{
				this->red_piece_num --;
				this->red_exist[i - PIECE_NUM] = 0;
			}
			i += 1;
		}
		// 1~6: blue pieces; 7~12: red pieces
		if (i < PIECE_NUM * 2)
		{
			this->board[position[index + 1] - '1'][position[index] - 'A'] = i + 1;
		}
	}
	fprintf(stderr, "\nThe current board:\n");
	this->Print_chessboard();
}

void MyAI::Print_chessboard() {
    fprintf(stderr, "\n");
    // 1~6 represent blue pieces; A~F represent red pieces
    for (int i = 0; i < BOARD_SIZE; i++) {
        fprintf(stderr, "<%d>   ", i + 1);
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (this->board[i][j] <= PIECE_NUM)
                fprintf(stderr, "%d  ", this->board[i][j]);
            else
                fprintf(stderr, "%c  ", 'A' + (this->board[i][j] - 7));
        }
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n     ");
    for (int i = 0; i < BOARD_SIZE; i++) {
        fprintf(stderr, "<%c>", 'A' + i);
    }
    fprintf(stderr, "\n\n");
    fprintf(stderr, "The number of blue pieces: %d\nThe number of red pieces: %d\n",
            this->blue_piece_num, this->red_piece_num);
}

// void MyAI::Generate_move(char* move)
// {
// 	int result[100]; // 這一百是不是亂開的??? 感覺是吧 隨便開一個絕對不會超過的陣列來存
// 	// get legal moves
// 	int move_count = this->get_legal_move(result);
// 	// randomly choose a legal move
// 	int rand_move = rand() % move_count;
// 	int piece = result[rand_move * 3];
// 	int start_point = result[rand_move * 3 + 1];
// 	int end_point = result[rand_move * 3 + 2];
// 	sprintf(move, "%c%c%c%c", 'A' + start_point % BOARD_SIZE, '1' + start_point / BOARD_SIZE,
// 'A' + end_point % BOARD_SIZE, '1' + end_point / BOARD_SIZE); 	this->Make_move(piece,
// start_point, end_point);
// 	// print the result
// 	fprintf(stderr, "============================\nMy result:\n");
// 	if(piece <= PIECE_NUM) fprintf(stderr, "Blue piece %d: (%c%c) -> (%c%c)\n", piece, move[0],
// move[1], move[2], move[3]); 	else fprintf(stderr, "Red piece %d: (%c%c) -> (%c%c)\n", piece -
// PIECE_NUM, move[0], move[1], move[2], move[3]); 	this->Print_chessboard();
// fprintf(stderr,
// "============================\n");
// }

void MyAI::Generate_move(char* move) {
    Nega_Board ng_board;
    Nega_Node ng_scout;
    // 先吃成我自己的盤面結構
    // fprintf(stderr, "generate start: \n");
    ng_board.red_exist_cnt = 0, ng_board.blue_exist_cnt = 0;
    for (int i = 1; i <= 12; i++) {
        ng_board.piece_list[i].exist = false;  // 要預設 false 不然有時候他剛好預設是true會噴error
    }
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            int piece_num = this->board[i][j];
            ng_board.ng_board[i][j] = piece_num;
            if (piece_num > 0) {
                ng_board.piece_list[piece_num].num = piece_num;
                ng_board.piece_list[piece_num].row = i;
                ng_board.piece_list[piece_num].col = j;
                ng_board.piece_list[piece_num].exist = true;
                piece_num > 6 ? ng_board.red_exist_cnt++ : ng_board.blue_exist_cnt++;
            }
        }
    }
    // 算piece value跟distance
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            int piece_num = ng_board.ng_board[i][j];
            if (piece_num > 6) {
                // 紅色
                ng_board.piece_list[piece_num].piece_distance = std::min(
                    ng_board.piece_list[piece_num].row, ng_board.piece_list[piece_num].col);
                ng_board.piece_list[piece_num].piece_value =
                    std::pow(ng_board.N, ng_board.piece_list[piece_num].piece_distance);

                // cerr << "piece_num=" << piece_num
                //      << ", piece_distance= " << ng_board.piece_list[piece_num].piece_distance
                //      << ", piece_value= " << ng_board.piece_list[piece_num].piece_value
                //      << std::endl;
            } else if (piece_num > 0) {
                // 藍色
                ng_board.piece_list[piece_num].piece_distance =
                    4 - std::max(ng_board.piece_list[piece_num].row,
                                 ng_board.piece_list[piece_num].col);
                ng_board.piece_list[piece_num].piece_value =
                    std::pow(ng_board.N, ng_board.piece_list[piece_num].piece_distance);
                // cerr << "piece_num=" << piece_num
                //      << ", piece_distance= " << ng_board.piece_list[piece_num].piece_distance
                //      << ", piece_value= " << ng_board.piece_list[piece_num].piece_value
                //      << std::endl;
            }
        }
    }
    // for (int i = 0; i < BOARD_SIZE; i++) {
        // fprintf(stderr, "----------%d  %d  %d  %d  %d -----\n", ng_board.ng_board[i][0],
        //         ng_board.ng_board[i][1], ng_board.ng_board[i][2], ng_board.ng_board[i][3],
        //         ng_board.ng_board[i][4]);
    // }
    // fprintf(stderr, "init board done: \n");
    // cerr << "init board done" << std::endl;
    if (this->color == RED) {
        ng_scout.side = 0;
        ng_scout.turn = 0;
    } else {
        ng_scout.side = 1;
        ng_scout.turn = 1;
    }
    ng_scout.dice = this->dice;
    ng_scout.board = &ng_board;
    // cerr << "this color = " << ng_scout.side << std::endl;
    double alpha = DBL_MIN, beta = DBL_MAX;
    double max_val = alpha;
    int dep = 0, limit = 1;
    // fprintf(stderr, "start search: \n");
    // cerr << "start search" << std::endl;
    double temp = ng_scout.ng_search(dep, limit, alpha, beta);
    // cerr << "end search, score = " << temp << std::endl;
    // fprintf(stderr, "end search   score:%f\n", temp);
    // fprintf(stderr, "fku\n");
    Nega_Board::move best_move = ng_scout.best_move;
    // int board[100];
    // for (int i = 0; i < move_count; i++) {
    //     // do move
    //     double now_val = ngsearch(board, dep, limit, (-1) * beta, (-1) * alpha);
    //     if (now_val > max_val) {
    //         now_val = max_val;
    //     }
    //     // undo move
    // }
    // return highest value move here

    // alpha beta
    // for all move
    // do_move
    // search
    // undo_move
    // return max value move

    int result[100];  // 這一百是不是亂開的??? 感覺是吧 隨便開一個絕對不會超過的陣列來存

    // get legal moves
    int move_count = this->get_legal_move(result);

    // randomly choose a legal move
    int rand_move = rand() % move_count;
    int piece = result[rand_move * 3];
    int start_point = result[rand_move * 3 + 1];
    int end_point = result[rand_move * 3 + 2];
    bool legal = false;
    int my_start_point = best_move.orgr * 5 + best_move.orgc;
    int my_end_point = best_move.tarr * 5 + best_move.tarc;
    // fprintf(stderr, "org(r,c) = (%d,%d)\n",best_move.orgr,
    //         best_move.orgc);
    int my_piece = ng_scout.board->ng_board[best_move.orgr][best_move.orgc];
    // if (this->color==RED) {
    //     my_piece -= 6;
    // }
    for (int i = 0; i < move_count; i++) {
        if (result[i * 3] == my_piece && result[i * 3 + 1] == my_start_point &&
            result[i * 3 + 2] == my_end_point) {
            legal = true;
            break;
        }
    }
    // legal = true;
    if (legal) {
        sprintf(move, "%c%c%c%c", 'A' + my_start_point % BOARD_SIZE,
                '1' + my_start_point / BOARD_SIZE, 'A' + my_end_point % BOARD_SIZE,
                '1' + my_end_point / BOARD_SIZE);
        this->Make_move(my_piece, my_start_point, my_end_point);
    } else {
        sprintf(move, "%c%c%c%c", 'A' + start_point % BOARD_SIZE, '1' + start_point / BOARD_SIZE,
                'A' + end_point % BOARD_SIZE, '1' + end_point / BOARD_SIZE);
        this->Make_move(piece, start_point, end_point);
        // fprintf(stderr, "illegal!\n");
    }

    // fprintf(stderr, "rand= %d  %d  %d\n", start_point, end_point, piece);
    // fprintf(stderr, "my  = %d  %d  %d\n", my_start_point, my_end_point, my_piece);
    // fprintf(stderr, "org(r,c) = (%d,%d), tar(r,c) = (%d,%d), dice = %d\n", best_move.orgr,
    //         best_move.orgc, best_move.tarr, best_move.tarc, this->dice);

    // print para
    fprintf(stderr, "search_cnt = %d, (N,k1,k2,k3) = (%f,%d,%d,%d)\n",ng_board.search_cnt,ng_board.N,ng_board.K1,ng_board.K2,ng_board.K3);
    // print the result
    fprintf(stderr, "============================v==\nMy result:\n");
    if (piece <= PIECE_NUM)
        fprintf(stderr, "Blue piece %d: (%c%c) -> (%c%c)\n", piece, move[0], move[1], move[2],
                move[3]);
    else
        fprintf(stderr, "Red piece %d: (%c%c) -> (%c%c)\n", piece - PIECE_NUM, move[0], move[1],
                move[2], move[3]);
    this->Print_chessboard();
    fprintf(stderr, "============================\n");
}

// get all legal moves
int MyAI::get_legal_move(int* result) {
    int src, dst[3];
    int movable_piece;
    int move_count = 0;
    int result_count = 0;

    if (this->color == BLUE) {
        // the corresponding piece is alive
        if (this->blue_exist[this->dice - 1]) {
            movable_piece = this->dice;
            move_count = this->referee(movable_piece, &src, dst);
            for (int i = result_count; i < result_count + move_count; i++) {
                result[i * 3] = movable_piece;
                result[i * 3 + 1] = src;
                result[i * 3 + 2] = dst[i];
            }
            result_count += move_count;
        }
        // the corresponding piece does not exist
        else {
            // seeking for the next-higher piece
            for (int i = this->dice; i <= PIECE_NUM; ++i) {
                if (this->blue_exist[i - 1]) {
                    movable_piece = i;
                    move_count = this->referee(movable_piece, &src, dst);
                    int index = 0;
                    for (int j = result_count; j < result_count + move_count; j++, index++) {
                        result[j * 3] = movable_piece;
                        result[j * 3 + 1] = src;
                        result[j * 3 + 2] = dst[index];
                    }
                    result_count += move_count;
                    break;
                }
            }
            // seeking for the next-lower piece
            for (int i = this->dice; i >= 1; --i) {
                if (this->blue_exist[i - 1]) {
                    movable_piece = i;
                    move_count = this->referee(movable_piece, &src, dst);
                    int index = 0;
                    for (int j = result_count; j < result_count + move_count; j++, index++) {
                        result[j * 3] = movable_piece;
                        result[j * 3 + 1] = src;
                        result[j * 3 + 2] = dst[index];
                    }
                    result_count += move_count;
                    break;
                }
            }
        }
    }

    else if (this->color == RED) {
        // the corresponding piece is alive
        if (this->red_exist[this->dice - 1]) {
            movable_piece = this->dice + PIECE_NUM;
            move_count = this->referee(movable_piece, &src, dst);
            for (int i = result_count; i < result_count + move_count; i++) {
                result[i * 3] = movable_piece;
                result[i * 3 + 1] = src;
                result[i * 3 + 2] = dst[i];
            }
            result_count += move_count;
        }
        // the corresponding piece does not exist
        else {
            // seeking for the next-higher piece
            for (int i = this->dice; i <= PIECE_NUM; ++i) {
                if (this->red_exist[i - 1]) {
                    movable_piece = i + PIECE_NUM;
                    move_count = this->referee(movable_piece, &src, dst);
                    int index = 0;
                    for (int j = result_count; j < result_count + move_count; j++, index++) {
                        result[j * 3] = movable_piece;
                        result[j * 3 + 1] = src;
                        result[j * 3 + 2] = dst[index];
                    }
                    result_count += move_count;
                    break;
                }
            }
            // seeking for the next-lower piece
            for (int i = this->dice; i >= 1; --i) {
                if (this->red_exist[i - 1]) {
                    movable_piece = i + PIECE_NUM;
                    move_count = this->referee(movable_piece, &src, dst);
                    int index = 0;
                    for (int j = result_count; j < result_count + move_count; j++, index++) {
                        result[j * 3] = movable_piece;
                        result[j * 3 + 1] = src;
                        result[j * 3 + 2] = dst[index];
                    }
                    result_count += move_count;
                    break;
                }
            }
        }
    }
    return result_count;
}

// get possible moves of the piece
int MyAI::referee(int piece, int* src, int* dst) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (this->board[i][j] == piece) {
                *src = i * BOARD_SIZE + j;
            }
        }
    }
    // blue piece
    if (piece <= PIECE_NUM) {
        // the piece is on the leftmost column
        if (*src % BOARD_SIZE == 0) {
            dst[0] = *src - BOARD_SIZE;  // up
            return 1;
        }
        // the piece is on the uppermost row
        else if (*src < BOARD_SIZE) {
            dst[0] = *src - 1;  // left
            return 1;
        } else {
            dst[0] = *src - 1;               // left
            dst[1] = *src - BOARD_SIZE;      // up
            dst[2] = *src - BOARD_SIZE - 1;  // upper left
            return 3;
        }
    }

    // red piece
    else {
        // the piece is on the rightmost column
        if (*src % BOARD_SIZE == 4) {
            dst[0] = *src + BOARD_SIZE;  // down
            return 1;
        }
        // the piece is on the downmost row
        else if (*src >= BOARD_SIZE * (BOARD_SIZE - 1)) {
            dst[0] = *src + 1;  // right
            return 1;
        } else {
            dst[0] = *src + 1;               // right
            dst[1] = *src + BOARD_SIZE;      // down
            dst[2] = *src + BOARD_SIZE + 1;  // bottom right
            return 3;
        }
    }
}

void MyAI::Make_move(const int piece, const int start_point, const int end_point) {
    int start_row = start_point / BOARD_SIZE;
    int start_col = start_point % BOARD_SIZE;
    int end_row = end_point / BOARD_SIZE;
    int end_col = end_point % BOARD_SIZE;

    this->board[start_row][start_col] = 0;

    // there has another piece on the target sqaure
    if (this->board[end_row][end_col] > 0) {
        if (this->board[end_row][end_col] <= PIECE_NUM) {
            this->blue_exist[this->board[end_row][end_col] - 1] = 0;
            this->blue_piece_num--;
        } else {
            this->red_exist[this->board[end_row][end_col] - 7] = 0;
            this->red_piece_num--;
        }
    }
    this->board[end_row][end_col] = piece;
}