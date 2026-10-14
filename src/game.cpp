/**
 * Mario-style platformer game implementation
 * Uses existing SSD1963 class API directly
 */
#include "game.hpp"
#include <cstdlib>
#include <cmath>
#include <cstdint>

Game::Game(SSD1963& display) : display_(display) {}

void Game::init() {
    running_ = true;
    score_ = 0;
    lives_ = 3;
    camera_x_ = 0;
    setup_level();
    spawn_enemies();
}

void Game::setup_level() {
    player_.x = 50;
    player_.y = 200;
    player_.vx = 0;
    player_.vy = 0;
    player_.on_ground = false;
    player_.facing_right = true;
    player_.frame = 0;
    player_.anim_timer = 0;
    
    // Create platforms
    platforms_.clear();
    
    // Ground platform
    platforms_.push_back({0, 240, 480, 32});
    
    // Floating platforms
    platforms_.push_back({100, 200, 80, 16});
    platforms_.push_back({220, 160, 80, 16});
    platforms_.push_back({350, 120, 80, 16});
    platforms_.push_back({200, 80, 60, 16});
    platforms_.push_back({350, 40, 60, 16});
}

void Game::spawn_enemies() {
    enemies_.clear();
    enemies_.push_back({150, 220, 1, 16, 16, true, true});
    enemies_.push_back({300, 140, -1, 16, 16, true, false});
    enemies_.push_back({400, 20, 1, 16, 16, true, true});
}

void Game::update() {
    handle_input();
    update_player();
    update_enemies();
    check_collisions();
    check_platform_collision();
    check_enemy_collision();
    
    // Update camera
    camera_x_ = player_.x - 240;
    if (camera_x_ < 0) camera_x_ = 0;
    if (camera_x_ > 480 - 480) camera_x_ = 0;
    
    render();
}

void Game::handle_input() {
    // Input handling would be added here with GPIO
}

void Game::handle_movement() {
    // Movement handled by GPIO input
}

void Game::handle_jump() {
    if (player_.on_ground) {
        player_.vy = -18; // JUMP_FORCE
        player_.on_ground = false;
    }
}

void Game::apply_gravity() {
    if (!player_.on_ground) {
        player_.vy += 2; // GRAVITY
        if (player_.vy > 15) player_.vy = 15; // MAX_FALL_SPEED
    }
}

void Game::update_player() {
    apply_gravity();
    
    player_.x += player_.vx;
    player_.y += player_.vy;
    
    // Animation
    if (player_.vx != 0) {
        player_.anim_timer++;
        if (player_.anim_timer > 8) {
            player_.frame = (player_.frame + 1) % 4;
            player_.anim_timer = 0;
        }
    } else {
        player_.frame = 0;
        player_.anim_timer = 0;
    }
    
    // Screen boundaries
    if (player_.x < 0) player_.x = 0;
    if (player_.x > 480 - 16) player_.x = 480 - 16;
    
    // Ground collision
    if (player_.y >= 240 - 24) {
        player_.y = 240 - 24;
        player_.vy = 0;
        player_.on_ground = true;
    }
}

void Game::update_enemies() {
    for (auto& enemy : enemies_) {
        if (!enemy.active) continue;
        
        if (enemy.moving_right) {
            enemy.x += 1;
            if (enemy.x > 450) enemy.moving_right = false;
        } else {
            enemy.x -= 1;
            if (enemy.x < 50) enemy.moving_right = true;
        }
    }
}

void Game::check_platform_collision() {
    // Ground collision handled in update_player
}

void Game::check_enemy_collision() {
    for (auto& enemy : enemies_) {
        if (!enemy.active) continue;
        
        int16_t dx = player_.x - enemy.x;
        int16_t dy = player_.y - enemy.y;
        
        if (abs(dx) < 16 && abs(dy) < 24) {
            if (player_.vy > 0 && player_.y < enemy.y) {
                enemy.active = false;
                score_ += 100;
                player_.vy = -12;
            } else {
                lives_--;
                if (lives_ == 0) {
                    running_ = false;
                } else {
                    player_.x = 50;
                    player_.y = 200;
                    player_.vx = 0;
                    player_.vy = 0;
                }
            }
        }
    }
}

void Game::check_collisions() {
    check_enemy_collision();
}

void Game::render() {
    display_.clear_screen(0x001F); // Sky blue
    
    draw_background();
    draw_platforms();
    draw_enemies();
    draw_player();
    draw_hud();
}

void Game::draw_background() {
    for (int y = 0; y < 272; y++) {
        uint16_t color = 0x001F + (y * 0x10);
        display_.draw_block(0, y, 480, 1, color);
    }
}

void Game::draw_platforms() {
    for (const auto& platform : platforms_) {
        display_.draw_block(platform.x - camera_x_, platform.y, 
                           platform.width, platform.height, 0x5AEB);
    }
}

void Game::draw_player() {
    int16_t px = player_.x - camera_x_;
    int16_t py = player_.y;
    display_.draw_block(px, py, 16, 16, 0xF800);
    display_.draw_block(px + 2, py - 8, 12, 8, 0xFD20);
}

void Game::draw_enemies() {
    for (const auto& enemy : enemies_) {
        if (!enemy.active) continue;
        int16_t ex = enemy.x - camera_x_;
        int16_t ey = enemy.y;
        display_.draw_block(ex, ey, 16, 16, 0xF800);
    }
}

void Game::draw_hud() {
    for (int i = 0; i < lives_; i++) {
        display_.draw_block(10 + i * 20, 10, 16, 16, 0xF800);
    }
}

void Game::reset_game() {
    score_ = 0;
    lives_ = 3;
    camera_x_ = 0;
    setup_level();
    spawn_enemies();
}

void Game::draw_game_over() {
}

void Game::update() {
    handle_input();
    update_player();
    update_enemies();
    check_collisions();
    check_platform_collision();
    check_enemy_collision();
    
    camera_x_ = player_.x - 240;
    if (camera_x_ < 0) camera_x_ = 0;
    if (camera_x_ > 480 - 480) camera_x_ = 0;
    
    render();
}

void Game::handle_input() {
}

void Game::handle_movement() {
}

void Game::handle_jump() {
    if (player_.on_ground) {
        player_.vy = -18;
        player_.on_ground = false;
    }
}

void Game::apply_gravity() {
    if (!player_.on_ground) {
        player_.vy += 2;
        if (player_.vy > 15) player_.vy = 15;
    }
}

void Game::update_player() {
    apply_gravity();
    player_.x += player_.vx;
    player_.y += player_.vy;
    
    if (player_.vx != 0) {
        player_.anim_timer++;
        if (player_.anim_timer > 8) {
            player_.frame = (player_.frame + 1) % 4;
            player_.anim_timer = 0;
        }
    } else {
        player_.frame = 0;
        player_.anim_timer = 0;
    }
    
    if (player_.x < 0) player_.x = 0;
    if (player_.x > 480 - 16) player_.x = 480 - 16;
    
    if (player_.y >= 240 - 24) {
        player_.y = 240 - 24;
        player_.vy = 0;
        player_.on_ground = true;
    }
}

void Game::update_enemies() {
    for (auto& enemy : enemies_) {
        if (!enemy.active) continue;
        
        if (enemy.moving_right) {
            enemy.x += 1;
            if (enemy.x > 450) enemy.moving_right = false;
        } else {
            enemy.x -= 1;
            if (enemy.x < 50) enemy.moving_right = true;
        }
    }
}

void Game::check_platform_collision() {
}

void Game::check_enemy_collision() {
    for (auto& enemy : enemies_) {
        if (!enemy.active) continue;
        
        int16_t dx = player_.x - enemy.x;
        int16_t dy = player_.y - enemy.y;
        
        if (abs(dx) < 16 && abs(dy) < 24) {
            if (player_.vy > 0 && player_.y < enemy.y) {
                enemy.active = false;
                score_ += 100;
                player_.vy = -12;
            } else {
                lives_--;
                if (lives_ == 0) {
                    running_ = false;
                } else {
                    player_.x = 50;
                    player_.y = 200;
                    player_.vx = 0;
                    player_.vy = 0;
                }
            }
        }
    }
}

void Game::check_collisions() {
    check_enemy_collision();
}

void Game::render() {
    display_.clear_screen(0x001F);
    draw_background();
    draw_platforms();
    draw_enemies();
    draw_player();
    draw_hud();
}

void Game::draw_background() {
    for (int y = 0; y < 272; y++) {
        uint16_t color = 0x001F + (y * 0x10);
        display_.draw_block(0, y, 480, 1, color);
    }
}

void Game::draw_platforms() {
    for (const auto& platform : platforms_) {
        display_.draw_block(platform.x - camera_x_, platform.y, 
                           platform.width, platform.height, 0x5AEB);
    }
}

void Game::draw_player() {
    int16_t px = player_.x - camera_x_;
    int16_t py = player_.y;
    display_.draw_block(px, py, 16, 16, 0xF800);
    display_.draw_block(px + 2, py - 8, 12, 8, 0xFD20);
}

void Game::draw_enemies() {
    for (const auto& enemy : enemies_) {
        if (!enemy.active) continue;
        int16_t ex = enemy.x - camera_x_;
        int16_t ey = enemy.y;
        display_.draw_block(ex, ey, 16, 16, 0xF800);
    }
}

void Game::draw_hud() {
    for (int i = 0; i < lives_; i++) {
        display_.draw_block(10 + i * 20, 10, 16, 16, 0xF800);
    }
}

void Game::reset_game() {
    score_ = 0;
    lives_ = 3;
    camera_x_ = 0;
    setup_level();
    spawn_enemies();
}

void Game::draw_game_over() {
}