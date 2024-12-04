#include<iostream>
#include<stdlib.h>
#include<easyx.h>
#include<graphics.h>
#include<conio.h>
#include<limits.h>
#include<string.h>

#define GRID_SIZE 40
#define BOARD_SIZE 15
#define MAX_HISTORY 2
#define MAX_DEPTH 2
#define _CRT_SECURE_NO_WARNINGS 1

/***第一步获得棋盘和悔棋栈的函数***/

typedef struct {//struct的简便形式，声明可以直接用
	int board[BOARD_SIZE][BOARD_SIZE];//棋盘状态
	int turn;//当前回合：1人，0机器
	int gameOver;//游戏是否结束
	int winner;//赢家：1黑，-1白，0无胜者
	int playerchess;
	int aichess;
}GameState;//游戏状态

typedef struct {
	GameState history[MAX_HISTORY];
	int top;
}UndoStack;//悔棋栈

void menu() {//主菜单
	printf("New Game:press N\n");
	printf("Continue Game:press C\n");
}
void sidemenu() {//副菜单
	printf("Exit and save:press E\n");
	printf("Undo:press U\n");
}
void chooseChess(GameState *game) {//选棋子
	printf("Choose your chess\n");
	printf("Black: press 1\n");
	printf("White: press 2\n");
	std::cin.ignore();
	char key;
	scanf_s("%c", &key,1);
	if (key == '1') {
		game->playerchess = 1;
		game->aichess = -1;
	}
	else if (key == '2') {
		game->playerchess = -1;
		game->aichess = 1;
	}
	else {
		printf("invalid input,try again\n");
		chooseChess(game);
	}
}

void initGame(GameState* game) {
	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			game->board[i][j] = 0;
		}
	}
	game->turn = 0;//->是指向结构体成员
	game->gameOver = 0;
	game->winner = 0;
	game->playerchess = 0;
	game->aichess = 0;
}//初始化游戏状态

void initUndoStack(UndoStack* stack) {
	stack->top = -1;
}//初始化悔棋栈

void pushUndo(UndoStack* stack, GameState* game) {
	if (stack->top >= MAX_HISTORY - 1) {
		// 如果堆栈已满，移除最早的一次记录
		for (int i = 0; i < MAX_HISTORY - 1; i++) {
			stack->history[i] = stack->history[i + 1];
		}
		stack->history[MAX_HISTORY - 1] = *game;
	}
	else {//悔棋至多回溯3次，可以在后续流程中添加提醒
		stack->history[++stack->top] = *game;
	}
}
void popUndo(UndoStack* stack, GameState* game) {
	if (stack->top <= MAX_HISTORY - 1 && stack->top > 0) {
		*game = stack->history[stack->top--];
		
	}
	else {
		*game = stack->history[MAX_HISTORY - 2];
	}
}//悔棋检测并取出

/***第二步绘制图形界面的函数***/

void initWindow() {
	initgraph(GRID_SIZE * BOARD_SIZE, GRID_SIZE * BOARD_SIZE);
	setbkcolor(0x96DED1);//美学不能输之莫兰迪蓝
	cleardevice();
}
//初始化界面

void drawBoard() {
	setlinecolor(0x008080);
	for (int i = 0; i < BOARD_SIZE; i++) {
		//绘制水平线
		line(GRID_SIZE / 2, GRID_SIZE / 2 + i * GRID_SIZE,
			GRID_SIZE / 2 + (BOARD_SIZE - 1) * GRID_SIZE, GRID_SIZE / 2 + i * GRID_SIZE);
		// 绘制垂直线
		line(GRID_SIZE / 2 + i * GRID_SIZE, GRID_SIZE / 2,
			GRID_SIZE / 2 + i * GRID_SIZE, GRID_SIZE / 2 + (BOARD_SIZE - 1) * GRID_SIZE);
	}
	
}
//画出界面

bool getBoardPosition(int mouseX, int mouseY, int* row, int* col,GameState* game) {
	*col = (mouseX - GRID_SIZE / 2 + GRID_SIZE / 2) / GRID_SIZE;
	*row = (mouseY - GRID_SIZE / 2 + GRID_SIZE / 2) / GRID_SIZE;
	if (*col >= 0 && *col < BOARD_SIZE && *row >= 0 && *row < BOARD_SIZE&&!game->board[*row][*col]) {
		return true;
	}
	return false;
}
//判断落棋位置是否可用

void drawChess(int row, int col, int player) {
	int centerX = GRID_SIZE / 2 + col * GRID_SIZE;
	int centerY = GRID_SIZE / 2 + row * GRID_SIZE;
	if (player == 1)
		setfillcolor(BLACK);
	else if(player==-1)
		setfillcolor(WHITE);
	solidcircle(centerX, centerY, GRID_SIZE / 3);

}
//画棋子的步骤

void previouschess(GameState &game) {
	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			if (game.board[i][j]) {
				drawChess(i, j, game.board[i][j]);
			}
		}
	}
}

/***第三步保存游戏函数***/

void saveGame(GameState* game, const char* filename) {
	FILE* file;
	errno_t err;
	err= fopen_s(&file,filename, "w");//以写入方式打开文件filename
	if (err != 0) {
		fclose(file);
		printf("找不到文件%s\n", filename);
		return;
	}
	else {
		fwrite(game, sizeof(GameState), 1, file);
		fclose(file);
		printf("游戏保存在%s成功\n", filename);
	}
}//保存游戏

void loadGame(GameState* game, const char* filename) {
	FILE* file;
	errno_t err;
	err = fopen_s(&file, filename, "r");//以读取方式打开文件filename
	if (err != 0) {
		fclose(file);
		printf("加载失败%s\n", filename);
		return;
	}
	else {
		fread(game, sizeof(GameState), 1, file);
		fclose(file);
		printf("游戏已从 %s 恢复\n", filename);
	}
}//游戏加载

/***第四步胜负逻辑检测***/

int checkWinner(int board[BOARD_SIZE][BOARD_SIZE], int x, int y) {//一共八个方向，分成两半分别计算count
	int directions[4][2] = { {0, 1}, {1, 0}, {1, 1}, {1, -1} };
	int current = board[x][y];//针对当前下棋者进行判断
	for (int d = 0; d < 4; d++) {
		int count = 1;//四个方向，检查到了就停止
		for (int step = 1; step < 5; step++) {  // 检查正方向
			int nx = x + step * directions[d][0];
			int ny = y + step * directions[d][1];
			if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE && board[nx][ny] == current) {
				count++;
			}
			else {
				break;
			}
		}
		for (int step = 1; step < 5; step++) {  // 检查反方向
			int nx = x - step * directions[d][0];
			int ny = y - step * directions[d][1];
			if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE && board[nx][ny] == current) {
				count++;
			}
			else {
				break;
			}
		}
		if (count >= 5) {
			return current;  // 返回胜利方
		}
	}
	return 0;  // 没有胜者
}
/***第五步禁手检测***/
int countforchang(int board[BOARD_SIZE][BOARD_SIZE], int x, int y, int dx, int dy, int player) {
	int count = 1;//某个player在该点的落棋在某个特定方向上形成了多少个连续
	for (int i = 1; i < 6; i++) {  // 只考虑 6 连内的
		int nx = x + i * dx;
		int ny = y + i * dy;
		if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE && board[nx][ny] == player) {
			count++;
		}
		else {
			break;
		}
	}
	for (int i = 1; i < 6; i++) {  // 只考虑 6 连内的
		int nx = x - i * dx;
		int ny = y - i * dy;
		if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE && board[nx][ny] == player) {
			count++;
		}
		else {
			break;
		}
	}
	return count;
}//检查长连连续
bool changLian(int x, int y, GameState& game) {
	if (countforchang(game.board, x, y, 1, 0, 1) >= 6 ||
		countforchang(game.board, x, y, 0, 1, 1) >= 6 ||
		countforchang(game.board, x, y, 1, 1, 1) >= 6 ||
		countforchang(game.board, x, y, 1, -1, 1) >= 6) {
		return 1;
	}
	return 0;
}//检查长连
std::string chesslines(GameState& game, int x, int y, int dx, int dy) {
	std::string lines;
	lines.push_back('1');//中间项
	for (int i = 1; i < 15; i++) {
		int nx = x + i * dx;
		int ny = y + i * dy;
		if (nx < 15 && nx >= 0 && ny < 15 && ny >= 0) {
			if (game.board[nx][ny] >= 0) {
				lines.push_back(game.board[nx][ny] + '0');//后面加
			}
			else {
				lines.push_back('-');
				lines.push_back('1');
			}
		}
		else
			break;
	}
	for (int i = 1; i < 15; i++) {
		int nx = x - i * dx;
		int ny = y - i * dy;
		if (nx < 15 && nx >= 0 && ny < 15 && ny >= 0) {
			if (game.board[nx][ny] >= 0) {
				lines.insert(lines.begin(), game.board[nx][ny] + '0');//前面加
			}
			else {
				lines.insert(lines.begin(), '1');
				lines.insert(lines.begin(), '-');
			}
		}
		else
			break;
	}
	lines.insert(lines.begin(), '|');//棋盘边缘
	lines.push_back('|');
	return lines;
}//记录所有方向上的棋盘形成的线
std::string huosan[5] = { "010110","011010" ,"0011100" ,"001110-1" ,"-1011100" };
std::string huosi[12] = { "0111010","011101-1","-1111010","-111101-1","0101110","010111-1","-110111-1",
"-1101110","0110110","-111011-1","-1110110","011011-1" };
std::string huosino[5] = { "10111101","101111-1","101111|","-1111101","|111101" };
//所有的活三和活四集合
bool found(std::string chesslines, int start, std::string dict) {
	if (start >= chesslines.length() - 4)
		return false;
	if (start + dict.length() <= chesslines.length()) {
		if (chesslines.substr(start, dict.length()) == dict) {
			return true;
		}
	}
	return found(chesslines, start + 1, dict);
}//在棋盘线中找有没有对应的
bool san(int x, int y, GameState& game) {
	int count = 0;
	std::string lines[4];
	lines[0] = chesslines(game, x, y, 1, 0);
	lines[1] = chesslines(game, x, y, 0, 1);
	lines[2] = chesslines(game, x, y, 1, 1);
	lines[3] = chesslines(game, x, y, 1, -1);//导入棋盘线
	for (int i = 0; i < 4; i++) {
		if (found(lines[i], 0, huosan[0])) {
			count++;
		}
		if (found(lines[i], 0, huosan[1])) {
			count++;
		}
		if (found(lines[i], 0, huosan[2]) || found(lines[i], 0, huosan[3]) || found(lines[i], 0, huosan[4])) {
			count++;
		}
	}
	if (count >= 2)
		return true;
	else
		return false;
}//寻找有没有两个以上活三
bool si(int x, int y, GameState& game) {
	int count = 0;
	std::string lines[4];
	lines[0] = chesslines(game, x, y, 1, 0);
	lines[1] = chesslines(game, x, y, 0, 1);
	lines[2] = chesslines(game, x, y, 1, 1);
	lines[3] = chesslines(game, x, y, 1, -1);
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 12; j++) {
			if (found(lines[i], 0, huosi[j])) {
				count++;
			}
		}
	}
	if (count >= 2)
		return true;
	else
		return false;
}//寻找有没有两个以上活四

/***第六步简易ai下棋***/

int evaluate(GameState *game);//打分函数
int minimax(GameState *game, int depth, int isMaximizingPlayer);
int isMovesLeft(int board[BOARD_SIZE][BOARD_SIZE]);
int checkDirection(GameState *game, int x, int y, int dx, int dy, int player);
// 检查棋盘是否还有空位
int isMovesLeft(int board[BOARD_SIZE][BOARD_SIZE]) {
	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			if (board[i][j] == 0) //空位
				return 1;
		}
	}
	return 0;
}
#define WIN_SCORE 10000
#define FOUR_SCORE 1000
#define THREE_SCORE 100
#define TWO_SCORE 10

// 检查连续的棋子数（活的或死的）
int countConsecutive(int board[BOARD_SIZE][BOARD_SIZE], int x, int y, int dx, int dy, int player) {
	int count = 0;//某个player在该点的落棋在某个特定方向上形成了多少个连续
	for (int i = 0; i < 5; i++) {  // 只考虑 5 连内的
		int nx = x + i * dx;
		int ny = y + i * dy;
		if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE && board[nx][ny] == player) {
			count++;
		}
		else {
			break;
		}
	}
	return count;
}

// 评分函数
int evaluate(GameState *game) {
	int score = 0;

	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			if (game->board[i][j] != 0) {  // 只评分有棋子的位置
				int player = game->board[i][j];//对所有棋子进行分别评分

				// 四个方向检查：水平、垂直、两个对角线
				score += checkDirection(game, i, j, 1, 0, player);  // 水平
				score += checkDirection(game, i, j, 0, 1, player);  // 垂直
				score += checkDirection(game, i, j, 1, 1, player);  // 对角线 
				score += checkDirection(game, i, j, 1, -1, player); // 对角线
			}
		}
	}

	return score;
}

// 具体检查方向和返回分值
int checkDirection(GameState *game, int x, int y, int dx, int dy, int player) {
	int count = countConsecutive(game->board, x, y, dx, dy, player);
	//以下是某个方向上形成几联的评分,分别是ai加法和player减法
	if (player == game->aichess) {
		if (count == 5) return WIN_SCORE;   // 赢局
		if (count == 4) return FOUR_SCORE;  // 四连
		if (count == 3) return THREE_SCORE; // 三连
		if (count == 2) return TWO_SCORE;   // 两连
	}
	else{
		if (count == 5) return -WIN_SCORE;   // 赢局
		if (count == 4) return -FOUR_SCORE;  // 四连
		if (count == 3) return -THREE_SCORE; // 三连
		if (count == 2) return -TWO_SCORE;   // 两连
	}
	return 0;  // 无特别得分
}


// Minimax 实现
int minimax(GameState *game, int depth, int isMaximizingPlayer) {
	int score = evaluate(game);//给现在的局势打个分

	// 如果达到最大深度，或者接下来没有可以放棋子的地方了，返回现在的分值
	if (depth == MAX_DEPTH || !isMovesLeft(game->board)) {
		return score;
	}
	//递归2次给局势打分
	if (isMaximizingPlayer) {//ai的回合
		int maxEval = INT_MIN;
		for (int i = 0; i < BOARD_SIZE; i++) {
			for (int j = 0; j < BOARD_SIZE; j++) {
				if (game->board[i][j] == 0) {//ai此步棋希望以后局势分数越高越好
					game->board[i][j] = game->aichess;
					int eval = minimax(game, depth + 1, 0);
					game->board[i][j] = 0;
					maxEval = (eval > maxEval) ? eval : maxEval;
					
				}
			}
		}
		return maxEval;//在ai落子的所有情况中选取最高分的局势
	}
	else {//玩家的回合
		int minEval = INT_MAX;
		for (int i = 0; i < BOARD_SIZE; i++) {
			for (int j = 0; j < BOARD_SIZE; j++) {
				if (game->board[i][j] == 0) {//玩家希望此步棋以后局势分数越低越好（对ai不利）
					game->board[i][j] =game-> playerchess;
					int eval = minimax(game, depth + 1, 1);
					game->board[i][j] = 0;
					minEval = (eval < minEval) ? eval : minEval;
				
				}
			}
		}
		return minEval;//在所有玩家可能的落子中选取最低分的情况
	}//在所有玩家使自己有利的落子情况中，ai希望找到尽可能对自己有利的落子方案
}

// AI 查找最佳落子位置
void findBestMove(GameState *game, int* bestRow, int* bestCol) {
	int bestValue = INT_MIN;
	*bestRow = -1;
	*bestCol = -1;

	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {//遍历ai可以落子的所有地方，得到每个位置的未来最大分值
			if (game->board[i][j] == 0) {
				game->board[i][j] = game->aichess;  // AI 落子
				if (game->aichess == -1) {
					int moveValue = minimax(game, 1, 0);
					game->board[i][j] = 0;

					if (moveValue > bestValue) {//分值比现在的好就换
						*bestRow = i;
						*bestCol = j;
						bestValue = moveValue;
					}
				}
				else if (game->aichess == 1&&!changLian(i,j,*game)&&!san(i,j,*game)&&!si(i,j,*game)){
					int moveValue = minimax(game, 1, 0);
					game->board[i][j] = 0;

					if (moveValue > bestValue) {//分值比现在的好就换
						*bestRow = i;
						*bestCol = j;
						bestValue = moveValue;
					}
				}
				else {
					game->board[i][j] = 0;
				}
				

			}
		}
	}
}

/***第七步主函数游戏运行***/
void gameStart() {
	GameState game;
	UndoStack stack;
	initGame(&game);
	initUndoStack(&stack);
	chooseChess(&game);
	sidemenu();
	initWindow();
	drawBoard();
	if (game.playerchess == 1) {
		game.turn = 1;
	}
	else {
		game.turn = 0;
	}
	while (!game.gameOver) {
		if (_kbhit()) {
			char key = _getch();
			switch (key) {
			case 'E':
				saveGame(&game, "save.txt");
				break;
			case 'U':
				popUndo(&stack,&game);
			}
		}
		if (game.turn == 1) {
			if (MouseHit()) {
				MOUSEMSG msg = GetMouseMsg();
				int row, col;
				if (msg.uMsg == WM_LBUTTONDOWN) {
					if (game.playerchess == 1) {
						if (getBoardPosition(msg.x, msg.y, &row, &col, &game)) {
							game.board[row][col] = game.playerchess;
							if (!changLian(row, col, game) && !san(row, col, game) && !si(row, col, game)) {
								pushUndo(&stack, &game);
								drawChess(row, col, game.playerchess);
								game.winner = checkWinner(game.board, row, col);
								if (game.winner == 0)
									game.turn = 0;
								else if (game.winner ==game.playerchess) {
									printf("You win!");
									game.gameOver = 1;
									break;
								}
							}
							else{
								game.board[row][col] = 0;
								printf("You lose!Because you broke rules!\n");
								game.gameOver = 1;
								break;
							}
						}
						else {
							printf("invalid position,try again\n");
						}
					}
					else {
						if (getBoardPosition(msg.x, msg.y, &row, &col, &game)) {
							game.board[row][col] =game.playerchess;
							pushUndo(&stack, &game);
							drawChess(row, col, game.playerchess);
							game.winner = checkWinner(game.board, row, col);
							if (game.winner == 0)
								game.turn = 0;
							else if (game.winner == game.playerchess) {
								printf("You win!");
								game.gameOver = 1;
								break;
							}

						}
						else {
							printf("invalid position,try again\n");
						}
					}
				}
			}
		}
		if (game.turn == 0) {
			if (isMovesLeft(game.board)) {
				int bestrow, bestcol;
				findBestMove(&game, &bestrow, &bestcol);
				game.board[bestrow][bestcol] = game.aichess;
				drawChess(bestrow, bestcol, game.aichess);
				game.winner = checkWinner(game.board, bestrow, bestcol);
				if (game.winner == 0)
					game.turn = 1;
				else if (game.winner == game.aichess) {
					printf("You lose!");
					game.gameOver = 1;
					break;
				}
	
			}
			else {
				printf("there is no room to place chess,you have to start a new game.\n");
			}
		}
	}
	char a=getchar();
	closegraph();
}
void previousgame() {
	GameState game;
	UndoStack stack;
	loadGame(&game, "save.txt");
	initUndoStack(&stack);
	initWindow();
	drawBoard();
	previouschess(game);
	if (game.gameOver) {
		printf("Your game was over,please start a new game.press E\n");
		Sleep(3000);
	}
	while (!game.gameOver) {
		if (_kbhit()) {
			char key = _getch();
			switch (key) {
			case 'E':
				saveGame(&game, "save.txt");
				break;
			case 'U':
				popUndo(&stack, &game);
			}
		}
		if (game.turn == 1) {
			if (MouseHit()) {
				MOUSEMSG msg = GetMouseMsg();
				int row, col;
				if (msg.uMsg == WM_LBUTTONDOWN) {
					if (game.playerchess == 1) {
						if (getBoardPosition(msg.x, msg.y, &row, &col, &game)) {
							game.board[row][col] = game.playerchess;
							if (!changLian(row, col, game) && !san(row, col, game) && !si(row, col, game)) {
								pushUndo(&stack, &game);
								drawChess(row, col, game.playerchess);
								game.winner = checkWinner(game.board, row, col);
								if (game.winner == 0)
									game.turn = 0;
								else if (game.winner == game.playerchess) {
									printf("You win!");
									game.gameOver = 1;
									break;
								}
							}
							else {
								game.board[row][col] = 0;
								printf("You lose!Because you broke rules!\n");
								game.gameOver = 1;
								break;
							}
						}
						else {
							printf("invalid position,try again\n");
						}
					}
					else {
						if (getBoardPosition(msg.x, msg.y, &row, &col, &game)) {
							game.board[row][col] = game.playerchess;
							pushUndo(&stack, &game);
							drawChess(row, col, game.playerchess);
							game.winner = checkWinner(game.board, row, col);
							if (game.winner == 0)
								game.turn = 0;
							else if (game.winner == game.playerchess) {
								printf("You win!");
								game.gameOver = 1;
								break;
							}

						}
						else {
							printf("invalid position,try again\n");
						}
					}
				}
			}
		}
		if (game.turn == 0) {
			if (isMovesLeft(game.board)) {
				int bestrow, bestcol;
				findBestMove(&game, &bestrow, &bestcol);
				game.board[bestrow][bestcol] = game.aichess;
				drawChess(bestrow, bestcol, game.aichess);
				game.winner = checkWinner(game.board, bestrow, bestcol);
				if (game.winner == 0)
					game.turn = 1;
				else if (game.winner == game.aichess) {
					printf("You lose!");
					game.gameOver = 1;
					break;
				}

			}
			else {
				printf("there is no room to place chess,you have to start a new game.\n");
			}
		}
	}
	char a=getchar();
	closegraph();
}
int main() {
	char input;
	input = '\0';
	menu();
	std::cin>>input;
	if (input == 'N') {
		gameStart();
	}
	else if (input == 'C') {
		printf("You are going to load the previous game\n");
		previousgame();
	}
	return 0;
}