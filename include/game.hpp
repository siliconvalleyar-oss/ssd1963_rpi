/**
 * Mario-style platformer game for SSD1963
 * Simple side-scrolling platformer with player, platforms, and enemies
 */
#pragma once

#include "ssd1963.hpp"
#include <vector>
#include <cstdint>

class Game {
public:
    explicit Game(SSD1963& display);
    
    void init();
    void update();
    void render();
    void handle_input();
    
    bool is_running() const { return running_; }
    
private:
    SSD1963& display_;
    bool running_ = true;
    uint32_t last_update_ = 0;
    
    // Player
    struct Player {
        int16_t x = 50;
        int16_t y = 200;
        int16_t vx = 0;
        int16_t vy = 0;
        bool on_ground = false;
        bool facing_right = true;
        uint8_t frame = 0;
        uint32_t anim_timer = 0;
    } player_;
    
    // Platform
    struct Platform {
        int16_t x, y;
        uint16_t width;
        uint16_t height;
    };
    
    // Enemy
    struct Enemy {
        int16_t x, y;
        int16_t vx;
        uint16_t width = 16;
        uint16_t height = 16;
        bool active = true;
        bool moving_right = true;
    };
    
    std::vector<Platform> platforms_;
    std::vector<Enemy> enemies_;
    
    uint32_t score_ = 0;
    uint8_t lives_ = 3;
    
    // Camera
    int16_t camera_x_ = 0;
    
    void setup_level();
    void update_player();
    void update_enemies();
    void check_collisions();
    void apply_gravity();
    void handle_jump();
    void handle_movement();
    void check_platform_collision();
    void check_enemy_collision();
    void draw_background();
    void draw_player();
    void draw_enemies();
    void draw_platforms();
    void draw_hud();
    void draw_game_over();
    void reset_game();
    void spawn_enemies();
    
    // Constants
    static constexpr int16_t GRAVITY = 2;
    static constexpr int16_t JUMP_FORCE = -18;
    static constexpr int16_t MAX_FALL_SPEED = 15;
    static constexpr int16_t MOVE_SPEED = 4;
    static constexpr uint16_t SCREEN_WIDTH = 480;
    static constexpr uint16_t SCREEN_HEIGHT = 272;
    static constexpr uint16_t PLAYER_WIDTH = 16;
    static constexpr uint16_t PLAYER_HEIGHT = 24;
    static constexpr uint16_t GROUND_Y = 240;
};