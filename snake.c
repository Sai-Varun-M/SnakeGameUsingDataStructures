#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#define GAME_TITLE "THE SNAKE GAME!"
#define INSTRUCTIONS "Press ENTER to continue. Use the arrow keys to move the snake. Press P to pause the game."
#define GAME_OVER "GAME OVER"
#define PAUSE_TEXT "PAUSED. PRESS P TO CONTINUE"
#define SNAKE_BODY_PIECE '0'
#define SCORE_UNIT 10
#define OFF 0
#define PLAYING 1
#define GAME_OVER_MENU 2
#define PAUSED 3
#define KEY_SPACE 32
#define KEY_Q 113
#define KEY_P 112
typedef struct Point {
    int x, y;
    struct Point* next;
} Point;

typedef struct Vector {
    int vx, vy;
} Vector;

typedef struct Display {
    int rows, columns;
} Display;

typedef struct Snake {
    Point* head;
    int length;
    Vector vector;
} Snake;
typedef struct GameStatus {
    Snake* snake;
    Display display;
    Point foodPiece;
    int score;
    int inGame;
    int lastDirection;
    int numbersEaten[10]; // Change: Array to store numbers eaten by the snake
    int numCount; // Change: Count of numbers in the array
} GameStatus;
void addFood(GameStatus* gameStatus) {
    int max_x = gameStatus->display.columns - 2;
    int min_x = 1;
    int max_y = gameStatus->display.rows - 2;
    int min_y = 1;
    gameStatus->foodPiece.x = (rand() % (max_x - min_x)) + min_x;
    gameStatus->foodPiece.y = (rand() % (max_y - min_y)) + min_y;
}
void initGame(GameStatus* gameStatus) {
    gameStatus->snake = (Snake*)malloc(sizeof(Snake));
    gameStatus->snake->head = (Point*)malloc(sizeof(Point));
    gameStatus->snake->head->x = gameStatus->display.columns / 2;
    gameStatus->snake->head->y = gameStatus->display.rows / 2;
    gameStatus->snake->head->next = NULL;
    gameStatus->snake->length = 1;
    gameStatus->snake->vector.vx = 1;
    gameStatus->snake->vector.vy = 0;
    gameStatus->score = 0;
    gameStatus->inGame = PLAYING;
    gameStatus->lastDirection = KEY_RIGHT;
    gameStatus->foodPiece.x = 0;
    gameStatus->foodPiece.y = 0;
    addFood(gameStatus);
    // Initialize number count
    gameStatus->numCount = 0;
}
int collisionWithWalls(Snake* snake, Display* display) {
    Point* head = snake->head;
    return (head->x > display->columns - 2 || head->x < 1 ||
            head->y > display->rows - 2 || head->y < 1);
}
int hitFood(Snake* snake, Point* foodPiece) {
    Point* head = snake->head;
    return (head->x == foodPiece->x && head->y == foodPiece->y);
}
int hitsItself(Snake* snake) {
    Point* head = snake->head;
    Point* current = head->next;
    while (current != NULL) {
        if (current->x == head->x && current->y == head->y) {
            return 1;
        }
        current = current->next;
    }
    return 0;
}
void draw(GameStatus* gameStatus) {
    clear();
    if (gameStatus->inGame == PLAYING) {
        Point* newHead = (Point*)malloc(sizeof(Point));
        newHead->x = gameStatus->snake->head->x + gameStatus->snake->vector.vx;
        newHead->y = gameStatus->snake->head->y + gameStatus->snake->vector.vy;
        newHead->next = gameStatus->snake->head;
        gameStatus->snake->head = newHead;
        if (collisionWithWalls(gameStatus->snake, &gameStatus->display) || hitsItself(gameStatus->snake)) {
            gameStatus->inGame = GAME_OVER_MENU;
        } else if (hitFood(gameStatus->snake, &gameStatus->foodPiece)) {
            gameStatus->score += SCORE_UNIT;
            // Check if there is space in the array to store the number
            if (gameStatus->numCount < sizeof(gameStatus->numbersEaten) / sizeof(gameStatus->numbersEaten[0])) {
                gameStatus->numbersEaten[gameStatus->numCount++] = rand() % 100; // Add a random number to the array
            }
            addFood(gameStatus);
        } else {
            Point* current = gameStatus->snake->head;
            Point* prev = NULL;
            while (current->next != NULL) {
                prev = current;
                current = current->next;
            }
            free(current);
            prev->next = NULL;
        }
    }
    if (gameStatus->inGame == PLAYING) {
        Point* current = gameStatus->snake->head;
        while (current != NULL) {
            mvaddch(current->y, current->x, SNAKE_BODY_PIECE);
            current = current->next;
        }
        mvaddch(gameStatus->foodPiece.y, gameStatus->foodPiece.x, SCORE_UNIT + '0');
    } else if (gameStatus->inGame == GAME_OVER_MENU) {
        mvprintw(gameStatus->display.rows / 2, (gameStatus->display.columns - strlen(GAME_OVER)) / 2, GAME_OVER);
        mvprintw(gameStatus->display.rows / 2 + 2, (gameStatus->display.columns - strlen(INSTRUCTIONS)) / 2, INSTRUCTIONS);

        // Display the numbers eaten by the snake
        mvprintw(gameStatus->display.rows / 2 + 4, (gameStatus->display.columns - strlen("Numbers Eaten: ")) / 2, "Numbers Eaten: ");
        for (int i = 0; i < gameStatus->numCount; ++i) {
            mvprintw(gameStatus->display.rows / 2 + 6, (gameStatus->display.columns - strlen("00 ")) / 2 + i * 3, "%02d ", gameStatus->numbersEaten[i]);
        }
    } else if (gameStatus->inGame == PAUSED) {
        mvprintw(gameStatus->display.rows / 2, (gameStatus->display.columns - strlen(PAUSE_TEXT)) / 2, PAUSE_TEXT);
    }
    mvprintw(1, 1, "Score: %d", gameStatus->score);
    refresh();
}
int main() {
    srand(time(NULL));
    initscr();
    keypad(stdscr, TRUE);
    noecho();
    curs_set(0);
    Display display;
    getmaxyx(stdscr, display.rows, display.columns);
    Snake snake;
    GameStatus gameStatus;
    gameStatus.snake = &snake;
    gameStatus.display = display;
    initGame(&gameStatus);
    int command = KEY_RIGHT;
    int timer = 0;
    while (command != KEY_Q) {
        if (command == KEY_P) {
            if (gameStatus.inGame == PAUSED) {
                gameStatus.inGame = PLAYING;
            } else {
                gameStatus.inGame = PAUSED;
            }
        } else if (command == KEY_SPACE) {
            gameStatus.inGame = PLAYING;
        } else if (gameStatus.inGame == PLAYING && timer % 10 == 0) {
            Point* newHead = (Point*)malloc(sizeof(Point));
            newHead->x = gameStatus.snake->head->x + gameStatus.snake->vector.vx;
            newHead->y = gameStatus.snake->head->y + gameStatus.snake->vector.vy;
            newHead->next = gameStatus.snake->head;
            gameStatus.snake->head = newHead;
            if (collisionWithWalls(gameStatus.snake, &gameStatus.display) || hitsItself(gameStatus.snake)) {
                gameStatus.inGame = GAME_OVER_MENU;
            } else if (hitFood(gameStatus.snake, &gameStatus.foodPiece)) {
                gameStatus.score += SCORE_UNIT;
                // Check if there is space in the array to store the number
                if (gameStatus.numCount < sizeof(gameStatus.numbersEaten) / sizeof(gameStatus.numbersEaten[0])) {
                    gameStatus.numbersEaten[gameStatus.numCount++] = rand() % 100; // Add a random number to the array
                }
                addFood(&gameStatus);
            } else {
                Point* current = gameStatus.snake->head;
                Point* prev = NULL;
                while (current->next != NULL) {
                    prev = current;
                    current = current->next;
                }
                free(current);
                prev->next = NULL;
            }
        } else if (gameStatus.inGame == PLAYING) {
            switch (gameStatus.lastDirection) {
                case KEY_LEFT:
                    gameStatus.snake->vector.vx = -1;
                    gameStatus.snake->vector.vy = 0;
                    break;
                case KEY_RIGHT:
                    gameStatus.snake->vector.vx = 1;
                    gameStatus.snake->vector.vy = 0;
                    break;
                case KEY_UP:
                    gameStatus.snake->vector.vx = 0;
                    gameStatus.snake->vector.vy = -1;
                    break;
                case KEY_DOWN:
                    gameStatus.snake->vector.vx = 0;
                    gameStatus.snake->vector.vy = 1;
                    break;
            }
        }
        draw(&gameStatus);
        usleep(10000);
        timer++;

        command = getch();
        if (command != ERR) {
            gameStatus.lastDirection = command;
        }
    }
    endwin();
    return 0;
}
