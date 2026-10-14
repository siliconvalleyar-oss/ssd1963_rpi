# =============================================================================
# Unified SSD1963 TFT driver for Raspberry Pi (merged from tmp/ snapshots)
#
# Merge source: v7, v8, v9 (C++ / bcm2835) + legacy ssd1963_drv (C / EGL).
# Base: v9 - latest, clean class-based split (16-bit parallel bus).
# Merged-in:
#   - color palette finalized from v7/v8/v9 experiments (color_rgb565.hpp)
#   - color-bars full-screen test merged from v8 (testFullScreen)
#   - original C EGL driver preserved under legacy/ssd1963_c_driver/ (reference)
# =============================================================================
cmake_minimum_required(VERSION 3.10)

project(ssd1963_rpi)

# ================= CONFIG =================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Carpetas
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(INC_DIR ${CMAKE_SOURCE_DIR}/include)
set(BIN_DIR ${CMAKE_SOURCE_DIR}/bin)
set(OBJ_DIR ${CMAKE_SOURCE_DIR}/obj)

# Crear carpetas si no existen
file(MAKE_DIRECTORY ${BIN_DIR})
file(MAKE_DIRECTORY ${OBJ_DIR})

# ================= OUTPUT =================
# Ejecutable en /bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})

# Objetos en /obj
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OBJ_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OBJ_DIR})

# (Truco para forzar objetos en obj/)
set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)
set(CMAKE_OBJECT_OUTPUT_DIRECTORY ${OBJ_DIR})

# ================= INCLUDE =================
include_directories(${INC_DIR})

# ================= SOURCES =================
# Solo *.cpp del src/ del proyecto C++ unificado.
# (legacy/ no se compila: es el driver C/EGL original de referencia.)
file(GLOB SRC_FILES ${SRC_DIR}/*.cpp)

# ================= EXECUTABLE =================
add_executable(ssd1963 ${SRC_FILES})

# ================= LIBRARIAS =================
target_link_libraries(ssd1963 bcm2835)

# ================= FLAGS =================
target_compile_options(ssd1963 PRIVATE
    -Wall
    -Wextra
    -O2
)
