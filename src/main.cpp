#include "raylib.h"
#include <deque>
#include <queue>
#include <vector>
#include <fstream>
#include <iostream>

const int CELL_SIZE = 30;       //each cell is 30x30
const int GRID_COLS = 20;       //no.of cells horizontally
const int GRID_ROWS = 20;       //no.of cells vertically

struct Cell{
    int x;
    int y;
};

enum class GameState {Playing, GameOver, GameWin};
const Cell DIRS[4] = { {1, 0}, {0, 1}, {-1, 0}, {0, -1} };

struct Game {
    std::deque<Cell> snake;
    Cell direction = {1, 0};
    Cell nextDirection = {1, 0};
    float moveTimer = 0.0f;
    Cell food = {0, 0};
    GameState state = GameState::Playing;
    int score = 0;
    bool aiEnabled = false;
};


Cell spawnFood(Game& game){
    Cell food;
    bool onSnake = false;

    do {
        if(game.snake.size() == GRID_COLS*GRID_ROWS){
            game.state = GameState::GameWin;
            break;
        }
        onSnake = false;

        food.x = GetRandomValue(0, GRID_COLS-1);
        food.y = GetRandomValue(0, GRID_ROWS-1);

        for ( const Cell& c: game.snake ){
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
    game.food = spawnFood(game);
    game.state = GameState::Playing;
    game.score = 0;
    // game.aiEnabled = false;
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
        std::ofstream outFile("scores.txt", std::ios::app);
        if (outFile.is_open()) {
            outFile << game.score << "\n";
            outFile.close();
        } else {
            std::cerr << "Error opening file";
        }
        game.state = GameState::GameOver;
    } else{
        game.snake.push_front(new_head);
        if (new_head.x == game.food.x && new_head.y == game.food.y) {
            game.food = spawnFood(game);
            game.score++;
        } else {
            game.snake.pop_back();
        }
    }
}

void Draw(const Game& game){
    ClearBackground(BLACK);
    for(int i=1; i<GRID_ROWS; i++){                                            //horizontal lines
        DrawLine(0, i*CELL_SIZE, CELL_SIZE * GRID_COLS, i*CELL_SIZE, DARKGRAY);
    }

    for(int j=1; j<GRID_COLS; j++){                                            //vertical lines    
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

    if ( game.state == GameState::GameWin ){
        const char* gamewin_text = "YOU WON!";
        DrawText(gamewin_text, (CELL_SIZE * GRID_COLS) / 2 - MeasureText(gamewin_text, 50) / 2, 300, 50, GREEN);
    }

    if (game.aiEnabled){
        const char* ai_on_text = "AI Mode: ON";
        DrawText(ai_on_text, (CELL_SIZE * GRID_COLS) - MeasureText(ai_on_text, 20)-10, 10, 20, WHITE);
    } else{
        const char* ai_off_text = "AI Mode: OFF";
        DrawText(ai_off_text, (CELL_SIZE * GRID_COLS) - MeasureText(ai_off_text, 20)-10, 10, 20, WHITE);
    }
}

//Counts how many free cells can be reached from start without crossing the snake
int CountReachable(const std::vector<std::vector<bool>>& blocked, Cell start){
    std::vector<std::vector<bool>> visited(GRID_ROWS, std::vector<bool>(GRID_COLS, false));
    std::queue<Cell> check_cells;

    if(blocked[start.y][start.x]) return 0;
    if(start.x < 0 || start.x >= GRID_COLS || start.y < 0 || start.y >= GRID_ROWS) return 0;

    check_cells.push(start);
    visited[start.y][start.x] = true;
    int count = 0;
    while(!check_cells.empty()){
        Cell current = check_cells.front();
        check_cells.pop();
        count++;

        for(const Cell& d: DIRS){
            Cell next = {d.x+current.x, d.y+current.y}; 
            if(next.x < 0 || next.x >= GRID_COLS || next.y < 0 || next.y >= GRID_ROWS){
                continue;
            }
            if(blocked[next.y][next.x]) continue;
            if(visited[next.y][next.x]) continue;

            visited[next.y][next.x] = true;
            check_cells.push(next);
        }
    }
    return count;
}

//Looks at the game and decides which direction to move next
Cell ChooseDirection(const Game& game){

    Cell head = game.snake.front();
    //blocked[y][x] is true if snake segment occupies that cell
    std::vector<std::vector<bool>> blocked(GRID_ROWS, std::vector<bool>(GRID_COLS, false));

    for (const Cell& c: game.snake) {
        blocked[c.y][c.x] = true;
    }

    //parent[y][x] = the cell we came from to reach (x,y)
    //{-1, -1} means unvisited
    std::vector<std::vector<Cell>> parent(GRID_ROWS, std::vector<Cell>(GRID_COLS, Cell{-1, -1}));

    std::queue<Cell> frontier;
    frontier.push(head);
    parent[head.y][head.x] = head;      //the start counts as visited

    bool found = false;
    while(!frontier.empty() && !found){
        Cell current = frontier.front();
        frontier.pop();

        for (const Cell& d: DIRS) {
            Cell next = {current.x+d.x, current.y+d.y};
            
            if(next.x < 0 || next.x >= GRID_COLS || next.y < 0 || next.y >= GRID_ROWS){
                //if next cell is out of the grid
                continue;
            }

            if(blocked[next.y][next.x]) continue; //if next cell lies on snake's body/blocked

            if(parent[next.y][next.x].x != -1) continue;    //already visited

            parent[next.y][next.x] = current;
            frontier.push(next);

            //if food is found then break
            if(next.x == game.food.x && next.y == game.food.y){
                found = true;
                break;
            }
        }
    }

    bool is_risky = false;
    Cell bestDir = game.direction;
    if(found){
        Cell step = game.food;
        while(parent[step.y][step.x].x != head.x || parent[step.y][step.x].y != head.y){
            step = parent[step.y][step.x];
        }
        Cell bestDir = Cell{step.x - head.x, step.y - head.y};

        if (CountReachable(blocked, {head.x+bestDir.x, head.y+bestDir.y}) < game.snake.size()){
            is_risky = true;
        }
        if(!is_risky) return bestDir;
    }
    int bestCount = -1;
    for(const Cell& d: DIRS){
        Cell next = {head.x+d.x, head.y+d.y};
        if(next.x < 0 || next.x >= GRID_COLS || next.y < 0 || next.y >= GRID_ROWS) continue;
        if(blocked[next.y][next.x]) continue;   
        int space = CountReachable(blocked, next);
        if(bestCount < space){
            bestCount = space;
            bestDir = d;
        }
    }
    return bestDir;
}


int main(){
    int screenWidth = GRID_COLS * CELL_SIZE;
    int screenHeight = GRID_ROWS * CELL_SIZE;

    InitWindow(screenWidth, screenHeight, "Snake");
    SetTargetFPS(60);

    Game game;
    ResetGame(game);                                 
    const float MOVE_INTERVAL = 0.07f;            //move every 0.15s

    

    while(!WindowShouldClose()){
        HandleInput(game);

        if(IsKeyPressed(KEY_A)) game.aiEnabled = !game.aiEnabled;

        game.moveTimer += GetFrameTime();        //seconds since last frame
        if (game.moveTimer >= MOVE_INTERVAL && game.state == GameState::Playing) {
            game.moveTimer -= MOVE_INTERVAL;
            if (game.aiEnabled){
                game.nextDirection = ChooseDirection(game);
            }
            Step(game);
        }

        BeginDrawing();
        Draw(game);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}