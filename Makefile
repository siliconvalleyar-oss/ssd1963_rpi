# =========================================================================
# Makefile — SSD1963 Game Engine for Raspberry Pi
# =========================================================================
# Targets principales:
#   make all           → Compila bin/ssd1963_app
#   make run           → Compila + ejecuta con sudo
#   make clean         → Elimina objetos y binario
#   make remote-build  → SSH a PI_HOST y compila en la Pi
#   make remote-run    → SSH, compila y ejecuta en la Pi
#   make install-deps  → Instala libbcm2835 y herramientas
#   make help          → Muestra esta ayuda
# =========================================================================

CXX       = g++
CXXFLAGS  = -Iinclude -std=c++11 -Wall -Wextra -O2
LDFLAGS   = -lbcm2835 -lm

SRC_DIR   = src
ENG_DIR   = src/engine
OBJ_DIR   = obj
BIN_DIR   = bin

# Recopilar fuentes
SRCS      = $(SRC_DIR)/main.cpp \
            $(SRC_DIR)/ssd1963.cpp \
            $(ENG_DIR)/GameEngine.cpp \
            $(ENG_DIR)/MenuScene.cpp \
            $(ENG_DIR)/ColorBlocksScene.cpp \
            $(ENG_DIR)/ImageViewerScene.cpp \
            $(ENG_DIR)/PaintScene.cpp \
            $(ENG_DIR)/PatternScene.cpp

OBJS      = $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(notdir $(SRCS)))
TARGET    = $(BIN_DIR)/ssd1963_app

PI_HOST  ?= pi@raspberry.local
PI_DIR   ?= /home/pi/src/ssd1963_photo_png_2026_sucess_rgb

$(shell mkdir -p $(OBJ_DIR) $(BIN_DIR))

.PHONY: all clean run help install-deps remote-build remote-run

all: $(TARGET)

# =========================================================================
# Reglas de compilación
# =========================================================================
$(TARGET): $(OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS)

# Regla genérica para archivos en src/
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Regla genérica para archivos en src/engine/
$(OBJ_DIR)/%.o: $(ENG_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# =========================================================================
# Utilidades
# =========================================================================
clean:
	rm -rf $(OBJ_DIR)/*.o $(TARGET)
	@echo "[Make] Limpieza completa."

run: $(TARGET)
	sudo ./$(TARGET)

install-deps:
	bash script_tools_ssd1963_rpi/install_deps.sh

# =========================================================================
# Compilación remota vía SSH
# =========================================================================
remote-build:
	ssh $(PI_HOST) "cd $(PI_DIR) && make -j4"

remote-run:
	ssh $(PI_HOST) "cd $(PI_DIR) && make run"

# =========================================================================
# Ayuda
# =========================================================================
help:
	@echo "╔══════════════════════════════════════════════════════════╗"
	@echo "║        SSD1963 Game Engine — Makefile Help              ║"
	@echo "╠══════════════════════════════════════════════════════════╣"
	@echo "║  make all           → Compilar proyecto                 ║"
	@echo "║  make run           → Compilar + ejecutar (sudo)        ║"
	@echo "║  make clean         → Eliminar build artifacts          ║"
	@echo "║  make install-deps  → Instalar libbcm2835               ║"
	@echo "║  make remote-build  → SSH a la Pi y compilar            ║"
	@echo "║  make remote-run    → SSH, compilar y ejecutar          ║"
	@echo "║  make help          → Mostrar esta ayuda                ║"
	@echo "╚══════════════════════════════════════════════════════════╝"
	@echo ""
	@echo "Variables (override):"
	@echo "  PI_HOST  = $(PI_HOST)"
	@echo "  PI_DIR   = $(PI_DIR)"
	@echo "  CXX      = $(CXX)"
	@echo "  CXXFLAGS = $(CXXFLAGS)"
	@echo "  LDFLAGS  = $(LDFLAGS)"
