
//////////////////////////////////////////////
  //            AlphaJo 1.0            //
//////////////////////////////////////////////

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
//////////////////////////////////////////////
//                 �����ʾ                 //
//////////////////////////////////////////////

#define  NOPIECE     0 //����
#define  KING        1 //��
#define  ARCHER      2 //������
#define  KNIGHT      3 //��ʿ
#define  FIGHTER     4 //����
#define  WHITE       0 //�׷�
#define  BLACK       1 //�ڷ�

#define MAX_GEN_MOVES    32 //�߷������������
#define SEARCH_DEPTH      7 //�����������
#define RANDOM_VALUE     10 //�������
#define INFINITY_VALUE  1000 //���þ����ֵ�����Ϊ1000

//ȫ�ֱ���
int currentPlayer = WHITE;   //��ʼ��Ϊ�׷����壬BLACK��ʾ�ڷ�����
int theMoves[MAX_GEN_MOVES]; //����һ���߷����������������ɵ������߷�
int bestMove;                //����߷����������
int theDepth;                //��ǰ�������

//��������(����������λ�ã��°��Ϻ�)
char board[64] = {
  0,  0,  0,  0,  0,  0,  0,  0,
  0, 18, 17, 16, 17, 18,  0,  0,
  0, 19, 19, 19, 19, 19,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,
  0, 11, 11, 11, 11, 11,  0,  0,
  0, 10,  9,  8,  9, 10,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0
};

//������������Ƿ��������ϵ�����
static const int isInBoard[64] = {
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 1, 1, 1, 1, 1, 0, 0,
  0, 1, 1, 1, 1, 1, 0, 0,
  0, 1, 1, 1, 1, 1, 0, 0,
  0, 1, 1, 1, 1, 1, 0, 0,
  0, 1, 1, 1, 1, 1, 0, 0,
  0, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0
};

//����������Ƿ���δ���ӺͲ����Ƿ���˵�����
static const int isAtHome[2][64] = {
  { //����(����)
    0, 0, 0, 0, 0, 0, 0, 8, //�±�Ϊ7��Ԫ�������жϲ����Ƿ����
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 0, 0,
    0, 1, 1, 1, 1, 1, 0, 0,
    0, 1, 1, 1, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
  }, { //����(����)
    0, 0, 0, 0, 0, 0, 0,-8, //�±�Ϊ7��Ԫ�������жϲ����Ƿ����
    0, 1, 1, 1, 1, 1, 0, 0,
    0, 1, 1, 1, 1, 1, 0, 0,
    0, 1, 1, 1, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
  }
};

//�⼸������������ʾ���ӵ����ӷ���
static const char kingMovesTable[8] = {-8, 8, -1, 1, -9, -7, 9, 7}; //��
static const char archerMovesTable[4] = {-9, -7, 9, 7}; //����
static const char knightMovesTable[4] = {-8, 8, -1, 1}; //��ʿ
static const char fightMovesTable[4] = {-8, 8, -1, 1}; //����

//���ӵļ�ֵ����
static const char pieceValue[4] = {9, 5, 5, 3}; //��9 ��5 ��5 ��3

//�����������±�������X����
int getXFromLocation(int location){
  return (location & 7) - 1;
}

//�����������±�������Y����
int getYFromLocation(int location){
  return (location >> 3) - 1;
}

//������X���꣬Y���������������±�
int getLocationFromXY(int x, int y){
  return (x + 1) + (y + 1 << 3);
}

//�ж��Ƿ�Ϊ��������
int isMyPiece(char piece){
  int tag;
  tag = (currentPlayer << 3) + 8;
  return piece & tag;
}

//�ı����巽������0 ����1
void changePlayer(){
  currentPlayer = 1 - currentPlayer;
}

//////////////////////////////////////////////
//                 �߷�����                 //
//////////////////////////////////////////////

//�������Ϸ�һö���ӵĺ���
void addPiece(int location, char piece){
  board[location] = piece;
}

//������������һö���ӵĺ���
void delPiece(int location){
  board[location] = NOPIECE;
}

//�����߷������߷����λ��(���λ�õ����������±�)
int generateMoveFrom(int move){
  return move & 255;
}

//�����߷������߷�Ŀ��λ��(Ŀ��λ�õ����������±�)
int generateMoveTo(int move){
  return move >> 8;
}

//�����λ�ú�Ŀ��λ�úϳ��߷�
int composeMove(int locationFrom, int locationTo){
  return locationFrom + locationTo * 256;
}

//�߷����ɺ���������һ������������߷�������һ���߷��б�����ָ�룬�������ɵ������߷�
int generateAllMoves(int *moves){
  int i, genCount, locationFrom, locationTo, sideTag;
  char pieceFrom, pieceTo;

  //�߷�����������
  genCount = 0;
  //���巽���
  sideTag = (currentPlayer << 3) + 8;

  //���������ҵ���ǰ���巽����
  for(locationFrom = 0; locationFrom < 64; locationFrom ++){
    //ȡ�õ�ǰλ�õ�����
    pieceFrom = board[locationFrom];

    //1.����ҵ��Ĳ��Ǳ������ӣ�������
    if(!isMyPiece(pieceFrom)){
      continue;
    }

    //2.�ҵ����ӣ������������������߷�
    switch (pieceFrom - sideTag + 1){

      case KING:
        for(i = 0; i < 8; i++){
          locationTo = locationFrom + kingMovesTable[i];
          if(isAtHome[currentPlayer][locationTo]){ //Ŀ��λ���Ƿ�δ������
            pieceTo = board[locationTo];
            if(!isMyPiece(pieceTo)){ //Ŀ��λ���Ƿ��ޱ�������
              moves[genCount] = composeMove(locationFrom, locationTo); //�����߷�
              genCount++; //����
            }
          }
        }
        break;

      case ARCHER:
        for(i = 0; i < 4; i++){
          locationTo = locationFrom + archerMovesTable[i];
          if(isInBoard[locationTo]){ //Ŀ��λ���Ƿ���������
            pieceTo = board[locationTo];
            if(!isMyPiece(pieceTo)){ //Ŀ��λ���Ƿ��ޱ�������
              moves[genCount] = composeMove(locationFrom, locationTo); //�����߷�
              genCount++; //����
            }
          }
        }
        break;

      case KNIGHT:
        for(i = 0; i < 4; i++){
          locationTo = locationFrom + knightMovesTable[i];
          if(isInBoard[locationTo]){ //Ŀ��λ���Ƿ���������
            pieceTo = board[locationTo];
            if(!isMyPiece(pieceTo)){ //Ŀ��λ���Ƿ��ޱ�������
              moves[genCount] = composeMove(locationFrom, locationTo); //�����߷�
              genCount++; //����
            }
          }
        }
        break;

      case FIGHTER:
        for(i = 0; i < 4; i++){
          locationTo = locationFrom + kingMovesTable[i];
          if(isInBoard[locationTo]){ //Ŀ��λ���Ƿ���������
            if(locationFrom + isAtHome[currentPlayer][7] != locationTo){ //�����Ƿ����
              pieceTo = board[locationTo];
              if(!isMyPiece(pieceTo)){ //Ŀ��λ���Ƿ��ޱ�������
                moves[genCount] = composeMove(locationFrom, locationTo); //�����߷�
                genCount++; //����
              }
            }
          }
        }
        break;
    }
  }
  return genCount; //�������ɵ��߷���
}

//�ܸ����߷���һ����ĺ���
int makeOneMove(int move, char *captured){ //����һ�������������λ�õ�����
  int i, genCount, isLegalMove, locationFrom, locationTo;
  char pieceFrom;

  isLegalMove = 1; //��ʼ���߷�Ϊ���Ϸ�
  genCount = generateAllMoves(theMoves); //���������߷�
  //�������߷��в��ҵ�ǰ�߷��Ƿ����
  for(i = 0; i < genCount; i++){
    //����ҵ�һ���߷����ڵ�ǰ�߷�
    if(theMoves[i] == move){
      isLegalMove = 0; //�����߷����д��߷���˵���߷��Ϸ�
      break;
    }
  }

  //1.�����ж��߷��Ƿ�Ϸ�
  if(isLegalMove == 1){
    return 0; //��������ʧ��
  }

  //2.�ֽ��߷���ȡ�����λ�ú�Ŀ��λ��
  locationFrom = generateMoveFrom(move);
  locationTo = generateMoveTo(move);

  //3.ȡ��Ҫ�ߵ���
  pieceFrom = board[locationFrom];

  //4.���汻�Ե���
  *captured = board[locationTo];

  //5.�ƶ�����
  if(*captured != NOPIECE){
    delPiece(locationTo);
  }
  delPiece(locationFrom);
  addPiece(locationTo, pieceFrom);

  //6.�������巽
  changePlayer();

  return 1; //��������ɹ�
}

//������һ����ĺ���
void undoOneMove(int move, char captured){
  int locationFrom, locationTo;
  char pieceFrom;

  //1.�ֽ��߷���ȡ�����λ�ú�Ŀ��λ��
  locationFrom = generateMoveFrom(move);
  locationTo = generateMoveTo(move);

  //3.ȡ��Ҫ��ԭ����
  pieceFrom = board[locationTo];

  //4.�������巽
  changePlayer();

  //5.�ƶ�����
  delPiece(locationTo);   //ɾ������
  addPiece(locationFrom, pieceFrom); //��Ӽ�������
  if(captured != NOPIECE){
    addPiece(locationTo, captured);
  }
}

//////////////////////////////////////////////
//                 ��������                 //
//////////////////////////////////////////////

//�ж�ָ�����巽�Ƿ�ֳ�ʤ���ĺ���
int isThePlayerDie(int thePlayer){
  int i, theKing, isDie;

  //��ʼ�����
  isDie = 1;

  //ȡ�õ�ǰ���巽����
  theKing = (thePlayer << 3) + 8;

  //�����������߷������ѱ�������������1
  if(generateAllMoves(theMoves)){
    //������Ƿ���ȥ
    for(i = 0; i < 64; i ++){
      if(board[i] == theKing){
        isDie = 0;
      }
    }
  }

  return isDie;
}

//������������
int evaluatePosition(){

  int i, whiteValue, blackValue, value;

  //�Լ�����
  if(isThePlayerDie(currentPlayer)){
    return -INFINITY_VALUE + theDepth;
  }

  //��ʼ��˫�����ܼ�ֵ
  whiteValue = blackValue = 0;

  //�������̣��ҵ�����
  for(i = 0; i < 64; i++){
    if(board[i] <= 15 && board[i] >= 1){
      whiteValue += pieceValue[board[i] % 8];
    }
    if(board[i] >= 16){
      blackValue += pieceValue[board[i] % 8];
    }
  }

  //��������ֵ
  value = whiteValue - blackValue;

  return currentPlayer == WHITE ? value : -value;
}

//////////////////////////////////////////////
//                 ��������                 //
//////////////////////////////////////////////

//Alpha-Beta��������
int AlphaBetaSearch(int depth, int alpha, int beta){
  int i, genCount, value;
  int allMoves[MAX_GEN_MOVES];
  char captured;

  //���������ָ����ȣ��򷵻ؾ�������ֵ
  if(depth == 0){
    return evaluatePosition();
  }

  //�����ɱ�壬�͸��ݾ�ɱ��Ĳ�����������
  if(isThePlayerDie(currentPlayer)){ //�Լ�����
    return -INFINITY_VALUE + theDepth;
  }

  genCount = generateAllMoves(allMoves);

  for(i = 0; i < genCount; i++){
    if(makeOneMove(allMoves[i], &captured)){ //�������ɹ�
      theDepth++;
      value = -AlphaBetaSearch(depth - 1, -beta, -alpha); //�ݹ�
      undoOneMove(allMoves[i], captured); //��ԭ
      theDepth--;

      if(value >= beta){
        return beta;
      }
      if(value > alpha){
        alpha = value;
        if(theDepth == 0){ //����Ǹ��ڵ㱣������߷�
          bestMove = allMoves[i];
          alpha += (rand() & RANDOM_VALUE) - (rand() & RANDOM_VALUE); //�����
        }
      }
    }
  }

  return alpha;
}

//�õ�������
void computerThink(){
  char captured;
  theDepth = 0; //������ľ���
  AlphaBetaSearch(SEARCH_DEPTH, -INFINITY_VALUE, INFINITY_VALUE);
  makeOneMove(bestMove, &captured);
}

//////////////////////////////////////////////
//                 �������                 //
//////////////////////////////////////////////

//����������������õ���(����)�ǰ׷������Ǻڷ�
int engine = BLACK;

//д����ת���̵ĺ���
int flipLocation(int location){
  location = 62 - location;
  return location;
}

//д���򵥵Ľ����
void showBoard(){
  int i, j, piece;
  int location;

  printf("\n");
  printf("                    y **********\n");
  for(i = 0; i < 6; i++){
    printf("                    %d ", i);
    for(j = 0; j < 5; j++){
   	  location = getLocationFromXY(j, i); //ȡ�������±�

      if(engine == WHITE){ //�������ְ׷�����ת����λ��
        location = flipLocation(location);
      }
      piece = board[location];
      if(piece == 8){
        printf("��");
      }else if(piece == 9){
        printf("��");
      }else if(piece == 10){
        printf("��");
      }else if(piece == 11){
        printf("��");
      }else if(piece == 16){
        printf("��");
      }else if(piece == 17){
        printf("��");
      }else if(piece == 18){
        printf("��");
      }else if(piece == 19){
        printf("��");
      }else if(piece == NOPIECE){
        printf("��");
      }else{
        printf(" ");
      }
    }
    printf("\n");
  }
  printf("                      **********\n");
  printf("                      0 1 2 3 4x\n\n");
}

////////////////////////////////////////////////////////////////////////////////
//                              ��������ʾ����                                //
////////////////////////////////////////////////////////////////////////////////

int main(void){
  int move, from, to, fromX, fromY, toX, toY;
  char command, captured;

  //�������������
  srand((long)time(NULL));

  /* *********�����ô���*********
  int i, genCount, value, eatCount;

  genCount = generateAllMoves(theMoves); //���ɿ��������߷�

  printf("����ʱ���׷���%d���߷����ֱ��ǣ�\n", genCount);
  for(i = 0; i < genCount;i++){
      printf("from %d to %d \n", generateMoveFrom(theMoves[i]), generateMoveTo(theMoves[i]));
  }

  printf("����ʱ������ļ�ֵ�ǣ�%d\n", evaluatePosition());

  value = AlphaBetaSearch(6, -INFINITY_VALUE, INFINITY_VALUE);

  printf("�����������棬�ҵ�����Ѿ������֣�%d\n", value);

  printf("�����������棬�ҵ�������߷��ǣ�from %d to %d \n", generateMoveFrom(bestMove), generateMoveTo(bestMove));
  */

  ///*
  printf("/////////////////////////////\n");
  printf("//    SmallChess1.0   //\n");
  printf("/////////////////////////////\n");
  printf("\n��ʼ��\n\n");

  showBoard();

  while(1){
    printf("\n����Q�˳�������D���壺");
    command = getch();
    if(command == 'q' || command == 'Q'){
      return 0;
    }
    if(command == 'd' || command == 'D'){ //ѡ��������

      printf("\n����Wѡ����壬����Bѡ����壬����C�ۿ����Զ��ģ�");
      command = getch();
      if(command == 'w' || command == 'W'){ //��ѡ���˰���
        engine = BLACK; //��������Ϊ����
        printf("\n��ѡ���˰��壡\n");
      }else if(command == 'b' || command == 'B'){ //��ѡ���˺���
        engine = WHITE; //��������Ϊ����
        printf("\n��ѡ���˺��壡\n");
      }else if(command == 'c' || command == 'C'){ //��ѡ���˺���
        engine = 2; //���õ��Զ��ı�־
        printf("\n���Զ��Ŀ�ʼ��\n");
      }else{
        continue;
      }
      while(1){
        if(isThePlayerDie(currentPlayer) && currentPlayer != engine){
          printf("\n������˼������ս�ܣ�����һ�������´򿪳���\n");
          printf("\n����q�˳���");
          command = getch();
          if(command == 'q' || command == 'Q'){
            return 0;
          }
        }else if(isThePlayerDie(currentPlayer) && currentPlayer == engine){
          printf("\n��ϲ�㣬���Ա����ܣ�����һ�������´򿪳���\n");
          printf("\n����q�˳���");
          command = getch();
          if(command == 'q' || command == 'Q'){
            return 1;
          }
        }else{
          if(engine == WHITE){ //����Ϊ����
            showBoard();
            if(1){ //������Գְ׷�������
              printf("\n��������˼��...\n");
              computerThink();
              move = bestMove;
              from = generateMoveFrom(move); //�ֽ��߷�
              to = generateMoveTo(move);
              from = flipLocation(from); //��תλ��
              to = flipLocation(to);
              fromX = getXFromLocation(from); //ȡ������
			  fromY = getYFromLocation(from);
			  toX = getXFromLocation(to);
			  toY = getYFromLocation(to);
              printf("\n�������ߵ����ǣ�from (%d,%d) to (%d,%d)\n", fromX, fromY, toX, toY);
              showBoard();
            }
            if(isThePlayerDie(currentPlayer)){ //�������Ѱ�
              continue;
            }
            if(1){ //�ֵ�������
              printf("\n�ֵ������壬���������� x,y x,y ��");
              scanf("%d,%d %d,%d", &fromX, &fromY, &toX, &toY);
              printf("\n���ߵ����ǣ�from (%d,%d) to (%d,%d)\n", fromX, fromY, toX, toY);
              from = getLocationFromXY(fromX, fromY); //�ϳ�λ��
              to = getLocationFromXY(toX, toY);
              from = flipLocation(from); //��תλ��
              to = flipLocation(to);
              move = composeMove(from, to); //�ϳ��߷�
              while(!makeOneMove(move, &captured)){
                printf("\n������������������������꣺");
                scanf("%d,%d %d,%d", &fromX, &fromY, &toX, &toY);
                from = getLocationFromXY(fromX, fromY);//�ϳ�λ��
                to = getLocationFromXY(toX, toY);
                from = flipLocation(from); //��תλ��
                to = flipLocation(to);
                move = composeMove(from, to); //�ϳ��߷�
                getchar();
              }
            }
          }

          if(engine == BLACK){ //����Ϊ�ڷ�
            showBoard();
            if(1){ //�ֵ�������
              printf("\n�ֵ������壬���������� x,y x,y ��");
              scanf("%d,%d %d,%d", &fromX, &fromY, &toX, &toY);
              printf("\n���ߵ����ǣ�from (%d,%d) to (%d,%d)\n", fromX, fromY, toX, toY);
              from = getLocationFromXY(fromX, fromY);//�ϳ�λ��
              to = getLocationFromXY(toX, toY);
              move = composeMove(from, to); //�ϳ��߷�
              while(!makeOneMove(move, &captured)){
                printf("\n������������������������꣺");
                scanf("%d,%d %d,%d", &fromX, &fromY, &toX, &toY);
                from = getLocationFromXY(fromX, fromY);//�ϳ�λ��
                to = getLocationFromXY(toX, toY);
                move = composeMove(from, to); //�ϳ��߷�
                getchar();
              }
              showBoard();
            }
            if(isThePlayerDie(currentPlayer)){ //��������Ѱ�
              continue;
            }
            if(1){ //������Ժڷ������
              printf("\n��������˼��...\n");
              computerThink();
              move = bestMove;
              from = generateMoveFrom(move); //�ֽ��߷�
              to = generateMoveTo(move);
              fromX = getXFromLocation(from); //ȡ������
			  fromY = getYFromLocation(from);
			  toX = getXFromLocation(to);
			  toY = getYFromLocation(to);
              printf("\n�������ߵ����ǣ�from (%d,%d) to (%d,%d)\n", fromX, fromY, toX, toY);
            }
          }

          if(engine == 2){ //�����ս��־
            showBoard();
            if(1){ //����A������
              printf("\n����A����˼��...\n");
              //sleep(3000);
              computerThink();
              move = bestMove;
              from = generateMoveFrom(move); //�ֽ��߷�
              to = generateMoveTo(move);
              fromX = getXFromLocation(from); //ȡ������
			  fromY = getYFromLocation(from);
			  toX = getXFromLocation(to);
			  toY = getYFromLocation(to);
              printf("\n����A���ߵ����ǣ�from (%d,%d) to (%d,%d)\n", fromX, fromY, toX, toY);
              showBoard();
            }
            if(isThePlayerDie(currentPlayer)){ //�������Ѱ�
              continue;
            }
            if(1){ //�ֵ�����B����
              printf("\n����B����˼��...\n");
              //sleep(3000);
              computerThink();
              move = bestMove;
              from = generateMoveFrom(move); //�ֽ��߷�
              to = generateMoveTo(move);
              fromX = getXFromLocation(from); //ȡ������
			  fromY = getYFromLocation(from);
			  toX = getXFromLocation(to);
			  toY = getYFromLocation(to);
              printf("\n����B���ߵ����ǣ�from (%d,%d) to (%d,%d)\n", fromX, fromY, toX, toY);
            }
          }
        }
      }
    }
  }
  //*/
  return 0;
}
