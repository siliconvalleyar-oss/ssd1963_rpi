#pragma once
#include <engine/Scene.hpp>
#include <engine/Sprite.hpp>

#define MAX_BULLETS  12
#define MAX_ENEMIES   8
#define MAX_STARS    40

class SpaceShooterScene : public Scene {
public:
    explicit SpaceShooterScene(SSD1963& display);

    bool on_enter() override;
    void update(uint32_t dt) override;
    void draw() override;
    const char* name() const override;

private:
    // Entidades del juego
    struct Bullet { float x, y; bool active; };
    struct Enemy  { float x, y; bool active; uint8_t hp; uint32_t timer; };
    struct Star   { uint16_t x, y; uint8_t speed; uint8_t brightness; };

    // Jugador
    float   m_player_x, m_player_y;
    int8_t  m_player_dir;      // -1 izq, 0 quieto, +1 der
    uint8_t m_lives;
    uint32_t m_score;

    // Arrays de juego
    Bullet   m_bullets[MAX_BULLETS];
    Enemy    m_enemies[MAX_ENEMIES];
    Star     m_stars[MAX_STARS];

    // Temporizadores
    uint32_t m_timer;
    uint32_t m_shoot_timer;
    uint32_t m_spawn_timer;
    uint32_t m_move_timer;

    // Sprites
    Sprite   m_spr_player;
    Sprite   m_spr_enemy;
    Sprite   m_spr_bullet;
    Sprite   m_spr_planet[3];

    bool     m_sprites_loaded;
    bool     m_planet_drawn;

    static constexpr uint16_t PLAYER_W  = 32;
    static constexpr uint16_t PLAYER_H  = 32;
    static constexpr uint16_t BULLET_W  = 8;
    static constexpr uint16_t BULLET_H  = 16;
    static constexpr uint16_t ENEMY_W   = 32;
    static constexpr uint16_t ENEMY_H   = 32;

    static constexpr float PLAYER_SPEED   = 1.5f;
    static constexpr float BULLET_SPEED   = 4.0f;
    static constexpr float ENEMY_SPEED    = 1.0f;
    static constexpr uint32_t SHOOT_INTERVAL = 400;
    static constexpr uint32_t SPAWN_INTERVAL = 1800;
    static constexpr uint32_t AUTO_RETURN_MS  = 60000;

    void spawn_enemy();
    void shoot();
    void update_bullets(uint32_t dt);
    void update_enemies(uint32_t dt);
    void check_collisions();
    void draw_hud();
    void draw_stars();
    void draw_planets();
};
