
#ifdef WIN32
#include <Windows.h>
#include <conio.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "Common.h"
#include "ConsoleUtil.h"

#define YUTORI 1
#define VERY_EASY 10
#define EASY 50
#define NORMAL 100
#define HARD 300
#define GAME_LEVEL (EASY) // シャッフル時の移動回数
 
#define MAX(x, y) (x > y ? x : y)

// #define APP_DEBUG (1)

#ifdef APP_DEBUG
#define dprintf(fmt, ...) printf("[DEBUG] " ## fmt, __VA_ARGS__);
#else
#define dprintf(fmt, ...)
#endif

// 使用するデータを"#if 1"にして有効にする
// AAのピースとして使用可能なのは、全角文字もしくは半角文字x2
// 同一の絵柄のピースを区別するので、Debug表示のピースIDを見ないとムリゲー

#if 0
#define ROW 4     //行　縦
#define COLUMN 4  //列　横
#define BYTE_PER_ELEMENT 1 // 半角: 1, 全角: 2
#define BLANK_PIECE_ID (ROW * COLUMN -1)

const char *g_ArrayAA[] = {
                    "0123", 
                    "4567", 
                    "89ab", 
                    "cdef" };
#endif

#if 1
#define ROW 3     //行　縦
#define COLUMN 8  //列　横
#define BYTE_PER_ELEMENT 2 // 半角: 1, 全角: 2
#define BLANK_PIECE_ID (ROW * COLUMN -1)

const char *g_ArrayAA[] = {
                    "　　_, ,_ パーン", 
                    "（　‘д‘）　　", 
                    "　⊂彡☆))Д´）"};
#endif

#if 0
#define ROW 4     //行　縦
#define COLUMN 11  //列　横
#define BYTE_PER_ELEMENT 2 // 半角: 1, 全角: 2
#define BLANK_PIECE_ID (ROW * COLUMN -1)

const char *g_ArrayAA[] = {
                    "　∧,,∧　　　　　　　", 
                    "(`・ω・）　　｡･ﾟ･⌒）", 
                    "/ 　　ｏ━ヽニニフ))　", 
                    "しー--Ｊ　　　　　　　" };
#endif

typedef enum Way
{
    LEFT,
    RIGHT,
    UP,
    DOWN,
    NONE,
    NUM_WAY,
} Way;

struct Piece
{
    int pieceID;
    char pieceChar[2];
};

struct Size
{
    int column;
    int row;
};

struct Pos
{
    int x;
    int y;
};

void playPuzzle(struct Piece **pieceContainer);

struct Size getPuzzleDataSize();

struct Pos getBlankPiecePos(const struct Piece **array);

void setArrayData(struct Piece **array);

enum Way getUserInputDirection();

int isClear(const struct Piece **array);

void drawResult(const struct Piece **array);

void drawArray(const struct Piece **array, int isClear);

int movePuzzlePiece(struct Piece **array, const struct Pos* blankPos, Way moveWay);

void shufflePuzzlePiece(struct Piece **array);

void printPieceIDs(const struct Piece **array);

int main(void)
{
    struct Piece **pieceContainer;
    int m, n;

    pieceContainer = (struct Piece **)malloc(sizeof(struct Piece*) * ROW);
    for(m = 0; m < ROW; m++) {
        pieceContainer[m] = (struct Piece *)malloc(sizeof(struct Piece) * COLUMN);
    }

    for(m = 0; m < ROW; m++) {
        for(n = 0; n < COLUMN; n++) {
            pieceContainer[m][n].pieceID = m * COLUMN + n;
        }
    }

    playPuzzle(pieceContainer);

    for(m = 0; m < ROW; m++) {
        free(pieceContainer[m]);
    }
    free(pieceContainer);

    return 0;
}

void playPuzzle(struct Piece **pieceContainer)
{
    Way moveWay;
    struct Pos blankPos;

    setArrayData(pieceContainer);

    shufflePuzzlePiece(pieceContainer);
    drawResult((const struct Piece**)pieceContainer);

    while(!isClear((const struct Piece**)pieceContainer)) {
        moveWay = getUserInputDirection();
        if(moveWay != NONE) {
            blankPos = getBlankPiecePos((const struct Piece**)pieceContainer);
            movePuzzlePiece(pieceContainer, &blankPos, moveWay);
            drawResult((const struct Piece**)pieceContainer);
        }
    }

    printf("完成！おめでとう！！\n");
    printEndAnimation();
    printf("何かボタンを押してね\n");

    getch();
}

void loadPieceData(struct Piece* piece, const char* srcData) {
    piece->pieceChar[0] = *srcData;
    if(BYTE_PER_ELEMENT == 2) {
        piece->pieceChar[1] = *(srcData + 1); // 2byteデータのピースは2byte分のデータをコピー
    }
}

void setArrayData(struct Piece** array)
{
    int i, j, row = 0;
    int column = sizeof(g_ArrayAA) / sizeof(char*);

    for(i = 0; i < column; i++) {
        int row = strlen(g_ArrayAA[i]);
        for(j = 0; j < row / BYTE_PER_ELEMENT; j++)
            loadPieceData(&array[i][j], &g_ArrayAA[i][j * BYTE_PER_ELEMENT]);
    }
}

struct Size getPuzzleDataSize()
{
    int i, column = 0, max_column = 0;
    int row = sizeof(g_ArrayAA) / sizeof(char*);
    struct Size retSize;

    for(i = 0; i < row; i++) {
        column = strlen(g_ArrayAA[i]);
        max_column = MAX(column, max_column);
    }

    retSize.column = max_column / BYTE_PER_ELEMENT;
    retSize.row = row;

    dprintf("Array: row = %d, column  = %d\n", retSize.row, retSize.column);

    return retSize;
}

struct Pos getBlankPiecePos(const struct Piece** array)
{
    struct Pos retPos;
    int i, j;

    for(i = 0; i < ROW; i++) {
        for(j = 0; j < COLUMN; j++) {
            if(array[i][j].pieceID == BLANK_PIECE_ID) {
                retPos.x = j;
                retPos.y = i;
            }
        }
    }

    return retPos;
}
 
enum Way convertKeyCodeToDirection(int keyCode)
{
    Way moveWay = NONE;

    if(keyCode == 0x4b) {
        moveWay = LEFT;
    }
    else if(keyCode == 0x48) {
        moveWay = UP;
    }
    else if(keyCode == 0x50) {
        moveWay = DOWN;
    }
    else if(keyCode == 0x4d) {
        moveWay = RIGHT;
    }

    return moveWay;
}

enum Way getUserInputDirection()
{
    int keyCode = 0;

    while(keyCode == 0) {
        keyCode = getch(); 
    }
    if(keyCode == 0xe0) keyCode = getch();

    return convertKeyCodeToDirection(keyCode);
}

int isClear(const struct Piece **array)
{
    int m, n, isClear = TRUE;

    for(m = 0; m < ROW; m++) {
        for(n = 0; n < COLUMN; n++) {
            if(array[m][n].pieceID != m * COLUMN + n) isClear = FALSE;
        }
    }
    return isClear;
}

void swapPiece(struct Piece **array, struct Pos blankPos, struct Pos dstPos)
{
    struct Piece tmp;

    dprintf("blankPos[%d, %d] -> dstPos[%d, %d]\n", blankPos.x, blankPos.y, dstPos.x, dstPos.y);

    tmp = array[blankPos.y][blankPos.x];
    array[blankPos.y][blankPos.x] = array[dstPos.y][dstPos.x];
    array[dstPos.y][dstPos.x] = tmp;
}

int canMovePiece(struct Size arraySize, struct Pos dstPos)
{
    int canMovePiece = FALSE;

    if(0 <= dstPos.x && dstPos.x < arraySize.column)
        if(0 <= dstPos.y && dstPos.y < arraySize.row)
            canMovePiece = TRUE;

    return canMovePiece;
}

int movePuzzlePiece(struct Piece **array, const struct Pos* blankPos, Way moveWay)
{
    struct Pos dstPos = *blankPos;
    struct Size arraySize = {COLUMN, ROW};
    int movedPiece = FALSE;

    switch(moveWay) {
    case LEFT:
        dstPos.x = dstPos.x - 1;
        break;
    case RIGHT:
        dstPos.x = dstPos.x + 1;
        break;
    case UP:
        dstPos.y = dstPos.y - 1;
        break;
    case DOWN:
        dstPos.y = dstPos.y + 1;
        break;
    default:
        printf("Error: invalid direction\n");
        break;
    }

    if(canMovePiece(arraySize, dstPos)) {
        swapPiece(array, *blankPos, dstPos);
        movedPiece = TRUE;
    }

    return movedPiece;
}

void shufflePuzzlePiece(struct Piece **array)
{
    Way wayTable[] = {LEFT, RIGHT, UP, DOWN};
    Way moveWay;
    struct Pos blankPos;
    int moveCount = 0, tableSize;

    srand((unsigned int)time(NULL));
    tableSize = sizeof(wayTable) / sizeof(Way);
    dprintf("size table: %d\n", tableSize);
    
    while(moveCount < GAME_LEVEL) {
        moveWay = wayTable[rand() % tableSize];
        blankPos = getBlankPiecePos((const struct Piece**)array);
        if(movePuzzlePiece(array, &blankPos, moveWay))
            moveCount++;
    }
}

void drawResult(const struct Piece **array)
{
    int status = isClear(array);

#ifdef WIN32
#ifndef APP_DEBUG
    system("cls");
#endif
#endif

    drawArray(array, status);
    printf("Print Puzzle Piece ID (Blank ID = %d)\n\n", BLANK_PIECE_ID);
    printPieceIDs(array);

    if(!status) printf("矢印キー [←↑→↓] で操作\n");
}

void drawPiece(const struct Piece piece, int isClear)
{
    if(BYTE_PER_ELEMENT == 2) {  // 2byte文字の表示
        if(piece.pieceID == BLANK_PIECE_ID && !isClear)
            printf("%c%c", ' ', ' ');
        else
            printf("%c%c", piece.pieceChar[0], piece.pieceChar[1]);
    } else {
        if(piece.pieceID == BLANK_PIECE_ID && !isClear)
            printf("%c", ' ');
        else
            printf("%c", piece.pieceChar[0]);
    }
}

void drawArray(const struct Piece **array, int isClear)
{
    int i, j;

    for(i = 0; i < ROW; i++) {
        for(j = 0; j < COLUMN; j++)
            drawPiece(array[i][j], isClear); 
        printf("\n");
    }
    printf("\n");
}

void printPieceIDs(const struct Piece **array)
{
    int i, j;

    for(i = 0; i < ROW; i++) {
        for(j = 0; j < COLUMN; j++) {
            printf("%2d ", array[i][j].pieceID);
        }
        printf("\n");
    }
    printf("\n");
}
