#pragma once
#include "lvgl.h"
#include "stdio.h"
#include <vector>

// Grid Configuration
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 480
#define GRID_SIZE 40
#define GRID_COLS (SCREEN_WIDTH / GRID_SIZE)
#define GRID_ROWS (SCREEN_HEIGHT / GRID_SIZE)
#define GET_ACTIVE_SCREEN() lv_scr_act()

enum class Direction { UP, DOWN, LEFT, RIGHT };
enum class ObjectType { FOOD, WALL, OBSTACLE };

struct Point
{
    int x;
    int y;
    bool operator==(const Point& other) const
    {
        return x == other.x && y == other.y;
    }
};

struct SnakeSegment
{
    Point pos;
    lv_obj_t* visual;
};

struct GameObject
{
    Point pos;
    ObjectType type;
    lv_obj_t* visual;
};

class SnakeGame
{
    public:
        SnakeGame();
        ~SnakeGame();

        void init(lv_obj_t* parent_screen);
        void step();
        void start_game();
        void set_direction(Direction new_dir);
        void add_obstacle(int grid_x, int grid_y, ObjectType type);
        bool is_running() const { return is_game_started && !is_game_over; }

    private:
        lv_obj_t* screen;
        lv_timer_t* game_timer;

        lv_obj_t* score_label;
        lv_obj_t* prompt_label;
        
        std::vector<SnakeSegment> snake;
        std::vector<GameObject> game_objects;
        
        Direction current_dir;
        Direction next_dir;
        bool is_game_over;
        bool is_game_started;

        int current_score;
        int last_score;

        lv_obj_t* create_cell_visual(Point p, lv_color_t color);
        void spawn_food();
        bool check_collision(const Point& projected_head, ObjectType& hit_type, int& hit_object_index);
        void handle_game_over();
        void update_score_display();
        void reset();
};

void start_snake_game();