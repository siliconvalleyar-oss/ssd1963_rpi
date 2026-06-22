#pragma once
#include <engine/Scene.hpp>
#include <engine/Sprite.hpp>

#define SPRITE_VIEWER_MAX 32

class SpriteViewerScene : public Scene {
public:
    explicit SpriteViewerScene(SSD1963& display);

    bool add_sprite(const char* filepath, const char* label);
    bool on_enter() override;
    void update(uint32_t dt) override;
    void draw() override;
    const char* name() const override;

private:
    struct Entry {
        Sprite  sprite;
        const char* label;
        bool    loaded;
    };
    Entry    m_entries[SPRITE_VIEWER_MAX];
    uint8_t  m_count;
    uint8_t  m_idx;
    uint32_t m_timer;
    bool     m_drawn;

    static constexpr uint32_t SPRITE_MS = 3000;
};
