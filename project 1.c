#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <Windows.h>
#include <time.h>

#pragma warning(disable:4996)

//colors
#define RED 12
#define BLUE 3
#define GREEN 10
#define YELLOW 14
#define GRAY 8
#define PINK 13
#define WHITE 15
#define WAIT_TIME_MILI_SEC 100
//directions
#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3
// general
#define BOARD_SIZE 40
#define INITIAL_SNAKE_LENGTH 3
#define MINIMUM_SNAKE_LENGTH 2
#define MAX_LEN_SNAKES 30
#define NUMBER_OF_MOUSES 20
//board_characters
#define EMPTY '0'
#define MOUSE 'm'
#define PLAYER1_SNAKE_HEAD '1'
#define PLAYER2_SNAKE_HEAD '2'
#define PLAYER1_SNAKE_BODY 'a'
#define PLAYER2_SNAKE_BODY 'b'
//Bernard, Poison and golden star
#define BERNARD_CLOCK 'c' //on the board character
#define GOLDEN_STAR '*' //on the board character
#define POISON 'x' //on the board character
#define NUMBER_OF_POISONS 5
#define NUMBER_OF_GOLDEN_STARS 3
#define BERNARD_CLOCK_APPEARANCE_CHANCE_PERCENT 20
#define BERNARD_CLOCK_APPEARANCE_CHECK_PERIOD_MILI_SEC 2000
#define BERNARD_CLOCK_FROZEN_TIME_MILI_SEC 4000

CONSOLE_FONT_INFOEX former_cfi;
CONSOLE_CURSOR_INFO former_info;
COORD former_screen_size;

void reset_console() {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleDisplayMode(consoleHandle, CONSOLE_WINDOWED_MODE, &former_screen_size);
    SetCurrentConsoleFontEx(consoleHandle, FALSE, &former_cfi);
    SetConsoleCursorInfo(consoleHandle, &former_info);
}

void hidecursor(HANDLE consoleHandle)
{
    GetConsoleCursorInfo(consoleHandle, &former_info);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(consoleHandle, &info);
}

void set_console_font_and_font_size(HANDLE consoleHandle) {
    former_cfi.cbSize = sizeof(former_cfi);
    GetCurrentConsoleFontEx(consoleHandle, FALSE, &former_cfi);
    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof(cfi);
    cfi.nFont = 0;
    cfi.dwFontSize.X = 20;
    cfi.dwFontSize.Y = 20;
    cfi.FontFamily = FF_DONTCARE;
    cfi.FontWeight = FW_NORMAL;
    wcscpy(cfi.FaceName, L"Courier");
    SetCurrentConsoleFontEx(consoleHandle, FALSE, &cfi);
}

void set_full_screen_mode(HANDLE consoleHandle) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    former_screen_size.X = csbi.dwSize.X; former_screen_size.Y = csbi.dwSize.Y;
    COORD coord;
    SetConsoleDisplayMode(consoleHandle, CONSOLE_FULLSCREEN_MODE, &coord);
}

void init_screen()
{
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    set_full_screen_mode(consoleHandle);
    hidecursor(consoleHandle);
    set_console_font_and_font_size(consoleHandle);

}

void wait_and_get_direction(int* player1_snake_direction, int* player2_snake_direction) {
    DWORD64 start_time, check_time;
    start_time = GetTickCount64();
    check_time = start_time + WAIT_TIME_MILI_SEC; //GetTickCount returns time in miliseconds
    char key = 0;
    char player1_key_hit = 0;
    char player2_key_hit = 0;

    while (check_time > GetTickCount64()) {
        if (_kbhit()) {
            key = _getch();
            if (key == 0)
                key = _getch();
            if (key == 'w' || key == 'a' || key == 's' || key == 'd')
                player1_key_hit = key;
            if (key == 'i' || key == 'j' || key == 'k' || key == 'l')
                player2_key_hit = key;
        }
    }

    switch (player1_key_hit) {
    case 'w': if (*player1_snake_direction != DOWN) *player1_snake_direction = UP; break;
    case 'a': if (*player1_snake_direction != RIGHT) *player1_snake_direction = LEFT; break;
    case 's': if (*player1_snake_direction != UP) *player1_snake_direction = DOWN; break;
    case 'd': if (*player1_snake_direction != LEFT) *player1_snake_direction = RIGHT; break;
    default: break;
    }

    switch (player2_key_hit) {
    case 'i': if (*player2_snake_direction != DOWN) *player2_snake_direction = UP; break;
    case 'j': if (*player2_snake_direction != RIGHT) *player2_snake_direction = LEFT; break;
    case 'k': if (*player2_snake_direction != UP) *player2_snake_direction = DOWN; break;
    case 'l': if (*player2_snake_direction != LEFT) *player2_snake_direction = RIGHT; break;
    default: break;
    }
}

void draw_point(char point_content) {
    switch (point_content) {
    case PLAYER1_SNAKE_HEAD: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), RED); printf("@"); break;
    case PLAYER2_SNAKE_HEAD: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BLUE);  printf("@"); break;
    case PLAYER1_SNAKE_BODY: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), RED);  printf("o"); break;
    case PLAYER2_SNAKE_BODY: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BLUE);  printf("o"); break;
    case MOUSE: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GRAY); printf("m"); break;
    case GOLDEN_STAR: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), YELLOW); printf("*"); break;
    case POISON: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GREEN); printf("x"); break;
    case BERNARD_CLOCK: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), PINK); printf("c"); break;
    default: printf(" ");
    }
}

void draw_horizonatal_walls() {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE);
    for (int i = 0; i < BOARD_SIZE + 2; ++i)
        printf("-");
    printf("\n");
}

void draw_board(char board_content[BOARD_SIZE][BOARD_SIZE]) {
    system("cls");
    draw_horizonatal_walls();
    for (int i = 0; i < BOARD_SIZE; i++) {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE);
        printf("|"); // vertical wall 
        for (int j = 0; j < BOARD_SIZE; j++)
            draw_point(board_content[i][j]);
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE);
        printf("|\n"); // vertical wall
    }
    draw_horizonatal_walls();
}

// prototypes
void init_screen();
void reset_console();
void wait_and_get_direction(int* player1_snake_direction, int* player2_snake_direction);
void draw_board(char board_content[BOARD_SIZE][BOARD_SIZE]);

// my functions

//shifts the coordinates of each part of the snake to the next column of snake array
void shift_snake(int snake[MAX_LEN_SNAKES + 1][2], int length)
{
    for (int i = length; i > 0; i--)
    {
        snake[i][0] = snake[i - 1][0];
        snake[i][1] = snake[i - 1][1];
    }
}

//changes the coordinates of the head, depending on the snake's direction 
//checks if it has hit the wall and if so, changes the coordinates so that it comes out from the other side
void move_snake(int snake[MAX_LEN_SNAKES + 1][2], int direction)
{
    switch (direction)
    {
    case RIGHT:
        snake[0][0]++;
        if (snake[0][0] == BOARD_SIZE)  //check hitting the right wall
            snake[0][0] = 0;
        break;
    case UP:
        snake[0][1]--;
        if (snake[0][1] == -1)  //check hitting the top wall
            snake[0][1] = BOARD_SIZE - 1;
        break;
    case LEFT:
        snake[0][0]--;
        if (snake[0][0] == -1)  //check hitting the left wall
            snake[0][0] = BOARD_SIZE - 1;
        break;
    case DOWN:
        snake[0][1]++;
        if (snake[0][1] == BOARD_SIZE)  //check hitting the bottom wall
            snake[0][1] = 0;
        break;
    default:
        break;
    }
}

//keeps generating random numbers as coordinates untill the spot with those coordinates is empty in board_content, then it puts a mouse in that spot
void draw_mouse(char board_content[BOARD_SIZE][BOARD_SIZE])
{
    int RandX, RandY;
    time_t t;
    do {
        srand((unsigned)time(&t));
        RandX = rand() % (BOARD_SIZE);
        RandY = rand() % (BOARD_SIZE);
    } while (board_content[RandX][RandY] != EMPTY);
    board_content[RandX][RandY] = MOUSE;
}

//keeps generating random numbers as coordinates untill the spot with those coordinates is empty in board_content, then it puts a star in that spot
void draw_star(char board_content[BOARD_SIZE][BOARD_SIZE])
{
    int RandX, RandY;
    time_t t;
    do {
        srand((unsigned)time(&t));
        RandX = rand() % (BOARD_SIZE);
        RandY = rand() % (BOARD_SIZE);
    } while (board_content[RandX][RandY] != EMPTY);
    board_content[RandX][RandY] = GOLDEN_STAR;
}

//gets the number of the winner, resets the console, prints the winner
void GameOver(int SnakeNum)
{
    switch (SnakeNum)
    {
    case 1:
        system("cls");
        reset_console();
        printf(" ------------\n|PLAYER 1 WON|\n ------------\n");
        break;
    case 2:
        system("cls");
        reset_console();
        printf(" ------------\n|PLAYER 2 WON|\n ------------\n");
        break;
    case 0:
        system("cls");
        reset_console();
        printf(" ----\n|DRAW|\n ----\n");
        break;
    default:
        break;
    }
}

//if the snake hasn't reached the MAX_LEN_SNAKES, it checks if the snake's head is in the same spot as a mouse, if so, the length increases
//puts a new mouse in a random spot, whenever one is eaten
void check_mouse(int SnakeNum, char board_content[BOARD_SIZE][BOARD_SIZE], int snake[MAX_LEN_SNAKES + 1][2], int* length)
{
    if ((*length) != MAX_LEN_SNAKES)
    {
        if (board_content[snake[0][1]][snake[0][0]] == MOUSE)
        {
            snake[*length + 1][0] = snake[*length][0];
            snake[*length + 1][1] = snake[*length][1];
            if (SnakeNum == 1)
                board_content[snake[*length][1]][snake[*length][0]] = PLAYER1_SNAKE_BODY;
            else
                board_content[snake[*length][1]][snake[*length][0]] = PLAYER2_SNAKE_BODY;
            *length += 1;
            draw_mouse(board_content);
        }
    }
}

//checks if the snake's head is in the same spot as a snake's body, if so, the other player wins, and the game is over
void check_body(int SnakeNum, char board_content[BOARD_SIZE][BOARD_SIZE], int snake_head_x, int snake_head_y, int* End)
{
    if (board_content[snake_head_y][snake_head_x] == PLAYER1_SNAKE_BODY || board_content[snake_head_y][snake_head_x] == PLAYER2_SNAKE_BODY)
    {
        GameOver(3 - SnakeNum);
        *End = 1;
    }
}

//checks if the snake's head is in the same spot as a snake's body, if so, the player with the longer snake wins, and the game is over
void check_head(int snake1_head_x, int snake1_head_y, int snake2_head_x, int snake2_head_y, int L1, int L2, int* End, char board_content[BOARD_SIZE][BOARD_SIZE])
{
    if (snake1_head_x == snake2_head_x && snake1_head_y == snake2_head_y)
    {
        if (L1 > L2) {
            GameOver(1);
        }
        else if (L1 < L2)
            GameOver(2);
        else
            GameOver(0);
        *End = 1;
    }

}

//checks the length of two snakes, if one of them is shorter than MINIMUM_SNAKE_LENGTH, the other one wins, if they both are shorter, it's a draw, and the game is over
void check_length(int L1, int L2, char board_content[BOARD_SIZE][BOARD_SIZE], int* End)
{
    if (L1 < MINIMUM_SNAKE_LENGTH && L2 < MINIMUM_SNAKE_LENGTH)
    {
        GameOver(0);
        *End = 1;
    }
    else if (L1 < MINIMUM_SNAKE_LENGTH)
    {
        GameOver(2);
        *End = 1;
    }
    else if (L2 < MINIMUM_SNAKE_LENGTH)
    {
        GameOver(1);
        *End = 1;
    }
}

//checks if any of the snakes' heads are in the smae spot as a star, if so, the other snake's length decreases 
//puts a new star in a random spot, whenever one is used
void check_star(int Snake1[MAX_LEN_SNAKES + 1][2], int Snake2[MAX_LEN_SNAKES + 1][2], int* L1, int* L2, char board_content[BOARD_SIZE][BOARD_SIZE])
{
    if (board_content[Snake1[0][1]][Snake1[0][0]] == GOLDEN_STAR)
    {
        Snake2[*L2][0] = 0;
        Snake2[*L2][1] = 0;
        *L2 -= 1;
        board_content[Snake2[*L2][1]][Snake2[*L2][0]] = EMPTY;
        draw_star(board_content);
    }
    if (board_content[Snake2[0][1]][Snake2[0][0]] == GOLDEN_STAR)
    {
        Snake1[*L1][0] = 0;
        Snake1[*L1][1] = 0;
        *L1 -= 1;
        board_content[Snake1[*L1][1]][Snake1[*L1][0]] = EMPTY;
        draw_star(board_content);
    }
}

//initializing the first values of the board_content and snakes arrays
void initialize_first_board(char board_content[BOARD_SIZE][BOARD_SIZE], int snake1[MAX_LEN_SNAKES + 1][2], int snake2[MAX_LEN_SNAKES + 1][2])
{
    //sets all elements of board_content to EMPTY
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
            board_content[i][j] = EMPTY;
    }

    //gives first values to snakes arrays
    for (int i = 0; i < 3; i++)
    {
        snake1[i][0] = 0;
        snake1[i][1] = 2 - i;
        snake2[i][0] = BOARD_SIZE - 1;
        snake2[i][1] = BOARD_SIZE - (3 - i);
    }

    //puts the snakes on the board
    board_content[snake1[0][1]][snake1[0][0]] = PLAYER1_SNAKE_HEAD;
    board_content[snake2[0][1]][snake2[0][0]] = PLAYER2_SNAKE_HEAD;
    board_content[snake1[1][1]][snake1[1][0]] = PLAYER1_SNAKE_BODY;
    board_content[snake2[1][1]][snake2[1][0]] = PLAYER2_SNAKE_BODY;
    board_content[snake1[2][1]][snake1[2][0]] = PLAYER1_SNAKE_BODY;
    board_content[snake2[2][1]][snake2[2][0]] = PLAYER2_SNAKE_BODY;

    //puts 20 mice on random spots on board
    for (int i = 0; i < NUMBER_OF_MOUSES; i++)
    {
        draw_mouse(board_content);
    }

    //puts 3 golden stars on random spots on board
    for (int i = 0; i < NUMBER_OF_GOLDEN_STARS; i++)
    {
        draw_star(board_content);
    }
}


int main()
{
    init_screen();

    int snake1[MAX_LEN_SNAKES + 1][2] = { NULL }, snake2[MAX_LEN_SNAKES + 1][2] = { NULL };     //for saving the coordinates of each part of the snakes
    char board_content[BOARD_SIZE][BOARD_SIZE];
    int player1_snake_direction = DOWN, player2_snake_direction = UP;
    int LSnake1 = INITIAL_SNAKE_LENGTH, LSnake2 = INITIAL_SNAKE_LENGTH;
    int End = 0;

    initialize_first_board(board_content, snake1, snake2);

    while (!End)
    {
        draw_board(board_content);

        //gets command from the players
        wait_and_get_direction(&player1_snake_direction, &player2_snake_direction);

        //moving the bodies
        shift_snake(snake1, LSnake1);
        shift_snake(snake2, LSnake2);

        board_content[snake1[LSnake1][1]][snake1[LSnake1][0]] = EMPTY;
        board_content[snake2[LSnake2][1]][snake2[LSnake2][0]] = EMPTY;

        board_content[snake1[1][1]][snake1[1][0]] = PLAYER1_SNAKE_BODY;
        board_content[snake2[1][1]][snake2[1][0]] = PLAYER2_SNAKE_BODY;

        //moving the heads
        move_snake(snake1, player1_snake_direction);
        move_snake(snake2, player2_snake_direction);

        //checking if there is anything in the spots the snakes have entered
        check_mouse(1, board_content, snake1, &LSnake1);
        check_mouse(2, board_content, snake2, &LSnake2);

        check_body(1, board_content, snake1[0][0], snake1[0][1], &End);
        check_body(2, board_content, snake2[0][0], snake2[0][1], &End);

        check_head(snake1[0][0], snake1[0][1], snake2[0][0], snake2[0][1], LSnake1, LSnake2, &End, board_content);

        check_star(snake1, snake2, &LSnake1, &LSnake2, board_content);

        board_content[snake1[0][1]][snake1[0][0]] = PLAYER1_SNAKE_HEAD;
        board_content[snake2[0][1]][snake2[0][0]] = PLAYER2_SNAKE_HEAD;

        //checking if any of the snakes are shorter than MINIMUM_SNAKE_LENGTH
        check_length(LSnake1, LSnake2, board_content, &End);
    }
}