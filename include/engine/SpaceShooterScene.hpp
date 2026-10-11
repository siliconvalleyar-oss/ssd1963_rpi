#pragma once
#include <engine/Scene.hpp>
#include <engine/Sprite.hpp>
#include <FrameBuffer.hpp>

#define MAX_BULLETS  8
#define MAX_ENEMIES   4
#define MAX_STARS    30

class SpaceShooterScene : public Scene {
public:
    explicit SpaceShooterScene(SSD1963& display);

    bool on_enter() override;
    void update(uint32_t dt) override;
    void draw(FrameBuffer& fb) override;
    const char* name() const override;

private:
    struct Bullet { float x, y, px, py; bool active; };
    struct Enemy  { float x, y, px, py; bool active; uint8_t hp; uint32_t timer; };
    struct Star   { uint16_t x; float y; uint8_t speed; uint8_t brightness; };

    float   m_player_x, m_player_y;
    float   m_prev_px, m_prev_py;
    int8_t  m_player_dir;
    uint8_t m_lives;
    uint32_t m_score;

    Bullet   m_bullets[MAX_BULLETS];
    Enemy    m_enemies[MAX_ENEMIES];
    Star     m_stars[MAX_STARS];

    uint32_t m_timer;
    uint32_t m_shoot_timer;
    uint32_t m_spawn_timer;
    uint32_t m_move_timer;

    Sprite   m_spr_player;
    Sprite   m_spr_enemy;
    Sprite   m_spr_bullet;
    Sprite   m_spr_planet[3];

    bool     m_sprites_loaded;
    bool     m_drawn_once;

    static constexpr uint16_t BG       = 0x00A3;
    static constexpr uint16_t PLAYER_W = 32;
    static constexpr uint16_t PLAYER_H = 32;
    static constexpr uint16_t BULLET_W = 8;
    static constexpr uint16_t BULLET_H = 16;
    static constexpr uint16_t ENEMY_W  = 32;
    static constexpr uint16_t ENEMY_H  = 32;

    static constexpr float PLAYER_SPEED   = 1.5f;
    static constexpr float BULLET_SPEED   = 4.0f;
    static constexpr float ENEMY_SPEED    = 1.0f;
    static constexpr uint32_t SHOOT_INTERVAL = 500;
    static constexpr uint32_t SPAWN_INTERVAL = 2000;
    static constexpr uint32_t AUTO_RETURN_MS  = 60000;

    void spawn_enemy();
    void shoot();
    void erase(FrameBuffer& fb, float x, float y, uint16_t w, uint16_t h);
    void update_bullets(uint32_t dt);
    void update_enemies(uint32_t dt);
    void check_collisions();
    void draw_hud(FrameBuffer& fb);
    void draw_stars(FrameBuffer& fb);
    void draw_planets();
};
