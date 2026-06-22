#pragma once

#include <engine/Scene.hpp>
#include <cstdio>

/*! \class ImageViewerScene
 *  \brief Muestra una imagen RGB565 a pantalla completa.
 *
 *  Carga un archivo .rgb565 desde el sistema de archivos
 *  y lo renderiza en el display. Soporta múltiples imágenes. */
class ImageViewerScene : public Scene {
public:
    /*! \brief Constructor.
     *  \param display Referencia al SSD1963.
     *  \param filepath Ruta al archivo .rgb565. */
    explicit ImageViewerScene(SSD1963& display, const char* filepath = "assets/capibaras.rgb565");

    bool on_enter() override;
    void update(uint32_t dt) override;
    void draw() override;
    uint8_t handle_button(uint8_t btn) override;
    const char* name() const override;

    /*! \brief Cambia la imagen a mostrar.
     *  \param filepath Nueva ruta de archivo. */
    void set_filepath(const char* filepath);

private:
    const char* m_filepath;        ///< Ruta del archivo de imagen
    bool        m_image_loaded;    ///< Flag de carga exitosa
    uint32_t    m_display_timer;   ///< Temporizador de visualización
};
