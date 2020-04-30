/*����Ŀ����*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>  /*C11�жԲ�������֧�ֿ�*/

/*���ű�������*/

/*�߽缫ֵ��*/
#define BOARD_SIZE				8	
#define MAX_STEP					8
#define MAX_CHESS_NUM		15
#define INFINITY_MAX			10000
#define INFINITY_MIN		   -10000
/*����״̬��*/
#define MY_CHESS					1
#define ENEMY_CHESS			2
#define MY_KING					5
#define ENEMY_KING				10
#define EMPTY						0

#define ME								1
#define ENEMY						2
#define PB PrintBoard()

/*����ṹ����ͼ*/
struct Command
{
	int x[MAX_STEP];
	int y[MAX_STEP];
	int StepNum;
	int Alpha;
	int Beta;
};

/*ȫ�ֱ�������*/
int Board[BOARD_SIZE][BOARD_SIZE] = { 0 };		/*����������*/
int TempBoard[BOARD_SIZE][BOARD_SIZE] = { 0 };
int MoveDirect[4][2] = { {1,-1}, {1,1}, {-1,1}, {-1,-1} };
int JumpDirect[4][2] = { {2,-2}, {2,2}, {-2,2}, {-2,-2} };
int MyHolder = 0;	/*�Լ��������ɫ 1Ϊ�� 2Ϊ�� ��Զ���϶��½���*/
int TotalTurn = 0;	/*���������� �����������������ж�*/
int MaxSearchDepth = 1;
struct Command FinalDecision;

int LocationValue[BOARD_SIZE][BOARD_SIZE] = {
	{0,2,0,2,0,2,0,2},
	{3,0,3,0,3,0,3,0},
	{0,4,0,4,0,4,0,4},
	{5,0,6,0,6,0,6,0},
	{0,7,0,8,0,7,0,6},
	{7,0,8,0,8,0,8,0},
	{0,9,0,9,0,9,0,8},
	{9,0,9,0,9,0,9,0},
};

/*��������*/

void loop();
void start();
void turn();
void place(struct Command* Cmd);
void end(int Status);
void PrintBoard();
void DeBug(char* str);
void RotateOrder(struct Command* const  Cmd);

void AI();
void SchemeAI(int Deep,int Actor, struct Command* const OuterCmd);

bool InRange(int x,int y);
bool CanJump(int x, int y, int Midx, int Midy);
bool ValueUpTrans(struct Command* const SimAction, int Actor, struct Command* const OuterCmd);
int DetectMove(int x, int y, int CountMove, struct Command* const Cmd);
int DetectJump(int x, int y, int CurStep,int CountJump, struct Command* const  Cmd);
int max(int a, int b);
int GetValue();

/*������*/
int main()
{
	loop();
	return 0;
}

/*�����嶨��*/

/*����ں���*/
void loop()
{
	char SubOrder[6] = { 0 };
	int Status = 0;
	struct Command Order = { {0}, {0}, 0 };
	/*�������ǰ׺ �����ִ��*/
	while (true)
	{
		memset(SubOrder, 0, sizeof(SubOrder));
		memset(&Order, 0, sizeof(struct Command));

		scanf_s("%s", SubOrder,sizeof(SubOrder));

		if (!strcmp(SubOrder, "START"))
		{
			//�˴����� 1������ִ���� 2������ִ����
			scanf_s("%d", &MyHolder);
			if (MyHolder == 2)
				TotalTurn = -1;	/*ʹ�����������������ж�*/
			start();
		}
		else if (!strcmp(SubOrder, "TURN"))
		{
			TotalTurn++;
			turn();
			fflush(stdout);
		}
		else if (!strcmp(SubOrder, "PLACE"))
		{
			TotalTurn++;											
			scanf_s("%d", &Order.StepNum);
			for (int i = 0; i < Order.StepNum; i++)
			{
				scanf_s("%d,%d", &Order.x[i], &Order.y[i]);
			}
			RotateOrder(&Order);
			place(&Order);
			fflush(stdout);
		}
		else if (!strcmp(SubOrder, "END"))
		{
			scanf_s("%d", &Status);
			end(Status);
		}
	}
}

/*����ָ���*/
void start()
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < BOARD_SIZE; j += 2)
		{
			Board[i][j + (i + 1) % 2] = MY_CHESS;
		}
	}
	for (int i = 5; i < BOARD_SIZE; i++)
	{
		for (int j = 0; j < BOARD_SIZE; j += 2)
		{
			Board[i][j + (i + 1) % 2] = ENEMY_CHESS;
		}
	}
	printf("OK\n");
	fflush(stdout);
}

void turn()
{
	memset(&FinalDecision, 0, sizeof(struct Command));
	FinalDecision.Alpha = INFINITY_MIN;
	FinalDecision.Beta = INFINITY_MAX;
	memcpy(TempBoard, Board, sizeof(Board));	/*�ݴ��ʼ����*/
	AI();
	memcpy(Board, TempBoard, sizeof(Board));	/*����*/
   	place(&FinalDecision);

	RotateOrder(&FinalDecision);
	printf("%d", FinalDecision.StepNum);
	for (int i = 0; i < FinalDecision.StepNum; i++)
	{
		printf(" %d,%d", FinalDecision.x[i], FinalDecision.y[i]);
	}
	printf("\n");
}

void place(struct Command* Cmd)
{
	for (int i = 1; i < Cmd->StepNum; i++)
	{
		/*Ӧ�ò��� ��������*/
		if (abs(Cmd->x[i] - Cmd->x[i - 1]) == 2)		/*���Բ���*/
		{
			int Midx = (Cmd->x[i] + Cmd->x[i - 1]) / 2, Midy = (Cmd->y[i] + Cmd->y[i - 1]) / 2;
			Board[Cmd->x[i]][Cmd->y[i]] = Board[Cmd->x[i - 1]][Cmd->y[i - 1]];
			Board[Cmd->x[i - 1]][Cmd->y[i - 1]] = EMPTY;
			Board[Midx][Midy] = EMPTY;
		}
		if (abs(Cmd->x[i] - Cmd->x[i - 1]) == 1)
		{
			Board[Cmd->x[i]][Cmd->y[i]] = Board[Cmd->x[i - 1]][Cmd->y[i - 1]];
			Board[Cmd->x[i - 1]][Cmd->y[i - 1]] = EMPTY;
		}
	}
	/*��������ж�*/
	for (int i = 0; i < BOARD_SIZE; i++)
	{
		if (Board[0][i] == ENEMY_CHESS)
			Board[0][i] = ENEMY_KING;
		if (Board[7][i] == MY_CHESS)
			Board[7][i] = MY_KING;
	}
}

void end(int Status)
{
	exit(0);
}

/*�����жϺ���*/

int max(int a, int b)
{
	return a >= b ? a : b;
}

/*Խ���ж�*/
bool InRange(int x,int y)
{
	if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE)
		return true;
	else
		return false;
}
/*���������Ƿ�����ж�*/
bool CanJump(int x, int y, int Midx, int Midy)
{
	if (Board[Midx][Midy] == EMPTY)		/*�ⲿ�Ѿ�ȷ��ĩλ�ô����ҿ���*/
		return false;
	else if ((Board[x][y] == MY_CHESS || Board[x][y] == MY_KING) && (Board[Midx][Midy] == ENEMY_CHESS || Board[Midx][Midy] == ENEMY_KING))
	{
		return true;
	}
	else if ((Board[x][y] == ENEMY_CHESS || Board[x][y] == ENEMY_KING) && (Board[Midx][Midy] == MY_CHESS || Board[Midx][Midy] == MY_KING))
	{
		return true;
	}
	else
		return false;
}
/*���ݳַ�ת����������*/
void RotateOrder(struct Command* Cmd)
{
	if (MyHolder == 1)
	{
		for (int i = 0; i < Cmd->StepNum; i++)
		{
			Cmd->x[i] = BOARD_SIZE - Cmd->x[i] - 1;
			Cmd->y[i] = BOARD_SIZE - Cmd->y[i] - 1;
		}
	}
}

/*��ֵ����*/
/*����Ҫ�������������������*/
/*Ҫ�ܹ�ʹ�����߷�������Ȩ��ֵ*/
int GetValue()
{
	int Value = 0;

	for (int i = 0; i < BOARD_SIZE; i++)
	{
		for (int j = 0; j < BOARD_SIZE; j++)
		{
			switch (Board[i][j])
			{
				case 1:Value += 50 + LocationValue[i][j]; 
					break;
				case 2:Value -= 50; 
					break;
				case 5:Value += 100 + LocationValue[i][j];
					break;
				case 10:Value -= 100; 
					break;
			}
		}
	}
	return Value;
}

/*���з�����⺯��*/
/*����ܷ��ƶ������ɷ���*/
int DetectMove(int x, int y, int CountMove, struct Command * const Cmd)
{
	int Newx = 0, Newy = 0, StartDirect = 0,EndDirect = 0;
	/*�ⲿ�Ѿ�ȷ��λ����ȷ��������*/
	/*ȷ������ǰ������*/
	if (Board[x][y] == MY_CHESS)	
		EndDirect = 2;
	else if (Board[x][y] == ENEMY_CHESS)
	{
		StartDirect = 2;
		EndDirect = 4;
	}
	else
		EndDirect = 4;

	for (int i = StartDirect; i < EndDirect; i++)
	{
		Newx = x + MoveDirect[i][0];
		Newy = y + MoveDirect[i][1];
		if ( InRange(Newx, Newy) && Board[Newx][Newy] == EMPTY)	/*������оͼ�¼ÿһ���ŷ�*/
			{
				Cmd[CountMove].x[0] = x;
				Cmd[CountMove].y[0] = y;
				Cmd[CountMove].x[1] = Newx;
				Cmd[CountMove].y[1] = Newy;
				Cmd[CountMove].StepNum = 2;
				CountMove++;
			}
	}
	return CountMove;
}

/*����Ƿ������Բ����ɷ��� �ݹ����*/
int DetectJump(int x, int y, int CurStep,int CountJump, struct Command* const Cmd)
{
	int Newx = 0, Newy = 0, Midx = 0, Midy = 0;
	int TempChess = 0, CanDo = 0;
	
	for (int i = 0; i < 4; i++)
	{
		Newx = x + JumpDirect[i][0];
		Newy = y + JumpDirect[i][1];
		Midx = (Newx + x) / 2;
		Midy = (Newy + y) / 2;
		/*������ڿ��з���*/
		if (InRange(Newx, Newy) && Board[Newx][Newy] == EMPTY && CanJump(x,y,Midx,Midy))
		{
			CanDo++;
			if (CanDo != 1)		/*��ǰ�ڵ��ж�ֲ� �����·��� ��Ŀǰ�ڵ�ǰ�����в��踴�Ƶ��·�����*/
			{
				memcpy(Cmd[CountJump + 1].x, &Cmd[CountJump].x, CurStep * sizeof(int));
				memcpy(Cmd[CountJump + 1].y, &Cmd[CountJump].y, CurStep * sizeof(int));
				Cmd[CountJump + 1].StepNum = CurStep;
				CountJump++;
			}

			if (CurStep == 1)
			{
				Cmd[CountJump].x[0] = x;
				Cmd[CountJump].y[0] = y;
				Cmd[CountJump].x[1] = Newx;
				Cmd[CountJump].y[1] = Newy;
				Cmd[CountJump].StepNum = 2;
			}
			else
			{
				Cmd[CountJump].x[CurStep] = Newx;
				Cmd[CountJump].y[CurStep] = Newy;
				Cmd[CountJump].StepNum++;
			}
			/*Ӧ�����Խ��*/
			Board[Newx][Newy] = Board[x][y];
			TempChess = Board[Midx][Midy];
			Board[Midx][Midy] = Board[x][y] = EMPTY;
			/*�ݹ����*/
			CountJump =  DetectJump(Newx, Newy, CurStep + 1, CountJump, Cmd);
			/*��ԭ��ǰһ�������ĸ���*/
			Board[x][y] = Board[Newx][Newy];
			Board[Midx][Midy] = TempChess;
			Board[Newx][Newy] = EMPTY;
		}
	}
	return CountJump;
}

/*minimax����㸸�ڵ㴫�ݱ���ÿһ������alpha-beta��ֵ*/
bool ValueUpTrans(struct Command* const SimAction, int Actor, struct Command* const OuterCmd)
{
	if (Actor == ME)
	{
		if (SimAction->Beta > OuterCmd->Alpha)
		{
			OuterCmd->Alpha = SimAction->Beta;
			return true;
		}
		else if(SimAction->Beta <= OuterCmd->Alpha)
		{
			return false;
		}
	}
	else
	{
		if (SimAction->Alpha < OuterCmd->Beta)
		{
			OuterCmd->Beta = SimAction->Alpha;
			return true;
		}
		else if (SimAction->Alpha >= OuterCmd->Beta)
		{
			return false;
		}
	}
	return false;
}

/*AI����*/
/*�����AI����*/
void AI()
{
	if (TotalTurn < 10)
	{
		MaxSearchDepth = 3;
	}
	else if (TotalTurn < 45)
	{
		MaxSearchDepth = 7;
	}
	else
	{
		MaxSearchDepth = 5;
	}
	SchemeAI(MaxSearchDepth, ME, &FinalDecision);
}

/*�߷�����������AI*/
/*�ݹ���� ���������� �������Ӧ�÷�ֵ�ж�minimax-alpha-beta��֦�㷨*/
/*Actor�����ж�����ִ�� ָ����������㸸�ڵ㴫�ݷ�֧���۷���*/
void SchemeAI(int Deep, int Actor, struct Command* const OuterCmd) 
{
	
	/*�������޶�����������Ҷ�ڵ��ֵ*/
	if (Deep == 0 )
	{
		int score = GetValue();
		/*Ҷ�ڵ�ʵ������ѡ���һ����Actor�෴*/
		if (Actor == ENEMY && OuterCmd->Beta >= score)
		{
			OuterCmd->Beta = score;
		}
		else if (Actor == ME && OuterCmd->Alpha <= score)
		{
			OuterCmd->Alpha = score;
		}
	}
	else
	{
		int CountJump = 0, CountMove = 0, BestSchemeNum = 0;
		int InnerTempBoard[BOARD_SIZE][BOARD_SIZE] = { 0 };
		
		/*���浱ǰ�����������Է���*/
		struct Command SimJump[MAX_CHESS_NUM];	
		for (int i = 0; i < MAX_CHESS_NUM; i++)	/*�ڲ����Է�����ʼ��*/
		{
			SimJump[i].StepNum = 0;
			SimJump[i].Alpha = OuterCmd->Alpha;
			SimJump[i].Beta = OuterCmd->Beta;
			memset(SimJump[i].x, 0, MAX_STEP * sizeof(int));
			memset(SimJump[i].y, 0, MAX_STEP * sizeof(int));
		}

		for (int i = 0; i < BOARD_SIZE; i++)
		{
			for (int j = 0; j < BOARD_SIZE; j++)
			{
				if (Board[i][j] != EMPTY && Board[i][j] & Actor)
				{
					CountJump = DetectJump(i, j, 1, CountJump, SimJump);
					if (SimJump[CountJump].StepNum != 0)	/*��ֹ��������λ�ظ�д��*/
						CountJump++;
				}
			}
		}

		if (CountJump != 0)			/*��������*/
		{
			int MaxJumpNum = 0;
			memcpy(InnerTempBoard, Board, sizeof(Board));
			for (int i = 0; i < CountJump; i++)		/*��ѡ�����������Լ��*/
			{
				MaxJumpNum = max(MaxJumpNum, SimJump[i].StepNum);
			}
			
			for (int i = 0; i < CountJump; i++)
			{
				if (OuterCmd->Beta < OuterCmd->Alpha)		/*��֦�������� ���ٶԺ���Ԥ�ⷽ�����*/
				{
					break;
				}
				
				if (SimJump[i].StepNum == MaxJumpNum)
				{
					place(&SimJump[i]);
					if (Actor == ENEMY)
						SchemeAI(Deep - 1, ME, &SimJump[i]);
					else
						SchemeAI(Deep - 1, ENEMY, &SimJump[i]);

					if (ValueUpTrans(&SimJump[i], Actor, OuterCmd))
					{
						BestSchemeNum = i;
					}

					for (int j = i + 1; j < CountJump; j++)
					{
						if (Actor == ME && SimJump[j].StepNum == MaxJumpNum)
						{
							SimJump[j].Alpha = OuterCmd->Alpha;
							break;
						}
						else if(Actor == ENEMY && SimJump[j].StepNum == MaxJumpNum)
						{
							SimJump[j].Beta = OuterCmd->Beta;
							break;
						}
					}
					
					memcpy(Board, InnerTempBoard, sizeof(Board));
					
					if (Deep == MaxSearchDepth)
					{
						FinalDecision = SimJump[BestSchemeNum];
					}
				}
			}
		}
		else
		{
			/*���浱ǰ���������ƶ����� �ô��ṹ����������̫С����ɶ���� ̫������ɶ����*/
			struct Command SimMove[3*MAX_CHESS_NUM];	
			for (int i = 0; i < 3*MAX_CHESS_NUM-1; i++)
			{
				SimMove[i].StepNum = 0;
				SimMove[i].Alpha = OuterCmd->Alpha;
				SimMove[i].Beta = OuterCmd->Beta;
				memset(SimMove[i].x, 0, MAX_STEP * sizeof(int));
				memset(SimMove[i].y, 0, MAX_STEP * sizeof(int));
			}

			for (int i = 0; i < BOARD_SIZE; i++)
			{
				for (int j = 0; j < BOARD_SIZE; j++)
				{
					if (Board[i][j] != EMPTY && Board[i][j] & Actor)
					{
						CountMove = DetectMove(i, j, CountMove, SimMove);
					}
				}
			}
			
			if (CountMove != 0)
			{
				memcpy(InnerTempBoard, Board, sizeof(Board));
				for (int i = 0; i < CountMove; i++)
				{
					if (OuterCmd->Beta < OuterCmd->Alpha)	/*��֦�������� ���ٶԺ���Ԥ�ⷽ�����*/
					{
						break;
					}
					
					place(&SimMove[i]);
					if (Actor == ENEMY)
						SchemeAI(Deep-1, ME, &SimMove[i]);
					else
						SchemeAI(Deep-1, ENEMY, &SimMove[i]);

					if (ValueUpTrans(&SimMove[i], Actor, OuterCmd))
					{
						BestSchemeNum = i;
					}
					
					if (Actor == ME)
					{
						SimMove[i+1].Alpha = OuterCmd->Alpha;
					}
					else
					{
						SimMove[i+1].Beta = OuterCmd->Beta;
					}

					memcpy(Board, InnerTempBoard, sizeof(Board));
				}
					if (Deep == MaxSearchDepth)
					{
						FinalDecision = SimMove[BestSchemeNum];
					}
			}
			/*��·����ʱ*/
			else
			{
				if (Actor == ENEMY)
				{
					OuterCmd->Beta = INFINITY_MIN - 1;
				}
				else if (Actor == ME)
				{
					OuterCmd->Alpha = INFINITY_MAX + 1;
				}
			}
		}
	}	
}

void PrintBoard()
{
	char visualBoard[BOARD_SIZE][BOARD_SIZE + 1] = { 0 };
	for (int i = 0; i < BOARD_SIZE; i++)
	{
		for (int j = 0; j < BOARD_SIZE; j++)
		{
			switch (Board[i][j])
			{
			case EMPTY:
				visualBoard[i][j] = '.';
				break;
			case ENEMY_CHESS:
				visualBoard[i][j] = 'E';
				break;
			case MY_CHESS:
				visualBoard[i][j] = 'M';
				break;
			case ENEMY_KING:
				visualBoard[i][j] = '@';
				break;
			case MY_KING:
				visualBoard[i][j] = '*';
				break;
			default:
				break;
			}
		}
		printf("%s\n", visualBoard[i]);
		fflush(stdout);
	}
}

void DeBug(char* str)
{
	printf("DEBUG %s", str);
}