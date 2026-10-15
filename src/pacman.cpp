/**
 * Pac-Man arcade game for SSD1963 (480x272, RGB565)
 * Implementacion sobre la base SSD1963 + libbcm2835.
 *
 * Laberinto clasico 28x31 (casillas de 8 px) a la izquierda, HUD a la derecha.
 * Render: back-buffer en RAM; solo se suben al display los rectangulos sucios,
 * imprescindible porque el bus 8080 bit-bangueado es lento.
 */
#include "pacman.hpp"
#include "config_hw.hpp"
#include "color.hpp"

#include <bcm2835.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>

// ----------------------------------------------------------------------
// Laberinto clasico 28x31. '#' muro, '.' punto, 'o' pastilla, '-' puerta
// de la casa, ' ' vacio, 'G' interior de la casa (sin puntos)
// ----------------------------------------------------------------------
static const char* const MAZE_ORIG[31] = {
    "############################", // 0
    "#............##............#", // 1
    "#.####.#####.##.#####.####.#", // 2
    "#o####.#####.##.#####.####o#", // 3
    "#.####.#####.##.#####.####.#", // 4
    "#..........................#", // 5
    "#.####.##.########.##.####.#", // 6
    "#.####.##.########.##.####.#", // 7
    "#......##....##....##......#", // 8
    "######.##### ## #####.######", // 9
    "######.##### ## #####.######", // 10
    "######.##          ##.######", // 11
    "######.## ###--### ##.######", // 12
    "######.## #GGGGGG# ##.######", // 13
    "      .   #GGGGGG#   .      ", // 14 (tunel)
    "######.## #GGGGGG# ##.######", // 15
    "######.## ######## ##.######", // 16
    "######.##          ##.######", // 17
    "######.## ######## ##.######", // 18
    "######.## ######## ##.######", // 19
    "#............##............#", // 20
    "#.####.#####.##.#####.####.#", // 21
    "#.####.#####.##.#####.####.#", // 22
    "#o..##................##..o#", // 23
    "###.##.##.########.##.##.###", // 24
    "###.##.##.########.##.##.###", // 25
    "#......##....##....##......#", // 26
    "#.##########.##.##########.#", // 27
    "#.##########.##.##########.#", // 28
    "#..........................#", // 29
    "############################", // 30
};

// ----------------------------------------------------------------------
// Colores RGB565
// ----------------------------------------------------------------------
static constexpr uint16_t CLR_BG        = BLACK;
static constexpr uint16_t CLR_WALL_EDGE = 0x211F;  // azul clasico
static constexpr uint16_t CLR_WALL_FILL = 0x08A2;  // azul muy oscuro
static constexpr uint16_t CLR_WALL_FLASH= 0xFFFF;  // parpadeo al completar nivel
static constexpr uint16_t CLR_DOOR      = 0xA5FF;  // azul claro (boca de la casa)
static constexpr uint16_t CLR_DOT       = 0xFDD2;  // crema
static constexpr uint16_t CLR_PELLET    = 0xFFFF;
static constexpr uint16_t CLR_PAC       = YELLOW;
static constexpr uint16_t CLR_FRIGHT    = 0x001F;
static constexpr uint16_t CLR_FRIGHT2   = 0xFFFF;
static constexpr uint16_t CLR_EYE       = 0xFFFF;
static constexpr uint16_t CLR_PUPIL     = 0x001F;
static constexpr uint16_t CLR_TEXT      = WHITE;

static const uint16_t GHOST_COLORS[4] = { RED, MAGENTA, CYAN, 0xFD20 }; // naranja

// Modos scatter/chase (en ticks de 60 Hz)
struct ModeInfo { bool scatter; uint32_t ticks; };
static const ModeInfo MODES[8] = {
    {true,   420},   // scatter 7 s
    {false, 1200},   // chase 20 s
    {true,   420},
    {false, 1200},
    {true,   300},   // 5 s
    {false, 1200},
    {true,   300},
    {false, 0x7FFFFFFFu},
};
static const uint8_t MODE_COUNT = 8;

// Esquinas de scatter por fantasma
static const int8_t SCATTER[4][2] = {{25, 0}, {2, 0}, {27, 30}, {0, 30}};

// Posiciones iniciales
static const int8_t PAC_START[2] = {13, 23};

// ----------------------------------------------------------------------
// Fuente 3x5 (5 filas de 3 bits, bit0 = columna izquierda)
// ----------------------------------------------------------------------
#define GL(a, b, c, d, e) ((uint16_t)((a) | ((b) << 3) | ((c) << 6) | ((d) << 9) | ((e) << 12)))

static const uint16_t FONT[41] = {
    GL(7,5,5,5,7), GL(2,6,2,2,7), GL(7,1,7,4,7), GL(7,1,7,1,7), GL(5,5,7,1,1), // 0-4
    GL(7,4,7,1,7), GL(7,4,7,5,7), GL(7,1,2,2,2), GL(7,5,7,5,7), GL(7,5,7,1,7), // 5-9
    GL(2,5,7,5,5), GL(6,5,6,5,6), GL(3,4,4,4,3), GL(6,5,5,5,6), GL(7,4,7,4,7), // A-E
    GL(7,4,7,4,4), GL(3,4,5,5,3), GL(5,5,7,5,5), GL(7,2,2,2,7), GL(1,1,1,5,2), // F-J
    GL(5,5,6,5,5), GL(4,4,4,4,7), GL(5,7,7,5,5), GL(6,5,5,5,5), GL(2,5,5,5,2), // K-O
    GL(6,5,6,4,4), GL(2,5,5,6,3), GL(6,5,6,5,5), GL(3,4,2,1,6), GL(7,2,2,2,2), // P-T
    GL(5,5,5,5,7), GL(5,5,5,5,2), GL(5,5,7,7,5), GL(5,5,2,5,5), GL(5,5,2,2,2), // U-Y
    GL(7,1,2,4,7),                                   // Z
    GL(0,0,7,0,0), GL(0,0,0,0,2), GL(0,2,0,2,0), GL(2,2,2,0,2), GL(0,0,0,0,0), // - . : ! ' '
};

static uint16_t glyph_index(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'Z') return 10 + (c - 'A');
    switch (c) {
        case '-': return 36;
        case '.': return 37;
        case ':': return 38;
        case '!': return 39;
        default:  return 40;
    }
}

uint16_t PacmanGame::glyph(char c) { return FONT[glyph_index(c)]; }

// ----------------------------------------------------------------------
// Construccion
// ----------------------------------------------------------------------
PacmanGame::PacmanGame(SSD1963& display) : display_(display) {}

void PacmanGame::init() {
    fb_.assign((size_t)LCD_WIDTH * LCD_HEIGHT, CLR_BG);

    // Botones: entradas con pull-up, activos en bajo
    const uint8_t pins[] = {PIN_UP, PIN_DOWN, PIN_LEFT, PIN_RIGHT, PIN_START};
    for (uint8_t p : pins) {
        bcm2835_gpio_fsel(p, BCM2835_GPIO_FSEL_INPT);
        bcm2835_gpio_set_pud(p, BCM2835_GPIO_PUD_UP);
    }

    load_level();
    reset_positions();
    state_ = State::ATTRACT;
    full_redraw_ = true;
}

// ----------------------------------------------------------------------
// Entrada
// ----------------------------------------------------------------------
bool PacmanGame::button(uint8_t pin) {
    return bcm2835_gpio_lev(pin) == LOW;
}

void PacmanGame::read_input() {
    if (button(PIN_UP))    { next_dx_ = 0;  next_dy_ = -1; }
    if (button(PIN_DOWN))  { next_dx_ = 0;  next_dy_ =  1; }
    if (button(PIN_LEFT))  { next_dx_ = -1; next_dy_ =  0; }
    if (button(PIN_RIGHT)) { next_dx_ =  1; next_dy_ =  0; }

    bool s = button(PIN_START);
    bool pressed = s && !start_prev_;
    start_prev_ = s;
    if (!pressed) return;

    switch (state_) {
        case State::ATTRACT:  new_game(); break;
        case State::PLAYING:  state_ = State::PAUSED;
                              mark_dirty(MAZE_X, MAZE_Y + 13 * TILE, MAZE_W * TILE, 56);
                              break;
        case State::PAUSED:   state_ = State::PLAYING;
                              mark_dirty(MAZE_X, MAZE_Y + 13 * TILE, MAZE_W * TILE, 56);
                              break;
        default: break;
    }
}

// ----------------------------------------------------------------------
// Partida
// ----------------------------------------------------------------------
void PacmanGame::new_game() {
    score_ = 0;
    lives_ = 3;
    level_ = 1;
    extra_life_given_ = false;
    load_level();
    reset_positions();
    state_ = State::READY;
    state_ticks_ = 0;
    hud_dirty_ = true;
    full_redraw_ = true;
}

void PacmanGame::load_level() {
    dots_left_ = 0;
    for (int8_t ty = 0; ty < MAZE_H; ty++) {
        for (int8_t tx = 0; tx < MAZE_W; tx++) {
            switch (MAZE_ORIG[ty][tx]) {
                case '#': maze_[ty][tx] = 3; break;
                case '.': maze_[ty][tx] = 1; dots_left_++; break;
                case 'o': maze_[ty][tx] = 2; dots_left_++; break;
                case '-': maze_[ty][tx] = 4; break;
                default:  maze_[ty][tx] = 0; break; // ' ' y 'G'
            }
        }
    }
    mode_index_ = 0;
    mode_ticks_ = 0;
    fright_ticks_ = 0;
    fright_combo_ = 0;
    dots_eaten_since_ = 0;
}

void PacmanGame::reset_positions() {
    pacman_.tx = PAC_START[0];
    pacman_.ty = PAC_START[1];
    pacman_.dx = -1;
    pacman_.dy = 0;
    pacman_.off = 0;
    pacman_.spd = 4;
    next_dx_ = -1;
    next_dy_ = 0;

    // Blinky fuera de la casa, el resto dentro
    ghosts_[G_BLINKY] = Ghost();
    ghosts_[G_BLINKY].e.tx = 13; ghosts_[G_BLINKY].e.ty = 11;
    ghosts_[G_BLINKY].e.dx = -1;
    ghosts_[G_BLINKY].state = GS_NORMAL;
    ghosts_[G_BLINKY].release_ticks = 0;

    ghosts_[G_PINKY] = Ghost();
    ghosts_[G_PINKY].e.tx = 13; ghosts_[G_PINKY].e.ty = 13;
    ghosts_[G_PINKY].state = GS_HOUSE;
    ghosts_[G_PINKY].release_ticks = tick_ + 60;

    ghosts_[G_INKY] = Ghost();
    ghosts_[G_INKY].e.tx = 11; ghosts_[G_INKY].e.ty = 14;
    ghosts_[G_INKY].state = GS_HOUSE;
    ghosts_[G_INKY].release_ticks = tick_ + 300;

    ghosts_[G_CLYDE] = Ghost();
    ghosts_[G_CLYDE].e.tx = 16; ghosts_[G_CLYDE].e.ty = 14;
    ghosts_[G_CLYDE].state = GS_HOUSE;
    ghosts_[G_CLYDE].release_ticks = tick_ + 600;

    for (uint8_t i = 0; i < GHOST_COUNT; i++) {
        ghosts_[i].id = i;
        ghosts_[i].e.spd = 4;
        ghosts_[i].fright = false;
        ghosts_[i].reverse_pending = false;
    }
}

void PacmanGame::lose_life() {
    lives_--;
    hud_dirty_ = true;
    if (lives_ < 0) {
        state_ = State::GAMEOVER;
        state_ticks_ = 0;
        if (score_ > high_score_) high_score_ = score_;
        hud_dirty_ = true;
        full_redraw_ = true;
    } else {
        reset_positions();
        state_ = State::READY;
        state_ticks_ = 0;
        full_redraw_ = true;
    }
}

void PacmanGame::start_frightened() {
    uint32_t dur = (level_ <= 1) ? 360 : (level_ <= 4) ? 240 : 120;
    fright_ticks_ = dur;
    fright_combo_ = 0;
    for (uint8_t i = 0; i < GHOST_COUNT; i++) {
        if (ghosts_[i].state == GS_NORMAL) {
            ghosts_[i].fright = true;
            ghosts_[i].reverse_pending = true;
        }
    }
}

void PacmanGame::end_frightened() {
    for (uint8_t i = 0; i < GHOST_COUNT; i++) {
        if (ghosts_[i].fright) {
            ghosts_[i].fright = false;
            ghosts_[i].reverse_pending = true;
        }
    }
    fright_combo_ = 0;
}

void PacmanGame::next_mode() {
    if (mode_index_ + 1 < MODE_COUNT) {
        mode_index_++;
        for (uint8_t i = 0; i < GHOST_COUNT; i++) {
            if (ghosts_[i].state == GS_NORMAL) ghosts_[i].reverse_pending = true;
        }
    }
}

// ----------------------------------------------------------------------
// Colisiones de casilla
// mode: 0 = pacman, 1 = fantasma normal, 2 = ojos (cualquier cosa salvo muro)
// ----------------------------------------------------------------------
bool PacmanGame::passable(int8_t tx, int8_t ty, int mode) const {
    if (ty < 0 || ty >= MAZE_H) return false;
    int8_t x = tx;
    if (ty == TUNNEL_ROW) {            // tunel: envolve horizontalmente
        if (x < 0) x = MAZE_W - 1;
        else if (x >= MAZE_W) x = 0;
    }
    if (x < 0 || x >= MAZE_W) return false;
    uint8_t t = maze_[ty][x];
    if (mode == 0) return t != 3 && t != 4;   // pacman: sin muros ni puerta
    if (mode == 1) return t == 0 || t == 1 || t == 2;
    return t != 3;                            // ojos: atraviesan todo salvo muro
}

void PacmanGame::wrap_tile(int8_t& tx) {
    if (tx < 0) tx = MAZE_W - 1;
    else if (tx >= MAZE_W) tx = 0;
}

int16_t PacmanGame::center_px_x(const Ent& e) {
    return (int16_t)(MAZE_X + e.tx * TILE + 4) + (int16_t)e.dx * (e.off >> 2);
}
int16_t PacmanGame::center_px_y(const Ent& e) {
    return (int16_t)(MAZE_Y + e.ty * TILE + 4) + (int16_t)e.dy * (e.off >> 2);
}

// ----------------------------------------------------------------------
// Movimiento
// ----------------------------------------------------------------------
void PacmanGame::eat_tile() {
    uint8_t& t = maze_[pacman_.ty][pacman_.tx];
    if (t == 1) {
        t = 0;
        dots_left_--;
        dots_eaten_since_++;
        score_ += 10;
        hud_dirty_ = true;
        mark_dirty((uint16_t)(MAZE_X + pacman_.tx * TILE - 1),
                   (uint16_t)(MAZE_Y + pacman_.ty * TILE - 1), 10, 10);
    } else if (t == 2) {
        t = 0;
        dots_left_--;
        score_ += 50;
        hud_dirty_ = true;
        mark_dirty((uint16_t)(MAZE_X + pacman_.tx * TILE - 1),
                   (uint16_t)(MAZE_Y + pacman_.ty * TILE - 1), 10, 10);
        start_frightened();
    }
    if (score_ > high_score_) high_score_ = score_;
    if (!extra_life_given_ && score_ >= 10000) {
        extra_life_given_ = true;
        lives_++;
        hud_dirty_ = true;
    }
}

void PacmanGame::move_pacman() {
    Ent& e = pacman_;
    if (e.off == 0) {
        if ((next_dx_ != 0 || next_dy_ != 0) && passable(e.tx + next_dx_, e.ty + next_dy_, 0)) {
            e.dx = next_dx_;
            e.dy = next_dy_;
        }
        if (!passable(e.tx + e.dx, e.ty + e.dy, 0)) {
            e.dx = 0;
            e.dy = 0;
        }
    }
    if (e.dx == 0 && e.dy == 0) return;
    e.off += e.spd;
    while (e.off >= 32) {
        e.off -= 32;
        e.tx = (int8_t)(e.tx + e.dx);
        e.ty = (int8_t)(e.ty + e.dy);
        wrap_tile(e.tx);
        eat_tile();
    }
}

void PacmanGame::ghost_target(const Ghost& g, int8_t& tx, int8_t& ty) const {
    if (MODES[mode_index_].scatter) {
        tx = SCATTER[g.id][0];
        ty = SCATTER[g.id][1];
        return;
    }
    const Ent& p = pacman_;
    switch (g.id) {
        case G_BLINKY:
            tx = p.tx; ty = p.ty;
            break;
        case G_PINKY:
            tx = (int8_t)(p.tx + 4 * p.dx);
            ty = (int8_t)(p.ty + 4 * p.dy);
            break;
        case G_INKY: {
            const Ent& b = ghosts_[G_BLINKY].e;
            int16_t ax = p.tx + 2 * p.dx;
            int16_t ay = p.ty + 2 * p.dy;
            tx = (int8_t)(2 * ax - b.tx);
            ty = (int8_t)(2 * ay - b.ty);
            break;
        }
        default: { // CLYDE
            int16_t dx = g.e.tx - p.tx;
            int16_t dy = g.e.ty - p.ty;
            if (dx * dx + dy * dy > 64) { tx = p.tx; ty = p.ty; }
            else { tx = SCATTER[G_CLYDE][0]; ty = SCATTER[G_CLYDE][1]; }
            break;
        }
    }
}

void PacmanGame::decide_ghost_dir(Ghost& g) {
    Ent& e = g.e;
    static const int8_t dirs[4][2] = {{0, -1}, {-1, 0}, {0, 1}, {1, 0}}; // arriba, izq, abajo, der
    int8_t best_dx = e.dx, best_dy = e.dy;
    bool found = false;

    if (g.fright) {
        int8_t opts[4][2];
        int n = 0;
        for (int i = 0; i < 4; i++) {
            int8_t ddx = dirs[i][0], ddy = dirs[i][1];
            if ((ddx == -e.dx && ddy == -e.dy) && (e.dx != 0 || e.dy != 0)) continue;
            if (passable((int8_t)(e.tx + ddx), (int8_t)(e.ty + ddy), 1)) {
                opts[n][0] = ddx; opts[n][1] = ddy; n++;
            }
        }
        if (n > 0) {
            int k = rand() % n;
            best_dx = opts[k][0];
            best_dy = opts[k][1];
            found = true;
        }
    } else {
        int8_t ttx, tty;
        ghost_target(g, ttx, tty);
        int32_t best_d = -1;
        for (int i = 0; i < 4; i++) {
            int8_t ddx = dirs[i][0], ddy = dirs[i][1];
            if ((ddx == -e.dx && ddy == -e.dy) && (e.dx != 0 || e.dy != 0)) continue;
            if (!passable((int8_t)(e.tx + ddx), (int8_t)(e.ty + ddy), 1)) continue;
            int32_t dx = (int32_t)e.tx + ddx - ttx;
            int32_t dy = (int32_t)e.ty + ddy - tty;
            int32_t d = dx * dx + dy * dy;
            if (best_d < 0 || d < best_d) {
                best_d = d;
                best_dx = ddx;
                best_dy = ddy;
                found = true;
            }
        }
    }
    if (!found) { // callejon sin salida: marcha atras
        best_dx = (int8_t)-e.dx;
        best_dy = (int8_t)-e.dy;
    }
    e.dx = best_dx;
    e.dy = best_dy;
}

void PacmanGame::move_ghost(Ghost& g) {
    Ent& e = g.e;
    switch (g.state) {
        case GS_HOUSE: {
            e.spd = 2;
            if (e.off == 0) {
                // rebotar verticalmente entre las filas 13 y 15 de la casa
                if (e.dy == 0 || (e.ty <= 13 && e.dy < 0)) e.dy = 1;
                else if (e.ty >= 15 && e.dy > 0) e.dy = -1;
                if (e.dy == 0) e.dy = -1;
            }
            e.off = (int16_t)(e.off + e.spd);
            while (e.off >= 32) { e.off -= 32; e.ty = (int8_t)(e.ty + e.dy); }
            if (tick_ >= g.release_ticks) {
                g.state = GS_EXIT;
                e.dx = 0; e.dy = 0; e.off = 0;
            }
            break;
        }
        case GS_EXIT: {
            e.spd = 4;
            const int8_t door_col = (g.id % 2 == 0) ? 13 : 14;
            if (e.off == 0) {
                if (e.tx != door_col) {
                    e.dx = (e.tx < door_col) ? 1 : -1;
                    e.dy = 0;
                } else if (e.ty > 11) {
                    e.dx = 0;
                    e.dy = -1;
                } else {
                    e.dx = -1; e.dy = 0; e.off = 0;
                    g.state = GS_NORMAL;
                    break;
                }
            }
            e.off = (int16_t)(e.off + e.spd);
            while (e.off >= 32) {
                e.off -= 32;
                e.tx = (int8_t)(e.tx + e.dx);
                e.ty = (int8_t)(e.ty + e.dy);
            }
            break;
        }
        case GS_NORMAL: {
            bool in_tunnel = (e.ty == TUNNEL_ROW && (e.tx <= 9 || e.tx >= 18));
            int16_t spd = (level_ >= 5) ? 5 : 4;
            if (level_ >= 16) spd = 6;
            if (g.fright) spd = 3;
            if (in_tunnel) spd = 2;
            e.spd = spd;
            if (e.off == 0) {
                if (g.reverse_pending) {
                    e.dx = (int8_t)-e.dx;
                    e.dy = (int8_t)-e.dy;
                    g.reverse_pending = false;
                }
                decide_ghost_dir(g);
            }
            e.off = (int16_t)(e.off + e.spd);
            while (e.off >= 32) {
                e.off -= 32;
                e.tx = (int8_t)(e.tx + e.dx);
                e.ty = (int8_t)(e.ty + e.dy);
                wrap_tile(e.tx);
            }
            break;
        }
        case GS_EATEN: {
            e.spd = 8;
            if (e.off == 0) {
                if (e.tx != 13) {
                    e.dx = (e.tx < 13) ? 1 : -1;
                    e.dy = 0;
                } else if (e.ty != 11) {
                    e.dx = 0;
                    e.dy = (e.ty < 11) ? 1 : -1;
                } else {
                    e.dx = 0; e.dy = 1;
                    g.state = GS_ENTER;
                }
            }
            e.off = (int16_t)(e.off + e.spd);
            while (e.off >= 32) {
                e.off -= 32;
                e.tx = (int8_t)(e.tx + e.dx);
                e.ty = (int8_t)(e.ty + e.dy);
                wrap_tile(e.tx);
            }
            break;
        }
        case GS_ENTER: {
            e.spd = 4;
            e.dx = 0;
            e.dy = 1;
            e.off = (int16_t)(e.off + e.spd);
            while (e.off >= 32) {
                e.off -= 32;
                e.ty = (int8_t)(e.ty + e.dy);
                if (e.ty >= 14) {
                    e.ty = 14;
                    e.off = 0;
                    e.dy = -1;
                    g.state = GS_HOUSE;
                    g.release_ticks = tick_ + 90;
                    break;
                }
            }
            break;
        }
    }
}

// ----------------------------------------------------------------------
// Colisiones pacman-fantasma
// ----------------------------------------------------------------------
void PacmanGame::check_collisions() {
    int16_t px = center_px_x(pacman_);
    int16_t py = center_px_y(pacman_);

    for (uint8_t i = 0; i < GHOST_COUNT; i++) {
        Ghost& g = ghosts_[i];
        if (g.state == GS_HOUSE || g.state == GS_EATEN || g.state == GS_ENTER) continue;
        int16_t gx = center_px_x(g.e);
        int16_t gy = center_px_y(g.e);

        int16_t ddx = (int16_t)abs(gx - px);
        int16_t ddy = (int16_t)abs(gy - py);
        // tunel (fila 14): las casillas 0 y 27 estan conectadas; en la
        // practica una separacion X mayor que la mitad del laberinto solo
        // ocurre alli, por lo que se envuelve la distancia
        if (ddx > (int16_t)(MAZE_W * TILE / 2)) ddx = (int16_t)(MAZE_W * TILE - ddx);
        if (ddx < 7 && ddy < 7) {
            if (g.fright) {
                uint32_t pts = 200u << (fright_combo_ > 3 ? 3 : fright_combo_);
                score_ += pts;
                if (score_ > high_score_) high_score_ = score_;
                if (fright_combo_ < 3) fright_combo_++;
                g.fright = false;
                g.state = GS_EATEN;
                hud_dirty_ = true;
                mark_dirty((uint16_t)(gx - 6), (uint16_t)(gy - 6), 12, 12);
            } else {
                state_ = State::DYING;
                state_ticks_ = 0;
                mark_dirty((uint16_t)(px - 6), (uint16_t)(py - 6), 12, 12);
                return;
            }
        }
    }
}

// ----------------------------------------------------------------------
// Update por estados
// ----------------------------------------------------------------------
void PacmanGame::update() {
    switch (state_) {
        case State::ATTRACT:  update_attract();  break;
        case State::READY:    update_ready();    break;
        case State::PLAYING:  update_playing();  break;
        case State::PAUSED:   update_paused();   break;
        case State::DYING:    update_dying();    break;
        case State::CLEARING: update_clearing(); break;
        case State::GAMEOVER: update_gameover(); break;
    }
}

void PacmanGame::update_attract() {
    attract_scroll_++;
    // linea de persecucion: limpiar rastro y marcar posiciones nuevas
    int16_t base_old = (int16_t)(LCD_WIDTH - ((attract_scroll_ - 1) % (LCD_WIDTH + 96)));
    int16_t base_new = (int16_t)(LCD_WIDTH - (attract_scroll_ % (LCD_WIDTH + 96)));
    for (int i = 0; i < 5; i++) {
        int16_t ox = base_old + i * 16;
        int16_t nx = base_new + i * 16;
        if (ox > -8 && ox < LCD_WIDTH) mark_dirty((uint16_t)ox, 164, 8, 12);
        if (nx > -8 && nx < LCD_WIDTH) mark_dirty((uint16_t)nx, 164, 8, 12);
    }
    bool blink = (tick_ / 30) & 1;
    if (blink != last_blink_) {
        last_blink_ = blink;
        mark_dirty(MAZE_X + 32, 212, 160, 14);
    }
}

void PacmanGame::update_ready() {
    state_ticks_++;
    if (state_ticks_ >= 150) {
        state_ = State::PLAYING;
        mark_dirty(MAZE_X, MAZE_Y + 13 * TILE, MAZE_W * TILE, 40);
    }
}

void PacmanGame::update_playing() {
    // cronometro de modos scatter/chase (se congela con el miedo)
    if (fright_ticks_ == 0) {
        mode_ticks_++;
        if (mode_ticks_ > MODES[mode_index_].ticks) next_mode();
    } else {
        fright_ticks_--;
        if (fright_ticks_ == 0) end_frightened();
    }

    move_pacman();
    for (uint8_t i = 0; i < GHOST_COUNT; i++) move_ghost(ghosts_[i]);
    check_collisions();
    if (state_ != State::PLAYING) return; // colision ha cambiado el estado

    // parpadeo de pastillas
    uint32_t phase = (tick_ / 25) & 1;
    if (phase != pellet_blink_) {
        pellet_blink_ = phase;
        for (int8_t ty = 0; ty < MAZE_H; ty++) {
            for (int8_t tx = 0; tx < MAZE_W; tx++) {
                if (maze_[ty][tx] == 2) {
                    mark_dirty((uint16_t)(MAZE_X + tx * TILE),
                               (uint16_t)(MAZE_Y + ty * TILE), 8, 8);
                }
            }
        }
    }

    if (dots_left_ == 0) {
        state_ = State::CLEARING;
        state_ticks_ = 0;
        full_redraw_ = true;
    }
    if (hud_dirty_) {
        hud_dirty_ = false;
        // marcar solo las zonas del HUD que cambian (el panel completo
        // son 63k px, demasiados para subirlos en cada punto)
        mark_dirty(HUD_X + 4, 28, 100, 16);   // 1UP / puntaje
        mark_dirty(HUD_X + 4, 68, 100, 16);   // HIGH
        mark_dirty(HUD_X + 4, 112, 48, 16);   // LEVEL
        mark_dirty(HUD_X, 160, 80, 20);       // vidas
    }
}

void PacmanGame::update_paused() {
    // congelado; el parpadeo global sigue viajando con tick_
}

void PacmanGame::update_dying() {
    state_ticks_++;
    // animacion: marcar siempre el area de pacman (se redibuja/oculta en render)
    int16_t px = center_px_x(pacman_);
    int16_t py = center_px_y(pacman_);
    mark_dirty((uint16_t)(px - 6), (uint16_t)(py - 6), 12, 12);
    if (state_ticks_ >= 150) lose_life();
}

void PacmanGame::update_clearing() {
    state_ticks_++;
    if (state_ticks_ >= 210) {
        if (level_ < 255) level_++;
        hud_dirty_ = true;
        load_level();
        reset_positions();
        state_ = State::READY;
        state_ticks_ = 0;
        full_redraw_ = true;
    }
}

void PacmanGame::update_gameover() {
    state_ticks_++;
    if (state_ticks_ >= 300) {
        state_ = State::ATTRACT;
        attract_scroll_ = 0;
        full_redraw_ = true;
    }
}

// ----------------------------------------------------------------------
// Render: rectangulos sucios
// ----------------------------------------------------------------------
void PacmanGame::mark_dirty(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    if (w == 0 || h == 0) return;
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    if (x + w > LCD_WIDTH) w = LCD_WIDTH - x;
    if (y + h > LCD_HEIGHT) h = LCD_HEIGHT - y;

    if (dirty_count_ >= MAX_DIRTY) { // desbordamiento: unir todo en uno
        Rect u = dirty_[0];
        for (uint8_t i = 1; i < dirty_count_; i++) {
            uint16_t x2 = dirty_[i].x + dirty_[i].w;
            uint16_t y2 = dirty_[i].y + dirty_[i].h;
            uint16_t u2 = u.x + u.w;
            uint16_t v2 = u.y + u.h;
            if (dirty_[i].x < u.x) u.x = dirty_[i].x;
            if (dirty_[i].y < u.y) u.y = dirty_[i].y;
            if (x2 > u2) u2 = x2;
            if (y2 > v2) v2 = y2;
            if (u.x + u.w < u2) u.w = u2 - u.x;
            if (u.y + u.h < v2) u.h = v2 - u.y;
        }
        dirty_[0] = u;
        dirty_count_ = 1;
    }
    dirty_[dirty_count_++] = Rect{x, y, w, h};
}

void PacmanGame::render() {
    if (full_redraw_) {
        full_redraw_ = false;
        dirty_count_ = 0;
        render_full();
        return;
    }
    if (dirty_count_ == 0) return;
    for (uint8_t i = 0; i < dirty_count_; i++) {
        render_region(dirty_[i].x, dirty_[i].y, dirty_[i].w, dirty_[i].h);
        upload(dirty_[i]);
    }
    dirty_count_ = 0;
}

void PacmanGame::upload(const Rect& r) {
    if (r.w == 0 || r.h == 0) return;
    // El back-buffer es full-width: cada fila del rect es contigua,
    // asi que se sube fila a fila (el driver hace un set_area por fila).
    for (uint16_t j = 0; j < r.h; j++) {
        const uint16_t* p = &fb_[(size_t)(r.y + j) * LCD_WIDTH + r.x];
        display_.draw_buffer(r.x, (uint16_t)(r.y + j), r.w, 1, p);
    }
}

void PacmanGame::render_full() {
    fb_fill(0, 0, LCD_WIDTH, LCD_HEIGHT, CLR_BG);
    if (state_ != State::ATTRACT) {
        for (int8_t ty = 0; ty < MAZE_H; ty++)
            for (int8_t tx = 0; tx < MAZE_W; tx++)
                draw_tile(tx, ty);
        if (state_ != State::CLEARING) {
            bool dying_gone = (state_ == State::DYING) && (state_ticks_ >= 130);
            if (!dying_gone) {
                int mouth = (state_ == State::DYING)
                            ? (int)((state_ticks_ / 6) & 3)
                            : (int)((tick_ / 5) & 3);
                draw_pacman(center_px_x(pacman_), center_px_y(pacman_),
                            next_dx_, next_dy_, mouth);
            }
            for (uint8_t i = 0; i < GHOST_COUNT; i++) draw_ghost(ghosts_[i]);
        }
        draw_overlay();
        draw_hud();
    } else {
        draw_attract();
    }
    Rect full = Rect{0, 0, LCD_WIDTH, LCD_HEIGHT};
    dirty_count_ = 0;
    upload(full);
}

void PacmanGame::render_region(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    if (state_ == State::ATTRACT) { draw_attract(); return; }

    // muros y puntos que tocan el rectangulo
    if (x < MAZE_X + MAZE_W * TILE && x + w > MAZE_X &&
        y < MAZE_Y + MAZE_H * TILE && y + h > MAZE_Y) {
        int tx0 = (x > MAZE_X) ? (x - MAZE_X) / TILE : 0;
        int tx1 = (int)((x + w - 1 - MAZE_X) / TILE);
        int ty0 = (y > MAZE_Y) ? (y - MAZE_Y) / TILE : 0;
        int ty1 = (int)((y + h - 1 - MAZE_Y) / TILE);
        if (tx0 < 0) tx0 = 0;
        if (ty0 < 0) ty0 = 0;
        if (tx1 > MAZE_W - 1) tx1 = MAZE_W - 1;
        if (ty1 > MAZE_H - 1) ty1 = MAZE_H - 1;
        for (int ty = ty0; ty <= ty1; ty++)
            for (int tx = tx0; tx <= tx1; tx++)
                draw_tile((int8_t)tx, (int8_t)ty);
    }

    if (state_ != State::CLEARING) {
        bool dying_gone = (state_ == State::DYING) && (state_ticks_ >= 130);
        if (!dying_gone) {
            int mouth = (state_ == State::DYING)
                        ? (int)((state_ticks_ / 6) & 3)
                        : (int)((tick_ / 5) & 3);
            draw_pacman(center_px_x(pacman_), center_px_y(pacman_),
                        next_dx_, next_dy_, mouth);
        }
        for (uint8_t i = 0; i < GHOST_COUNT; i++) draw_ghost(ghosts_[i]);
    }
    draw_overlay();
    if (x + w > HUD_X) draw_hud();
}

// ----------------------------------------------------------------------
// Primitivas sobre el back-buffer
// ----------------------------------------------------------------------
void PacmanGame::fb_pixel(int16_t x, int16_t y, uint16_t color) {
    if (x < 0 || y < 0 || x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    fb_[(size_t)y * LCD_WIDTH + x] = color;
}

void PacmanGame::fb_fill(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    if (x + w > LCD_WIDTH) w = LCD_WIDTH - x;
    if (y + h > LCD_HEIGHT) h = LCD_HEIGHT - y;
    for (uint16_t j = 0; j < h; j++) {
        uint16_t* row = &fb_[(size_t)(y + j) * LCD_WIDTH + x];
        for (uint16_t i = 0; i < w; i++) row[i] = color;
    }
}

void PacmanGame::draw_wall_tile(int16_t sx, int16_t sy, int8_t tx, int8_t ty) {
    bool flash = (state_ == State::CLEARING) && ((state_ticks_ / 25) & 1);
    uint16_t edge = flash ? CLR_WALL_FLASH : CLR_WALL_EDGE;
    fb_fill((uint16_t)sx, (uint16_t)sy, 8, 8, CLR_WALL_FILL);

    // borde brillante en los lados que dan a un espacio abierto
    auto is_wall = [&](int8_t x, int8_t y) {
        return (x >= 0 && x < MAZE_W && y >= 0 && y < MAZE_H) && maze_[y][x] == 3;
    };
    if (!is_wall(tx, (int8_t)(ty - 1))) fb_fill((uint16_t)sx, (uint16_t)sy, 8, 2, edge);
    if (!is_wall(tx, (int8_t)(ty + 1))) fb_fill((uint16_t)sx, (uint16_t)(sy + 6), 8, 2, edge);
    if (!is_wall((int8_t)(tx - 1), ty)) fb_fill((uint16_t)sx, (uint16_t)sy, 2, 8, edge);
    if (!is_wall((int8_t)(tx + 1), ty)) fb_fill((uint16_t)(sx + 6), (uint16_t)sy, 2, 8, edge);
}

void PacmanGame::draw_tile(int8_t tx, int8_t ty) {
    int16_t sx = MAZE_X + tx * TILE;
    int16_t sy = MAZE_Y + ty * TILE;
    uint8_t t = maze_[ty][tx];

    if (t == 3) { draw_wall_tile(sx, sy, tx, ty); return; }

    fb_fill((uint16_t)sx, (uint16_t)sy, 8, 8, CLR_BG);

    if (t == 4) { // boca de la casa
        fb_fill((uint16_t)sx, (uint16_t)(sy + 3), 8, 1, CLR_DOOR);
        fb_fill((uint16_t)sx, (uint16_t)(sy + 4), 8, 1, CLR_DOOR);
        return;
    }
    if (t == 1) { // punto
        fb_fill((uint16_t)(sx + 3), (uint16_t)(sy + 3), 2, 2, CLR_DOT);
        return;
    }
    if (t == 2) { // pastilla que parpadea
        if (((tick_ / 25) & 1) == 0) {
            fb_fill((uint16_t)(sx + 1), (uint16_t)(sy + 2), 6, 4, CLR_PELLET);
            fb_fill((uint16_t)(sx + 2), (uint16_t)(sy + 1), 4, 6, CLR_PELLET);
        }
    }
}

void PacmanGame::draw_pacman(int16_t cx, int16_t cy, int8_t dx, int8_t dy, int mouth) {
    static const uint16_t cos2x256[4] = {256, 210, 128, 210};
    uint16_t c2 = cos2x256[mouth & 3];
    int ux = (dx == 0 && dy == 0) ? 1 : dx;   // si esta quieto, boca hacia delante
    int uy = dy;
    if (ux == 0 && uy == 0) { ux = 1; uy = 0; }

    for (int py = -4; py <= 3; py++) {
        for (int px = -4; px <= 3; px++) {
            int32_t d2 = px * px + py * py;
            if (d2 > 14) continue;
            if (c2 < 256) {
                int32_t dot = px * ux + py * uy;
                if (dot > 0 && dot * dot > ((int32_t)c2 * d2) / 256) continue;
            }
            fb_pixel((int16_t)(cx + px), (int16_t)(cy + py), CLR_PAC);
        }
    }
}

void PacmanGame::draw_ghost(const Ghost& g) {
    const Ent& e = g.e;
    int16_t cx = center_px_x(e);
    int16_t cy = center_px_y(e);
    int16_t ox = (int16_t)(cx - 4);
    int16_t oy = (int16_t)(cy - 4);
    bool eyes_only = (g.state == GS_EATEN);

    uint16_t body = GHOST_COLORS[g.id];
    if (g.fright) {
        bool flash = (fright_ticks_ < 150) && ((fright_anim_ / 4) & 1);
        body = flash ? CLR_FRIGHT2 : CLR_FRIGHT;
    }

    if (!eyes_only) {
        // cuerpo: domo + cuerpo recto + zig-zag inferior
        auto put = [&](int px, int py) {
            fb_pixel((int16_t)(ox + px), (int16_t)(oy + py), body);
        };
        for (int px = 2; px <= 5; px++) put(px, 0);
        for (int px = 1; px <= 6; px++) put(px, 1);
        for (int py = 2; py <= 6; py++) for (int px = 0; px <= 7; px++) put(px, py);
        // pies
        put(0, 7); put(1, 7);
        put(3, 7); put(4, 7);
        put(6, 7); put(7, 7);
    }

    if (g.fright) {
        // ojos y boca temblorosa del modo asustado
        fb_pixel((int16_t)(ox + 2), (int16_t)(oy + 3), CLR_EYE);
        fb_pixel((int16_t)(ox + 5), (int16_t)(oy + 3), CLR_EYE);
        fb_pixel((int16_t)(ox + 1), (int16_t)(oy + 6), CLR_EYE);
        fb_pixel((int16_t)(ox + 3), (int16_t)(oy + 6), CLR_EYE);
        fb_pixel((int16_t)(ox + 5), (int16_t)(oy + 6), CLR_EYE);
        fright_anim_++;
    } else {
        // ojos blancos con pupila segun la direccion
        int fx = e.dx, fy = e.dy;
        if (fx == 0 && fy == 0) { fx = -1; fy = 0; }
        for (int py = 2; py <= 3; py++) {
            for (int px = 2; px <= 3; px++)
                fb_pixel((int16_t)(ox + px), (int16_t)(oy + py), CLR_EYE);
            for (int px = 5; px <= 6; px++)
                fb_pixel((int16_t)(ox + px), (int16_t)(oy + py), CLR_EYE);
        }
        fb_pixel((int16_t)(ox + 2 + fx), (int16_t)(oy + 2 + fy), CLR_PUPIL);
        fb_pixel((int16_t)(ox + 5 + fx), (int16_t)(oy + 2 + fy), CLR_PUPIL);
    }
}

// ----------------------------------------------------------------------
// Texto y HUD
// ----------------------------------------------------------------------
void PacmanGame::draw_text(int16_t x, int16_t y, const char* text,
                           uint16_t color, uint8_t scale) {
    for (const char* p = text; *p; p++) {
        uint16_t bits = glyph(*p);
        for (int row = 0; row < 5; row++) {
            uint16_t line = (bits >> (row * 3)) & 7;
            for (int col = 0; col < 3; col++) {
                if (line & (1 << col)) {
                    fb_fill((uint16_t)(x + col * scale), (uint16_t)(y + row * scale),
                            scale, scale, color);
                }
            }
        }
        x = (int16_t)(x + 4 * scale);
    }
}

void PacmanGame::draw_text_centered(int16_t y, const char* text,
                                    uint16_t color, uint8_t scale) {
    int16_t w = (int16_t)(strlen(text) * 4 * scale - scale);
    draw_text((int16_t)(MAZE_X + (MAZE_W * TILE - w) / 2), y, text, color, scale);
}

void PacmanGame::draw_hud() {
    fb_fill(HUD_X, 0, LCD_WIDTH - HUD_X, LCD_HEIGHT, CLR_BG);
    const int16_t x = HUD_X + 4;

    draw_text(x, 16, "1UP", CLR_TEXT, 2);
    char buf[12];
    snprintf(buf, sizeof(buf), "%lu", (unsigned long)score_);
    draw_text(x, 30, buf, YELLOW, 2);

    draw_text(x, 56, "HIGH", CLR_TEXT, 2);
    snprintf(buf, sizeof(buf), "%lu", (unsigned long)high_score_);
    draw_text(x, 70, buf, YELLOW, 2);

    draw_text(x, 100, "LEVEL", CLR_TEXT, 2);
    snprintf(buf, sizeof(buf), "%d", level_);
    draw_text(x, 114, buf, CYAN, 2);

    draw_text(x, 148, "LIVES", CLR_TEXT, 2);
    for (int16_t i = 0; i < lives_; i++) {
        draw_pacman((int16_t)(x + 4 + i * 14), 166, 1, 0, (int)((tick_ / 8) & 3));
    }

    draw_text(x, 196, "BOTONES", CLR_TEXT, 2);
    draw_text(x, 212, "8 ARRIBA",  0x8410, 1);
    draw_text(x, 222, "9 ABAJO",   0x8410, 1);
    draw_text(x, 232, "10 IZQUIER",0x8410, 1);
    draw_text(x, 242, "11 DERECHA",0x8410, 1);
    draw_text(x, 252, "7 INICIO/PAUSA", 0x8410, 1);
}

void PacmanGame::draw_overlay() {
    switch (state_) {
        case State::READY:
            draw_text_centered((int16_t)(MAZE_Y + 17 * TILE), "READY!", YELLOW, 2);
            break;
        case State::PAUSED:
            draw_text_centered((int16_t)(MAZE_Y + 15 * TILE), "PAUSA", WHITE, 3);
            draw_text_centered((int16_t)(MAZE_Y + 18 * TILE), "PULSA 7", CYAN, 2);
            break;
        case State::GAMEOVER:
            draw_text_centered((int16_t)(MAZE_Y + 14 * TILE), "GAME", RED, 3);
            draw_text_centered((int16_t)(MAZE_Y + 17 * TILE), "OVER", RED, 3);
            break;
        default:
            break;
    }
}

void PacmanGame::draw_attract() {
    // titulo
    const char* title = "PAC-MAN";
    int16_t w = (int16_t)(strlen(title) * 4 * 4 - 4);
    draw_text((int16_t)((LCD_WIDTH - w) / 2), 40, title, YELLOW, 4);

    const char* sub = "SSD1963 ARCADE";
    w = (int16_t)(strlen(sub) * 4 * 2 - 2);
    draw_text((int16_t)((LCD_WIDTH - w) / 2), 84, sub, CYAN, 2);

    // linea de persecucion (PAC-MAN + 4 fantasmas)
    int16_t base = (int16_t)(LCD_WIDTH - (attract_scroll_ % (LCD_WIDTH + 96)));
    draw_pacman((int16_t)(base + 4), 170, -1, 0, (int)((tick_ / 5) & 3));
    for (int i = 0; i < 4; i++) {
        int16_t ox = (int16_t)(base + 16 + i * 16);
        uint16_t c = GHOST_COLORS[i];
        for (int px = 2; px <= 5; px++) fb_pixel((int16_t)(ox + px), 166, c);
        for (int px = 1; px <= 6; px++) fb_pixel((int16_t)(ox + px), 167, c);
        for (int py = 168; py <= 174; py++)
            for (int px = 0; px <= 7; px++)
                fb_pixel((int16_t)(ox + px), (int16_t)py, c);
        // pies
        fb_pixel((int16_t)ox, 175, c);
        fb_pixel((int16_t)(ox + 1), 175, c);
        fb_pixel((int16_t)(ox + 3), 175, c);
        fb_pixel((int16_t)(ox + 4), 175, c);
        fb_pixel((int16_t)(ox + 6), 175, c);
        fb_pixel((int16_t)(ox + 7), 175, c);
        // ojos
        fb_pixel((int16_t)(ox + 2), 169, CLR_EYE);
        fb_pixel((int16_t)(ox + 5), 169, CLR_EYE);
        fb_pixel((int16_t)(ox + 3), 173, CLR_EYE);
        fb_pixel((int16_t)(ox + 4), 173, CLR_EYE);
    }

    // "PULSA 7 PARA JUGAR" parpadeante
    if (((tick_ / 30) & 1) == 0) {
        const char* msg = "PULSA 7 PARA JUGAR";
        w = (int16_t)(strlen(msg) * 4 * 2 - 2);
        draw_text((int16_t)((LCD_WIDTH - w) / 2), 212, msg, WHITE, 2);
    }
}

// ----------------------------------------------------------------------
// Frame publico
// ----------------------------------------------------------------------
void PacmanGame::frame() {
    read_input();
    update();
    render();
    tick_++;
}
