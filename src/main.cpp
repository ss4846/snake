#include "raylib.h"
#include <deque>
#include <random>

const int CELL_SIZE = 30;       //each cell is 30x30
const int GRID_COLS = 20;       //no.of cells horizontally
const int GRID_ROWS = 20;       //no.of cells vertically

struct Cell{
    int x;
    int y;
};

Cell spawnFood(const std::deque<Cell>& snake){
    Cell food;
    bool onSnake = false;

    do {
        std::random_device rd;
        std::mt19937 gen(rd());

        std::uniform_int_distribution<int> dist_rows(0, GRID_ROWS-1);
        std::uniform_int_distribution<int> dist_cols(0, GRID_COLS-1);

        food = {dist_cols(gen), dist_rows(gen)};

        for ( const Cell& c: snake ){
            if (c.x == food.x && c.y == food.y) onSnake = true;
        }
    } while (onSnake);

    return food;
}

enum class GameState {Playing, GameOver};

void ResetGame(std::deque<Cell>& snake, Cell& direction, Cell& nextDirection,float& moveTimer, Cell& food, GameState& state) {
    snake = { {10, 10}, {9, 10}, {8, 10} };             //head is first(cell co-ords not pixels)
    direction = {1, 0};                                 //1 cell right per move
    nextDirection = direction;
    moveTimer = 0.0f;                                   //time since last move
    food = spawnFood(snake);
    state = GameState::Playing;
}


int main(){
    int screenWidth = 600;
    int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Snake");
    SetTargetFPS(60);

    std::deque<Cell> snake;    
    Cell direction;                                   
    Cell nextDirection;
    float moveTimer;                                     
    const float MOVE_INTERVAL = 0.15f;            //move every 0.15s
    Cell food;
    GameState state = GameState::Playing;

    ResetGame(snake, direction, nextDirection, moveTimer, food, state);
    

    while(!WindowShouldClose()){

        moveTimer += GetFrameTime();        //seconds since last frame

        //game reset
        if (state == GameState::GameOver && IsKeyPressed(KEY_R)){
            ResetGame(snake, direction, nextDirection, moveTimer, food, state);
        }

        //inputs
        if (IsKeyPressed(KEY_UP) && direction.y != 1) {
            nextDirection = {0, -1};
        }
        if (IsKeyPressed(KEY_DOWN) && direction.y != -1) {
            nextDirection = {0, 1};
        }
        if (IsKeyPressed(KEY_LEFT) && direction.x != 1) {
            nextDirection = {-1, 0};
        }
        if (IsKeyPressed(KEY_RIGHT) && direction.x != -1) {
            nextDirection = {1, 0};
        }

        if (moveTimer >= MOVE_INTERVAL && state == GameState::Playing) {
            moveTimer -= MOVE_INTERVAL;

            //snake movement
            direction = nextDirection;
            Cell new_head = {snake.front().x + direction.x, snake.front().y + direction.y};
            
            //dead condition
            bool dead = false;
            if (new_head.x < 0 || new_head.x >= GRID_COLS || new_head.y < 0 || new_head.y >= GRID_ROWS) dead = true;
            for (const Cell& c: snake){
                if (new_head.x == c.x && new_head.y == c.y) dead = true;
            }
            if (dead){
                state = GameState::GameOver;
            } else{
                snake.push_front(new_head);
                if (new_head.x == food.x && new_head.y == food.y) {
                    food = spawnFood(snake);
                } else {
                    snake.pop_back();
                }
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        for(int i=1; i<20; i++){                                                    //horizontal lines
            DrawLine(0, i*CELL_SIZE, screenWidth, i*CELL_SIZE, DARKGRAY);
        }

        for(int j=1; j<20; j++){                                                    //vertical lines    
            DrawLine(j*CELL_SIZE, 0, j*CELL_SIZE, screenHeight, DARKGRAY);
        }

        for( const Cell& c: snake ){                                                //draw snake
            DrawRectangle(1 + c.x * CELL_SIZE, 1 + c.y * CELL_SIZE, CELL_SIZE-2, CELL_SIZE-2, GREEN);
        }

        DrawRectangle(1 + food.x * CELL_SIZE, 1 + food.y * CELL_SIZE, CELL_SIZE-2, CELL_SIZE-2, RED);

        if ( state == GameState::GameOver ){
            const char* gameover_text = "GAME OVER";
            const char* restart_text = "Press R to restart";
            DrawText(gameover_text, screenWidth / 2 - MeasureText(gameover_text, 40) / 2, 200, 40, RED);
            DrawText(restart_text, screenWidth / 2  - MeasureText(restart_text, 30) / 2, 300, 30, BLUE);
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}