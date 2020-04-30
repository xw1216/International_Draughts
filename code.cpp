/*必须的库组件*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>  /*C11中对布尔量的支持库*/

/*符号变量定义*/

/*边界极值类*/
#define BOARD_SIZE		8	
#define MAX_STEP		8
#define MAX_CHESS_NUM		15
#define INFINITY_MAX		10000
#define INFINITY_MIN		-10000
/*棋盘状态类*/
#define MY_CHESS		1
#define ENEMY_CHESS		2
#define MY_KING			5
#define ENEMY_KING		10
#define EMPTY			0

#define ME			1
#define ENEMY			2
#define PB PrintBoard()

/*命令结构体蓝图*/
struct Command
{
	int x[MAX_STEP];
	int y[MAX_STEP];
	int StepNum;
	int Alpha;
	int Beta;
};

/*全局变量定义*/
int Board[BOARD_SIZE][BOARD_SIZE] = { 0 };		/*演算主棋盘*/
int TempBoard[BOARD_SIZE][BOARD_SIZE] = { 0 };
int MoveDirect[4][2] = { {1,-1}, {1,1}, {-1,1}, {-1,-1} };
int JumpDirect[4][2] = { {2,-2}, {2,2}, {-2,2}, {-2,-2} };
int MyHolder = 0;	/*自己持棋的颜色 1为黑 2为白 永远自上而下进攻*/
int TotalTurn = 0;	/*局数计数器 己方总是在奇数局行动*/
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

/*函数声明*/

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

/*主程序*/
int main()
{
	loop();
	return 0;
}

/*函数体定义*/

/*总入口函数*/
void loop()
{
	char SubOrder[6] = { 0 };
	int Status = 0;
	struct Command Order = { {0}, {0}, 0 };
	/*检测命令前缀 分情况执行*/
	while (true)
	{
		memset(SubOrder, 0, sizeof(SubOrder));
		memset(&Order, 0, sizeof(struct Command));

		scanf_s("%s", SubOrder,sizeof(SubOrder));

		if (!strcmp(SubOrder, "START"))
		{
			//此处读数 1代表己方执黑棋 2代表己方执白棋
			scanf_s("%d", &MyHolder);
			if (MyHolder == 2)
				TotalTurn = -1;	/*使己方总是在奇数局行动*/
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

/*基础指令函数*/
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
	memcpy(TempBoard, Board, sizeof(Board));	/*暂存初始局面*/
	AI();
	memcpy(Board, TempBoard, sizeof(Board));	/*复盘*/
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
		/*应用操作 更新棋盘*/
		if (abs(Cmd->x[i] - Cmd->x[i - 1]) == 2)		/*跳吃操作*/
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
	/*王棋晋升判断*/
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

/*功能判断函数*/

int max(int a, int b)
{
	return a >= b ? a : b;
}

/*越界判断*/
bool InRange(int x,int y)
{
	if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE)
		return true;
	else
		return false;
}
/*跳吃条件是否成立判断*/
bool CanJump(int x, int y, int Midx, int Midy)
{
	if (Board[Midx][Midy] == EMPTY)		/*外部已经确保末位置存在且空置*/
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
/*根据持方转换命令坐标*/
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

/*估值函数*/
/*还需要加入对整体的评估与分析*/
/*要能够使王棋走法更灵活的权重值*/
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

/*可行方案侦测函数*/
/*检测能否移动并生成方案*/
int DetectMove(int x, int y, int CountMove, struct Command * const Cmd)
{
	int Newx = 0, Newy = 0, StartDirect = 0,EndDirect = 0;
	/*外部已经确保位置上确保有棋子*/
	/*确定棋子前进方向*/
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
		if ( InRange(Newx, Newy) && Board[Newx][Newy] == EMPTY)	/*如果可行就记录每一种着法*/
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

/*检测是否能跳吃并生成方案 递归调用*/
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
		/*如果存在可行方案*/
		if (InRange(Newx, Newy) && Board[Newx][Newy] == EMPTY && CanJump(x,y,Midx,Midy))
		{
			CanDo++;
			if (CanDo != 1)		/*当前节点有多分叉 产生新方案 则将目前节点前的已有步骤复制到新方案中*/
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
			/*应用跳吃结果*/
			Board[Newx][Newy] = Board[x][y];
			TempChess = Board[Midx][Midy];
			Board[Midx][Midy] = Board[x][y] = EMPTY;
			/*递归调用*/
			CountJump =  DetectJump(Newx, Newy, CurStep + 1, CountJump, Cmd);
			/*还原当前一步所做的更改*/
			Board[x][y] = Board[Newx][Newy];
			Board[Midx][Midy] = TempChess;
			Board[Newx][Newy] = EMPTY;
		}
	}
	return CountJump;
}

/*minimax向外层父节点传递本层每一方案的alpha-beta估值*/
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

/*AI函数*/
/*总入口AI函数*/
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

/*走法方案树生成AI*/
/*递归调用 生成搜索树 从最深层应用分值判定minimax-alpha-beta剪枝算法*/
/*Actor用以判断棋子执方 指针用以向外层父节点传递分支评价分数*/
void SchemeAI(int Deep, int Actor, struct Command* const OuterCmd) 
{
	
	/*若到达限定的最深层则对叶节点估值*/
	if (Deep == 0 )
	{
		int score = GetValue();
		/*叶节点实际上做选择的一方与Actor相反*/
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
		
		/*储存当前步的所有跳吃方案*/
		struct Command SimJump[MAX_CHESS_NUM];	
		for (int i = 0; i < MAX_CHESS_NUM; i++)	/*内部跳吃方案初始化*/
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
					if (SimJump[CountJump].StepNum != 0)	/*防止计数器错位重复写入*/
						CountJump++;
				}
			}
		}

		if (CountJump != 0)			/*能跳则跳*/
		{
			int MaxJumpNum = 0;
			memcpy(InnerTempBoard, Board, sizeof(Board));
			for (int i = 0; i < CountJump; i++)		/*仅选择步数最多的跳吃检测*/
			{
				MaxJumpNum = max(MaxJumpNum, SimJump[i].StepNum);
			}
			
			for (int i = 0; i < CountJump; i++)
			{
				if (OuterCmd->Beta < OuterCmd->Alpha)		/*剪枝条件满足 不再对后续预测方案检测*/
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
			/*储存当前步的所有移动方案 该处结构体数组容量太小易造成堆损毁 太大易造成堆溢出*/
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
					if (OuterCmd->Beta < OuterCmd->Alpha)	/*剪枝条件满足 不再对后续预测方案检测*/
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
			/*无路可走时*/
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
