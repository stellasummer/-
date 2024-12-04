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

/***��һ��������̺ͻ���ջ�ĺ���***/

typedef struct {//struct�ļ����ʽ����������ֱ����
	int board[BOARD_SIZE][BOARD_SIZE];//����״̬
	int turn;//��ǰ�غϣ�1�ˣ�0����
	int gameOver;//��Ϸ�Ƿ����
	int winner;//Ӯ�ң�1�ڣ�-1�ף�0��ʤ��
	int playerchess;
	int aichess;
}GameState;//��Ϸ״̬

typedef struct {
	GameState history[MAX_HISTORY];
	int top;
}UndoStack;//����ջ

void menu() {//���˵�
	printf("New Game:press N\n");
	printf("Continue Game:press C\n");
}
void sidemenu() {//���˵�
	printf("Exit and save:press E\n");
	printf("Undo:press U\n");
}
void chooseChess(GameState *game) {//ѡ����
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
	game->turn = 0;//->��ָ��ṹ���Ա
	game->gameOver = 0;
	game->winner = 0;
	game->playerchess = 0;
	game->aichess = 0;
}//��ʼ����Ϸ״̬

void initUndoStack(UndoStack* stack) {
	stack->top = -1;
}//��ʼ������ջ

void pushUndo(UndoStack* stack, GameState* game) {
	if (stack->top >= MAX_HISTORY - 1) {
		// �����ջ�������Ƴ������һ�μ�¼
		for (int i = 0; i < MAX_HISTORY - 1; i++) {
			stack->history[i] = stack->history[i + 1];
		}
		stack->history[MAX_HISTORY - 1] = *game;
	}
	else {//�����������3�Σ������ں����������������
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
}//�����Ⲣȡ��

/***�ڶ�������ͼ�ν���ĺ���***/

void initWindow() {
	initgraph(GRID_SIZE * BOARD_SIZE, GRID_SIZE * BOARD_SIZE);
	setbkcolor(0x96DED1);//��ѧ������֮Ī������
	cleardevice();
}
//��ʼ������

void drawBoard() {
	setlinecolor(0x008080);
	for (int i = 0; i < BOARD_SIZE; i++) {
		//����ˮƽ��
		line(GRID_SIZE / 2, GRID_SIZE / 2 + i * GRID_SIZE,
			GRID_SIZE / 2 + (BOARD_SIZE - 1) * GRID_SIZE, GRID_SIZE / 2 + i * GRID_SIZE);
		// ���ƴ�ֱ��
		line(GRID_SIZE / 2 + i * GRID_SIZE, GRID_SIZE / 2,
			GRID_SIZE / 2 + i * GRID_SIZE, GRID_SIZE / 2 + (BOARD_SIZE - 1) * GRID_SIZE);
	}
	
}
//��������

bool getBoardPosition(int mouseX, int mouseY, int* row, int* col,GameState* game) {
	*col = (mouseX - GRID_SIZE / 2 + GRID_SIZE / 2) / GRID_SIZE;
	*row = (mouseY - GRID_SIZE / 2 + GRID_SIZE / 2) / GRID_SIZE;
	if (*col >= 0 && *col < BOARD_SIZE && *row >= 0 && *row < BOARD_SIZE&&!game->board[*row][*col]) {
		return true;
	}
	return false;
}
//�ж�����λ���Ƿ����

void drawChess(int row, int col, int player) {
	int centerX = GRID_SIZE / 2 + col * GRID_SIZE;
	int centerY = GRID_SIZE / 2 + row * GRID_SIZE;
	if (player == 1)
		setfillcolor(BLACK);
	else if(player==-1)
		setfillcolor(WHITE);
	solidcircle(centerX, centerY, GRID_SIZE / 3);

}
//�����ӵĲ���

void previouschess(GameState &game) {
	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			if (game.board[i][j]) {
				drawChess(i, j, game.board[i][j]);
			}
		}
	}
}

/***������������Ϸ����***/

void saveGame(GameState* game, const char* filename) {
	FILE* file;
	errno_t err;
	err= fopen_s(&file,filename, "w");//��д�뷽ʽ���ļ�filename
	if (err != 0) {
		fclose(file);
		printf("�Ҳ����ļ�%s\n", filename);
		return;
	}
	else {
		fwrite(game, sizeof(GameState), 1, file);
		fclose(file);
		printf("��Ϸ������%s�ɹ�\n", filename);
	}
}//������Ϸ

void loadGame(GameState* game, const char* filename) {
	FILE* file;
	errno_t err;
	err = fopen_s(&file, filename, "r");//�Զ�ȡ��ʽ���ļ�filename
	if (err != 0) {
		fclose(file);
		printf("����ʧ��%s\n", filename);
		return;
	}
	else {
		fread(game, sizeof(GameState), 1, file);
		fclose(file);
		printf("��Ϸ�Ѵ� %s �ָ�\n", filename);
	}
}//��Ϸ����

/***���Ĳ�ʤ���߼����***/

int checkWinner(int board[BOARD_SIZE][BOARD_SIZE], int x, int y) {//һ���˸����򣬷ֳ�����ֱ����count
	int directions[4][2] = { {0, 1}, {1, 0}, {1, 1}, {1, -1} };
	int current = board[x][y];//��Ե�ǰ�����߽����ж�
	for (int d = 0; d < 4; d++) {
		int count = 1;//�ĸ����򣬼�鵽�˾�ֹͣ
		for (int step = 1; step < 5; step++) {  // ���������
			int nx = x + step * directions[d][0];
			int ny = y + step * directions[d][1];
			if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE && board[nx][ny] == current) {
				count++;
			}
			else {
				break;
			}
		}
		for (int step = 1; step < 5; step++) {  // ��鷴����
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
			return current;  // ����ʤ����
		}
	}
	return 0;  // û��ʤ��
}
/***���岽���ּ��***/
int countforchang(int board[BOARD_SIZE][BOARD_SIZE], int x, int y, int dx, int dy, int player) {
	int count = 1;//ĳ��player�ڸõ��������ĳ���ض��������γ��˶��ٸ�����
	for (int i = 1; i < 6; i++) {  // ֻ���� 6 ���ڵ�
		int nx = x + i * dx;
		int ny = y + i * dy;
		if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE && board[nx][ny] == player) {
			count++;
		}
		else {
			break;
		}
	}
	for (int i = 1; i < 6; i++) {  // ֻ���� 6 ���ڵ�
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
}//��鳤������
bool changLian(int x, int y, GameState& game) {
	if (countforchang(game.board, x, y, 1, 0, 1) >= 6 ||
		countforchang(game.board, x, y, 0, 1, 1) >= 6 ||
		countforchang(game.board, x, y, 1, 1, 1) >= 6 ||
		countforchang(game.board, x, y, 1, -1, 1) >= 6) {
		return 1;
	}
	return 0;
}//��鳤��
std::string chesslines(GameState& game, int x, int y, int dx, int dy) {
	std::string lines;
	lines.push_back('1');//�м���
	for (int i = 1; i < 15; i++) {
		int nx = x + i * dx;
		int ny = y + i * dy;
		if (nx < 15 && nx >= 0 && ny < 15 && ny >= 0) {
			if (game.board[nx][ny] >= 0) {
				lines.push_back(game.board[nx][ny] + '0');//�����
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
				lines.insert(lines.begin(), game.board[nx][ny] + '0');//ǰ���
			}
			else {
				lines.insert(lines.begin(), '1');
				lines.insert(lines.begin(), '-');
			}
		}
		else
			break;
	}
	lines.insert(lines.begin(), '|');//���̱�Ե
	lines.push_back('|');
	return lines;
}//��¼���з����ϵ������γɵ���
std::string huosan[5] = { "010110","011010" ,"0011100" ,"001110-1" ,"-1011100" };
std::string huosi[12] = { "0111010","011101-1","-1111010","-111101-1","0101110","010111-1","-110111-1",
"-1101110","0110110","-111011-1","-1110110","011011-1" };
std::string huosino[5] = { "10111101","101111-1","101111|","-1111101","|111101" };
//���еĻ����ͻ��ļ���
bool found(std::string chesslines, int start, std::string dict) {
	if (start >= chesslines.length() - 4)
		return false;
	if (start + dict.length() <= chesslines.length()) {
		if (chesslines.substr(start, dict.length()) == dict) {
			return true;
		}
	}
	return found(chesslines, start + 1, dict);
}//��������������û�ж�Ӧ��
bool san(int x, int y, GameState& game) {
	int count = 0;
	std::string lines[4];
	lines[0] = chesslines(game, x, y, 1, 0);
	lines[1] = chesslines(game, x, y, 0, 1);
	lines[2] = chesslines(game, x, y, 1, 1);
	lines[3] = chesslines(game, x, y, 1, -1);//����������
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
}//Ѱ����û���������ϻ���
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
}//Ѱ����û���������ϻ���

/***����������ai����***/

int evaluate(GameState *game);//��ֺ���
int minimax(GameState *game, int depth, int isMaximizingPlayer);
int isMovesLeft(int board[BOARD_SIZE][BOARD_SIZE]);
int checkDirection(GameState *game, int x, int y, int dx, int dy, int player);
// ��������Ƿ��п�λ
int isMovesLeft(int board[BOARD_SIZE][BOARD_SIZE]) {
	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			if (board[i][j] == 0) //��λ
				return 1;
		}
	}
	return 0;
}
#define WIN_SCORE 10000
#define FOUR_SCORE 1000
#define THREE_SCORE 100
#define TWO_SCORE 10

// �������������������Ļ����ģ�
int countConsecutive(int board[BOARD_SIZE][BOARD_SIZE], int x, int y, int dx, int dy, int player) {
	int count = 0;//ĳ��player�ڸõ��������ĳ���ض��������γ��˶��ٸ�����
	for (int i = 0; i < 5; i++) {  // ֻ���� 5 ���ڵ�
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

// ���ֺ���
int evaluate(GameState *game) {
	int score = 0;

	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			if (game->board[i][j] != 0) {  // ֻ���������ӵ�λ��
				int player = game->board[i][j];//���������ӽ��зֱ�����

				// �ĸ������飺ˮƽ����ֱ�������Խ���
				score += checkDirection(game, i, j, 1, 0, player);  // ˮƽ
				score += checkDirection(game, i, j, 0, 1, player);  // ��ֱ
				score += checkDirection(game, i, j, 1, 1, player);  // �Խ��� 
				score += checkDirection(game, i, j, 1, -1, player); // �Խ���
			}
		}
	}

	return score;
}

// �����鷽��ͷ��ط�ֵ
int checkDirection(GameState *game, int x, int y, int dx, int dy, int player) {
	int count = countConsecutive(game->board, x, y, dx, dy, player);
	//������ĳ���������γɼ���������,�ֱ���ai�ӷ���player����
	if (player == game->aichess) {
		if (count == 5) return WIN_SCORE;   // Ӯ��
		if (count == 4) return FOUR_SCORE;  // ����
		if (count == 3) return THREE_SCORE; // ����
		if (count == 2) return TWO_SCORE;   // ����
	}
	else{
		if (count == 5) return -WIN_SCORE;   // Ӯ��
		if (count == 4) return -FOUR_SCORE;  // ����
		if (count == 3) return -THREE_SCORE; // ����
		if (count == 2) return -TWO_SCORE;   // ����
	}
	return 0;  // ���ر�÷�
}


// Minimax ʵ��
int minimax(GameState *game, int depth, int isMaximizingPlayer) {
	int score = evaluate(game);//�����ڵľ��ƴ����

	// ����ﵽ�����ȣ����߽�����û�п��Է����ӵĵط��ˣ��������ڵķ�ֵ
	if (depth == MAX_DEPTH || !isMovesLeft(game->board)) {
		return score;
	}
	//�ݹ�2�θ����ƴ��
	if (isMaximizingPlayer) {//ai�Ļغ�
		int maxEval = INT_MIN;
		for (int i = 0; i < BOARD_SIZE; i++) {
			for (int j = 0; j < BOARD_SIZE; j++) {
				if (game->board[i][j] == 0) {//ai�˲���ϣ���Ժ���Ʒ���Խ��Խ��
					game->board[i][j] = game->aichess;
					int eval = minimax(game, depth + 1, 0);
					game->board[i][j] = 0;
					maxEval = (eval > maxEval) ? eval : maxEval;
					
				}
			}
		}
		return maxEval;//��ai���ӵ����������ѡȡ��߷ֵľ���
	}
	else {//��ҵĻغ�
		int minEval = INT_MAX;
		for (int i = 0; i < BOARD_SIZE; i++) {
			for (int j = 0; j < BOARD_SIZE; j++) {
				if (game->board[i][j] == 0) {//���ϣ���˲����Ժ���Ʒ���Խ��Խ�ã���ai������
					game->board[i][j] =game-> playerchess;
					int eval = minimax(game, depth + 1, 1);
					game->board[i][j] = 0;
					minEval = (eval < minEval) ? eval : minEval;
				
				}
			}
		}
		return minEval;//��������ҿ��ܵ�������ѡȡ��ͷֵ����
	}//���������ʹ�Լ���������������У�aiϣ���ҵ������ܶ��Լ����������ӷ���
}

// AI �����������λ��
void findBestMove(GameState *game, int* bestRow, int* bestCol) {
	int bestValue = INT_MIN;
	*bestRow = -1;
	*bestCol = -1;

	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {//����ai�������ӵ����еط����õ�ÿ��λ�õ�δ������ֵ
			if (game->board[i][j] == 0) {
				game->board[i][j] = game->aichess;  // AI ����
				if (game->aichess == -1) {
					int moveValue = minimax(game, 1, 0);
					game->board[i][j] = 0;

					if (moveValue > bestValue) {//��ֵ�����ڵĺþͻ�
						*bestRow = i;
						*bestCol = j;
						bestValue = moveValue;
					}
				}
				else if (game->aichess == 1&&!changLian(i,j,*game)&&!san(i,j,*game)&&!si(i,j,*game)){
					int moveValue = minimax(game, 1, 0);
					game->board[i][j] = 0;

					if (moveValue > bestValue) {//��ֵ�����ڵĺþͻ�
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

/***���߲���������Ϸ����***/
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