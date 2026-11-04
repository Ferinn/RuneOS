#include "snake.hpp"
#include <cstdlib>
#include <cstdio>

static SnakeGame* g_game_instance = nullptr;

static void game_timer_cb(lv_timer_t* timer)
{
    if (g_game_instance)
    {
        g_game_instance->step();
    }
}

static void touch_input_cb(lv_event_t* e)
{
    if (!g_game_instance) return;

    if (!g_game_instance->is_running())
    {
        g_game_instance->start_game();
        return;
    }

    lv_indev_t* indev = lv_event_get_indev(e);
    if (!indev) return;

    lv_point_t touch_pt;
    lv_indev_get_point(indev, &touch_pt);

    int center_x = SCREEN_WIDTH / 2;
    int center_y = SCREEN_HEIGHT / 2;
    int dx = touch_pt.x - center_x;
    int dy = touch_pt.y - center_y;

    if (abs(dx) > abs(dy))
    {
        if (dx > 0) g_game_instance->set_direction(Direction::RIGHT);
        else g_game_instance->set_direction(Direction::LEFT);
    }
    else
    {
        if (dy > 0) g_game_instance->set_direction(Direction::DOWN);
        else g_game_instance->set_direction(Direction::UP);
    }
}

SnakeGame::SnakeGame()
{
    screen = nullptr;
    game_timer = nullptr;
    score_label = nullptr;
    prompt_label = nullptr;
    current_dir = Direction::RIGHT;
    next_dir = Direction::RIGHT;
    is_game_over = false;
    is_game_started = false;
    current_score = 0;
    last_score = 0;
}

SnakeGame::~SnakeGame()
{
    reset();
    if (game_timer)
    {
        lv_timer_delete(game_timer);
    }
}

lv_obj_t* SnakeGame::create_cell_visual(Point p, lv_color_t color)
{
    lv_obj_t* cell = lv_obj_create(screen);
    lv_obj_set_size(cell, GRID_SIZE, GRID_SIZE);
    
    lv_obj_set_pos(cell, p.x * GRID_SIZE, p.y * GRID_SIZE);

    lv_obj_set_style_bg_color(cell, color, 0);
    lv_obj_set_style_bg_opa(cell, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(cell, 0, 0);
    lv_obj_set_style_radius(cell, 0, 0);
    lv_obj_remove_flag(cell, LV_OBJ_FLAG_SCROLLABLE);

    return cell;
}

void SnakeGame::init(lv_obj_t* parent_screen)
{
    screen = parent_screen;
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    lv_obj_add_event_cb(screen, touch_input_cb, LV_EVENT_PRESSED, NULL);

    // Score
    score_label = lv_label_create(screen);
    lv_obj_align(score_label, LV_ALIGN_TOP_LEFT, 20, 20);
    lv_obj_set_style_text_color(score_label, lv_color_hex(0xFFFFFF), 0);

    // Start / Game Over
    prompt_label = lv_label_create(screen);
    lv_obj_align(prompt_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(prompt_label, lv_color_hex(0xFFFF00), 0);
    lv_obj_set_style_text_align(prompt_label, LV_TEXT_ALIGN_CENTER, 0);

    reset();

    is_game_started = false;
    is_game_over = false;

    // Show Start
    char prompt_text[128];
    snprintf(prompt_text, sizeof(prompt_text), "SNAKE GAME\n\nTap screen to play!");
    lv_label_set_text(prompt_label, prompt_text);

    game_timer = lv_timer_create(game_timer_cb, 120, NULL);
}

void SnakeGame::start_game()
{
    reset();
    is_game_started = true;
    is_game_over = false;

    lv_obj_add_flag(prompt_label, LV_OBJ_FLAG_HIDDEN);
}

void SnakeGame::reset() {
    for (auto& seg : snake)
    {
        if (seg.visual) lv_obj_delete(seg.visual);
    }
    snake.clear();

    for (auto& obj : game_objects)
    {
        if (obj.visual) lv_obj_delete(obj.visual);
    }
    game_objects.clear();

    current_dir = Direction::RIGHT;
    next_dir = Direction::RIGHT;
    is_game_over = false;

    Point head_pos = {5, 5};
    Point body_pos = {4, 5};
    Point tail_pos = {3, 5};

    snake.push_back({head_pos, create_cell_visual(head_pos, lv_color_hex(0x00FF00))});
    snake.push_back({body_pos, create_cell_visual(body_pos, lv_color_hex(0x00AA00))});
    snake.push_back({tail_pos, create_cell_visual(tail_pos, lv_color_hex(0x00AA00))});

    add_obstacle(15, 8, ObjectType::WALL);
    add_obstacle(15, 9, ObjectType::WALL);
    add_obstacle(15, 10, ObjectType::WALL);

    spawn_food();
}

void SnakeGame::update_score_display()
{
    if (score_label)
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "Score: %d", current_score);
        lv_label_set_text(score_label, buf);
    }
}

void SnakeGame::set_direction(Direction new_dir) {
    if (current_dir == Direction::UP && new_dir == Direction::DOWN) return;
    if (current_dir == Direction::DOWN && new_dir == Direction::UP) return;
    if (current_dir == Direction::LEFT && new_dir == Direction::RIGHT) return;
    if (current_dir == Direction::RIGHT && new_dir == Direction::LEFT) return;

    next_dir = new_dir;
}

void SnakeGame::add_obstacle(int grid_x, int grid_y, ObjectType type)
{
    Point p = {grid_x, grid_y};
    lv_color_t color = (type == ObjectType::WALL) ? lv_color_hex(0x808080) : lv_color_hex(0xFF8C00);
    lv_obj_t* visual = create_cell_visual(p, color);
    
    game_objects.push_back({p, type, visual});
}

void SnakeGame::spawn_food()
{
    Point food_pos;
    bool valid_position = false;

    while (!valid_position)
    {
        food_pos.x = rand() % GRID_COLS;
        food_pos.y = rand() % GRID_ROWS;

        valid_position = true;

        for (const auto& seg : snake)
        {
            if (seg.pos == food_pos)
            {
                valid_position = false;
                break;
            }
        }

        if (valid_position)
        {
            for (const auto& obj : game_objects)
            {
                if (obj.pos == food_pos)
                {
                    valid_position = false;
                    break;
                }
            }
        }
    }

    lv_obj_t* visual = create_cell_visual(food_pos, lv_color_hex(0xFF0000));
    game_objects.push_back({food_pos, ObjectType::FOOD, visual});
}

bool SnakeGame::check_collision(const Point& projected_head, ObjectType& hit_type, int& hit_object_index)
{
    if (projected_head.x < 0 || projected_head.x >= GRID_COLS ||
        projected_head.y < 0 || projected_head.y >= GRID_ROWS)
    {
        return true;
    }

    size_t check_length = snake.size() - 1; 
    for (size_t i = 0; i < check_length; i++)
    {
        if (snake[i].pos == projected_head)
        {
            return true;
        }
    }

    for (size_t i = 0; i < game_objects.size(); i++)
    {
        if (game_objects[i].pos == projected_head)
        {
            hit_type = game_objects[i].type;
            hit_object_index = static_cast<int>(i);
            return false;
        }
    }

    return false;
}

void SnakeGame::step()
{
    if (!is_running()) return;

    current_dir = next_dir;

    Point current_head = snake[0].pos;
    Point projected_head = current_head;

    switch (current_dir)
    {
        case Direction::UP:    projected_head.y -= 1; break;
        case Direction::DOWN:  projected_head.y += 1; break;
        case Direction::LEFT:  projected_head.x -= 1; break;
        case Direction::RIGHT: projected_head.x += 1; break;
    }

    ObjectType hit_type;
    int hit_object_idx = -1;

    bool is_fatal_collision = check_collision(projected_head, hit_type, hit_object_idx);

    if (is_fatal_collision || (hit_object_idx != -1 && hit_type == ObjectType::WALL))
    {
        handle_game_over();
        return;
    }

    bool ate_food = (hit_object_idx != -1 && hit_type == ObjectType::FOOD);

    if (ate_food)
    {
        lv_obj_delete(game_objects[hit_object_idx].visual);
        game_objects.erase(game_objects.begin() + hit_object_idx);

        current_score += 10;
        update_score_display();

        spawn_food();
    }
    else
    {
        SnakeSegment tail = snake.back();
        lv_obj_delete(tail.visual);
        snake.pop_back();
    }

    lv_obj_t* new_head_visual = create_cell_visual(projected_head, lv_color_hex(0x00FF00));
    
    lv_obj_set_style_bg_color(snake[0].visual, lv_color_hex(0x00AA00), 0);

    snake.insert(snake.begin(), {projected_head, new_head_visual});
}

void SnakeGame::handle_game_over()
{
    is_game_over = true;
    is_game_started = false;
    last_score = current_score;
    current_score = 0;
    update_score_display();

    char msg[128];
    snprintf(msg, sizeof(msg), "GAME OVER!\nScore: %d\n\nTap screen to play again", last_score);
    
    lv_label_set_text(prompt_label, msg);
    lv_obj_clear_flag(prompt_label, LV_OBJ_FLAG_HIDDEN);
}

void start_snake_game()
{
    if (!g_game_instance)
    {
        g_game_instance = new SnakeGame();
        g_game_instance->init(GET_ACTIVE_SCREEN());
    }
}