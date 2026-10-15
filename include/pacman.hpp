/**
 * Pac-Man arcade game for SSD1963 (480x272, RGB565)
 * Mismo proyecto base: clase SSD1963 + libbcm2835 (GPIO bit-banging).
 *
 * - Laberinto clasico 28x31 casillas de 8 px (224x248), centrado a la izquierda
 * - Panel HUD a la derecha (puntaje, nivel, vidas, ayuda de botones)
 * - 4 fantasmas con IA clasica (scatter/chase, frightened, ojos a la casa)
 * - Render por back-buffer en RAM + subida solo de rectangulos sucios
 * - Entrada: botones GPIO activos en bajo con pull-up interno
 */
#pragma once

#include "ssd1963.hpp"
#include <cstdint>
#include <vector>

class PacmanGame {
public:
    explicit PacmanGame(SSD1963& display);

    void init();      // estado inicial, back-buffer, laberinto, GPIO de entrada
    void frame();     // un frame: entrada + logica + render de rectangulos sucios
    bool is_running() const { return running_; }
    void quit()       { running_ = false; }

private:
    // ------------------------------------------------------------------
    // Geometria del laberinto
    // ------------------------------------------------------------------
    static const uint8_t  MAZE_W  = 28;   // casillas
    static const uint8_t  MAZE_H  = 31;   // casillas
    static const uint8_t  TILE    = 8;    // px por casilla
    static const uint16_t MAZE_X  = 16;   // px: origen X del laberinto en pantalla
    static const uint16_t MAZE_Y  = 12;   // px: origen Y del laberinto
    static const uint8_t  TUNNEL_ROW = 14;
    static const uint16_t HUD_X      = 248;   // px: panel de informacion a la derecha

    // ------------------------------------------------------------------
    // Entrada (botones a GND, pull-up interno, activos en bajo)
    // ------------------------------------------------------------------
    static const uint8_t PIN_UP    = 8;
    static const uint8_t PIN_DOWN  = 9;
    static const uint8_t PIN_LEFT  = 10;
    static const uint8_t PIN_RIGHT = 11;
    static const uint8_t PIN_START = 7;

    // ------------------------------------------------------------------
    // Maquina de estados
    // ------------------------------------------------------------------
    enum class State : uint8_t {
        INTRO,      // presentacion de titulo (~3 s)
        DEMO,       // partida automatica (IA) para el escaparate
        READY,      // "READY!" antes de cada ronda
        PLAYING,
        PAUSED,
        DYING,      // animacion de muerte
        CLEARING,   // laberinto completo: parpadeo de muros
        GAMEOVER
    };

    enum : uint8_t { G_BLINKY = 0, G_PINKY, G_INKY, G_CLYDE, GHOST_COUNT };

    enum : uint8_t {
        GS_HOUSE,   // dentro de la casa, rebotando
        GS_EXIT,    // saliendo por la puerta
        GS_NORMAL,
        GS_FRIGHT,
        GS_EATEN,   // solo ojos volviendo a la casa
        GS_ENTER    // ojos descendiendo por la puerta
    };

    // Entidad sobre el laberinto: casilla actual + desplazamiento en cuartos
    // de pixel desde el centro de esa casilla (0..31 = 0..7.75 px)
    struct Ent {
        int8_t  tx = 0, ty = 0;   // casilla
        int8_t  dx = 0, dy = 0;   // direccion actual
        int16_t off = 0;          // cuartos de px avanzados desde el centro
        int16_t spd = 4;          // cuartos de px por tick (4 = 1 px/tick)
    };

    struct Ghost {
        Ent     e;
        uint8_t state = GS_HOUSE;
        uint8_t id = 0;                // 0..3: Blinky, Pinky, Inky, Clyde
        bool    fright = false;        // azul (comestible)
        uint32_t release_ticks = 0;    // cuando puede salir de la casa
        bool    reverse_pending = false;
    };

    // ------------------------------------------------------------------
    // Miembros
    // ------------------------------------------------------------------
    SSD1963& display_;
    bool running_ = true;
    State state_ = State::INTRO;

    // Back-buffer en RAM (se sube solo lo que cambia)
    std::vector<uint16_t> fb_;
    bool full_redraw_ = true;

    // Rectangulos sucios (hasta MAX_DIRTY; si se desbordan se unen en uno)
    static const uint8_t MAX_DIRTY = 16;
    struct Rect { uint16_t x, y, w, h; };
    Rect    dirty_[MAX_DIRTY];
    uint8_t dirty_count_ = 0;

    // Laberinto
    uint8_t  maze_[MAZE_H][MAZE_W];   // copia de trabajo: 0=vacio,1=punto,2=pastilla,3=muro,4=puerta
    uint16_t dots_left_ = 0;
    uint8_t  dots_eaten_since_ = 0;   // para liberar fantasmas de la casa

    // Entidades
    Ent   pacman_;
    int8_t next_dx_ = -1, next_dy_ = 0;  // direccion solicitada por el jugador
    Ghost ghosts_[GHOST_COUNT];

    // Estado de partida
    uint32_t score_ = 0;
    uint32_t high_score_ = 0;
    int16_t  lives_ = 3;
    uint8_t  level_ = 1;
    bool     extra_life_given_ = false;

    // Modos scatter/chase y miedo
    uint8_t  mode_index_ = 0;
    uint32_t mode_ticks_ = 0;
    uint32_t fright_ticks_ = 0;
    uint8_t  fright_combo_ = 0;
    uint32_t fright_anim_ = 0;

    // Temporizadores de estado
    uint32_t state_ticks_ = 0;

    // Animacion / otros
    uint32_t tick_ = 0;
    uint32_t pellet_blink_ = 0;
    uint32_t attract_scroll_ = 0;
    bool     hud_dirty_ = true;
    bool     start_prev_ = false;
    bool     last_blink_ = false;
    bool     demo_mode_ = false;   // la partida actual la controla la IA

    // ------------------------------------------------------------------
    // Entrada
    // ------------------------------------------------------------------
    bool button(uint8_t pin);
    void read_input();

    // ------------------------------------------------------------------
    // Logica
    // ------------------------------------------------------------------
    void update();
    void update_intro();
    void update_demo();
    void update_ready();
    void update_playing();
    void update_paused();
    void update_dying();
    void update_clearing();
    void update_gameover();

    void new_game();
    void start_demo();     // reinicia partida en modo demo (IA)
    void enter_intro();    // vuelve a la presentacion
    void load_level();
    void reset_positions();
    void lose_life();
    void start_frightened();
    void end_frightened();
    void next_mode();

    bool passable(int8_t tx, int8_t ty, int mode) const; // mode: 0=pacman,1=ghost normal,2=puerta(ojos)
    void move_pacman();
    void move_ghost(Ghost& g);
    void decide_ghost_dir(Ghost& g);
    void ghost_target(const Ghost& g, int8_t& tx, int8_t& ty) const;
    void check_collisions();
    void demo_ai();                    // IA del jugador en modo demo
    int  demo_search(bool chase_ghost); // BFS: primer paso al objetivo
    int  demo_flee();                   // paso que maximiza la distancia a los fantasmas
    void eat_tile();

    static void wrap_tile(int8_t& tx);
    static int16_t center_px_x(const Ent& e);
    static int16_t center_px_y(const Ent& e);

    // ------------------------------------------------------------------
    // Render
    // ------------------------------------------------------------------
    void mark_dirty(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    void render();
    void render_region(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    void render_full();
    void upload(const Rect& r);

    // primitivas sobre el back-buffer
    void fb_pixel(int16_t x, int16_t y, uint16_t color);
    void fb_fill(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
    void draw_tile(int8_t tx, int8_t ty);
    void draw_wall_tile(int16_t sx, int16_t sy, int8_t tx, int8_t ty);
    void draw_pacman(int16_t cx, int16_t cy, int8_t dx, int8_t dy, int mouth2);
    void draw_ghost(const Ghost& g);
    void draw_text(int16_t x, int16_t y, const char* text, uint16_t color, uint8_t scale);
    void draw_text_centered(int16_t y, const char* text, uint16_t color, uint8_t scale);
    void draw_hud();
    void draw_overlay();
    void draw_intro();

    static uint16_t glyph(char c);
};
