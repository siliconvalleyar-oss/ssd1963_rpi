#include <engine/SpaceShooterScene.hpp>
#include <engine/GameEngine.hpp>
#include <cstdio>
#include <cstdlib>
#include <iostream>

static uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    return RGB565CONVERT(r, g, b);
}

SpaceShooterScene::SpaceShooterScene(SSD1963& display)
    : Scene(display)
    , m_player_x(0), m_player_y(0), m_prev_px(0), m_prev_py(0)
    , m_player_dir(1)
    , m_lives(3), m_score(0)
    , m_timer(0), m_shoot_timer(0), m_spawn_timer(0), m_move_timer(0)
    , m_sprites_loaded(false), m_drawn_once(false)
{
    for (auto& b : m_bullets) b.active = false;
    for (auto& e : m_enemies) e.active = false;
    for (auto& s : m_stars) {
        s.x = rand() % 480;
        s.y = float(rand() % 272);
        s.speed = 1 + (rand() % 3);
        s.brightness = 80 + (rand() % 120);
    }
}

bool SpaceShooterScene::on_enter() {
    m_player_x = 224;
    m_player_y = 230;
    m_prev_px = m_player_x;
    m_prev_py = m_player_y;
    m_player_dir = 1;
    m_lives = 3;
    m_score = 0;
    m_timer = m_shoot_timer = m_spawn_timer = m_move_timer = 0;
    m_drawn_once = false;

    for (auto& b : m_bullets) b.active = false;
    for (auto& e : m_enemies) e.active = false;

    if (!m_sprites_loaded) {
        m_sprites_loaded =
            m_spr_player.load("assets/naves/player.spr")  &&
            m_spr_enemy.load("assets/naves/enemy.spr")    &&
            m_spr_bullet.load("assets/naves/bullet.spr")  &&
            m_spr_planet[0].load("assets/naves/planet_00.spr") &&
            m_spr_planet[1].load("assets/naves/planet_01.spr") &&
            m_spr_planet[2].load("assets/naves/planet_02.spr");
    }

    m_display.clear_screen(BG);
    std::cout << "[Shooter] START! lives=" << (int)m_lives << "\n";
    return true;
}

void SpaceShooterScene::update(uint32_t dt) {
    m_timer += dt;

    if (m_timer > AUTO_RETURN_MS || m_lives == 0) {
        if (m_engine && m_engine->menu_scene()) {
            m_engine->set_scene(m_engine->menu_scene());
        }
        return;
    }

    m_move_timer += dt;
    if (m_move_timer > 30) {
        m_move_timer = 0;
        m_player_x += PLAYER_SPEED * m_player_dir;
        if (m_player_x <= 10)  m_player_dir = 1;
        if (m_player_x >= 438) m_player_dir = -1;
    }

    m_shoot_timer += dt;
    if (m_shoot_timer > SHOOT_INTERVAL) {
        m_shoot_timer = 0;
        shoot();
    }

    m_spawn_timer += dt;
    if (m_spawn_timer > SPAWN_INTERVAL) {
        m_spawn_timer = 0;
        spawn_enemy();
    }

    update_bullets(dt);
    update_enemies(dt);
    check_collisions();
}

void SpaceShooterScene::draw() {
    // --- Erase old positions ---
    uint16_t bg_for_erase = BG;

    // Player
    m_display.fill_rect(m_prev_px - 1, m_prev_py - 1,
                        PLAYER_W + 2, PLAYER_H + 2, bg_for_erase);
    // Bullets
    for (auto& b : m_bullets) {
        if (b.active)
            m_display.fill_rect(b.x, b.y, BULLET_W, BULLET_H, bg_for_erase);
    }
    // Enemies
    for (auto& e : m_enemies) {
        if (e.active)
            m_display.fill_rect(e.x, e.y, ENEMY_W, ENEMY_H, bg_for_erase);
    }

    // --- Background stars (parallax) ---
    draw_stars();

    // --- Static planets (once) ---
    if (!m_drawn_once) {
        m_drawn_once = true;
        m_spr_planet[0].draw(m_display, 20,  20);
        m_spr_planet[1].draw(m_display, 340, 150);
        m_spr_planet[2].draw(m_display, 200, 100);
    }

    // --- Draw sprites at new positions ---
    for (auto& b : m_bullets) {
        if (b.active)
            m_spr_bullet.draw(m_display, (int16_t)b.x, (int16_t)b.y);
    }
    for (auto& e : m_enemies) {
        if (e.active)
            m_spr_enemy.draw(m_display, (int16_t)e.x, (int16_t)e.y);
    }
    m_spr_player.draw(m_display, (int16_t)m_player_x, (int16_t)m_player_y);

    // --- HUD ---
    draw_hud();

    // --- Save positions for next frame ---
    m_prev_px = m_player_x;
    m_prev_py = m_player_y;
}

const char* SpaceShooterScene::name() const { return "SpaceShooterScene"; }

void SpaceShooterScene::shoot() {
    for (auto& b : m_bullets) {
        if (!b.active) {
            b.x = m_player_x + (PLAYER_W - BULLET_W) / 2;
            b.y = m_player_y - BULLET_H;
            b.active = true;
            break;
        }
    }
}

void SpaceShooterScene::spawn_enemy() {
    for (auto& e : m_enemies) {
        if (!e.active) {
            e.x = 10 + (rand() % (460 - ENEMY_W));
            e.y = -ENEMY_H;
            e.active = true;
            e.hp = 1;
            e.timer = 0;
            break;
        }
    }
}

void SpaceShooterScene::update_bullets(uint32_t /*dt*/) {
    for (auto& b : m_bullets) {
        if (!b.active) continue;
        b.y -= BULLET_SPEED;
        if (b.y + BULLET_H < 0) b.active = false;
    }
}

void SpaceShooterScene::update_enemies(uint32_t dt) {
    for (auto& e : m_enemies) {
        if (!e.active) continue;
        e.timer += dt;
        e.y += ENEMY_SPEED;
        e.x += 0.3f * (e.timer % 2000 < 1000 ? 1 : -1);
        if (e.x < 5) e.x = 5;
        if (e.x > 480 - ENEMY_W - 5) e.x = 480 - ENEMY_W - 5;
        if (e.y > LCD_HEIGHT) e.active = false;
    }
}

void SpaceShooterScene::check_collisions() {
    for (auto& b : m_bullets) {
        if (!b.active) continue;
        for (auto& e : m_enemies) {
            if (!e.active) continue;
            bool hit = (b.x + BULLET_W > e.x) &&
                       (b.x < e.x + ENEMY_W) &&
                       (b.y + BULLET_H > e.y) &&
                       (b.y < e.y + ENEMY_H);
            if (hit) {
                b.active = false;
                e.active = false;
                m_score += 10;
                std::cout << "[Shooter] HIT! score=" << m_score << "\n";
                break;
            }
        }
    }
    for (auto& e : m_enemies) {
        if (!e.active) continue;
        bool hit = (m_player_x + PLAYER_W > e.x) &&
                   (m_player_x < e.x + ENEMY_W) &&
                   (m_player_y + PLAYER_H > e.y) &&
                   (m_player_y < e.y + ENEMY_H);
        if (hit) {
            e.active = false;
            m_lives--;
            std::cout << "[Shooter] DAMAGE! lives=" << (int)m_lives << "\n";
            if (m_lives == 0)
                std::cout << "[Shooter] GAME OVER! score=" << m_score << "\n";
        }
    }
}

void SpaceShooterScene::draw_hud() {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "SCORE: %04u", m_score);
    m_display.draw_string(4, 2, buf, rgb(255, 255, 100), BG);
    for (uint8_t i = 0; i < m_lives; i++) {
        m_display.draw_string(4 + i * 30, LCD_HEIGHT - 12, "v",
                              rgb(255, 80, 80), BG);
    }
    uint32_t secs = m_timer / 1000;
    std::snprintf(buf, sizeof(buf), "%02u:%02u", secs / 60, secs % 60);
    m_display.draw_string(LCD_WIDTH - 48, 2, buf, rgb(150, 150, 150), BG);
    if (m_lives == 0) {
        m_display.draw_string_centered(LCD_WIDTH / 2, LCD_HEIGHT / 2 - 10,
                                       "GAME OVER", rgb(255, 0, 0), BG);
        std::snprintf(buf, sizeof(buf), "SCORE: %u", m_score);
        m_display.draw_string_centered(LCD_WIDTH / 2, LCD_HEIGHT / 2 + 10,
                                       buf, rgb(255, 255, 0), BG);
    }
}

void SpaceShooterScene::draw_stars() {
    for (auto& s : m_stars) {
        // Erase old star
        m_display.draw_pixel(s.x, (uint16_t)s.y, BG);
        // Move
        s.y += s.speed * 0.5f;
        if (s.y > LCD_HEIGHT) {
            s.y = 0;
            s.x = rand() % 480;
        }
        // Draw new star
        uint8_t br = s.brightness;
        m_display.draw_pixel(s.x, (uint16_t)s.y, rgb(br/3, br/3, br));
    }
}
