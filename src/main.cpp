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

enum class GameState {Playing, GameOver};

struct Game {
    std::deque<Cell> snake;
    Cell direction = {1, 0};
    Cell nextDirection = {1, 0};
    float moveTimer = 0.0f;
    Cell food = {0, 0};
    GameState state = GameState::Playing;
    int score = 0;
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

void ResetGame(Game& game) {
    game.snake = { {10, 10}, {9, 10}, {8, 10} };             //head is first(cell co-ords not pixels)
    game.direction = {1, 0};                                 //1 cell right per move
    game.nextDirection = game.direction;
    game.moveTimer = 0.0f;                                   //time since last move
    game.food = spawnFood(game.snake);
    game.state = GameState::Playing;
    game.score = 0;
}

void HandleInput(Game& game){
    //game reset
    if (game.state == GameState::GameOver && IsKeyPressed(KEY_R)){
        ResetGame(game);
    }

    //inputs
    if (IsKeyPressed(KEY_UP) && game.direction.y != 1) {
        game.nextDirection = {0, -1};
    }
    if (IsKeyPressed(KEY_DOWN) && game.direction.y != -1) {
        game.nextDirection = {0, 1};
    }
    if (IsKeyPressed(KEY_LEFT) && game.direction.x != 1) {
        game.nextDirection = {-1, 0};
    }
    if (IsKeyPressed(KEY_RIGHT) && game.direction.x != -1) {
        game.nextDirection = {1, 0};
    }
}

void Step(Game& game){
    //snake movement
    game.direction = game.nextDirection;
    Cell new_head = {game.snake.front().x + game.direction.x, game.snake.front().y + game.direction.y};
            
    //dead condition
    bool dead = false;
    if (new_head.x < 0 || new_head.x >= GRID_COLS || new_head.y < 0 || new_head.y >= GRID_ROWS) dead = true;
    for (const Cell& c: game.snake){
        if (new_head.x == c.x && new_head.y == c.y) dead = true;
    }
    if (dead){
        game.state = GameState::GameOver;
    } else{
        game.snake.push_front(new_head);
        if (new_head.x == game.food.x && new_head.y == game.food.y) {
            game.food = spawnFood(game.snake);
            game.score++;
        } else {
            game.snake.pop_back();
        }
    }
}

void Draw(const Game& game){
    ClearBackground(BLACK);
    for(int i=1; i<20; i++){                                            //horizontal lines
        DrawLine(0, i*CELL_SIZE, CELL_SIZE * GRID_COLS, i*CELL_SIZE, DARKGRAY);
    }

    for(int j=1; j<20; j++){                                            //vertical lines    
        DrawLine(j*CELL_SIZE, 0, j*CELL_SIZE, CELL_SIZE * GRID_ROWS, DARKGRAY);
    }

    for( const Cell& c: game.snake ){                                   //draw snake
        DrawRectangle(1 + c.x * CELL_SIZE, 1 + c.y * CELL_SIZE, CELL_SIZE-2, CELL_SIZE-2, GREEN);
    }

    DrawRectangle(1 + game.food.x * CELL_SIZE, 1 + game.food.y * CELL_SIZE, CELL_SIZE-2, CELL_SIZE-2, RED);

    DrawText(TextFormat("Score: %d", game.score), 10, 10, 20, WHITE);
    if ( game.state == GameState::GameOver ){
        const char* gameover_text = "GAME OVER";
        const char* restart_text = "Press R to restart";
        DrawText(gameover_text, (CELL_SIZE * GRID_COLS) / 2 - MeasureText(gameover_text, 40) / 2, 200, 40, RED);
        DrawText(restart_text, (CELL_SIZE * GRID_COLS) / 2  - MeasureText(restart_text, 30) / 2, 300, 30, BLUE);
    }
}


int main(){
    int screenWidth = 600;
    int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Snake");
    SetTargetFPS(60);

    Game game;
    ResetGame(game);                                 
    const float MOVE_INTERVAL = 0.15f;            //move every 0.15s

    while(!WindowShouldClose()){
        HandleInput(game);

        game.moveTimer += GetFrameTime();        //seconds since last frame
        if (game.moveTimer >= MOVE_INTERVAL && game.state == GameState::Playing) {
            game.moveTimer -= MOVE_INTERVAL;
            Step(game);
        }

        BeginDrawing();
        Draw(game);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}