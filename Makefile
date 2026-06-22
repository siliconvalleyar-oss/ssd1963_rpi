CXX       = g++
CXXFLAGS  = -Iinclude -std=c++11 -Wall -Wextra -O2
LDFLAGS   = -lbcm2835 -lm

SRC_DIR   = src
ENG_DIR   = src/engine
OBJ_DIR   = obj
BIN_DIR   = bin

SRCS      = $(SRC_DIR)/main.cpp \
            $(SRC_DIR)/ssd1963.cpp \
            $(ENG_DIR)/GameEngine.cpp \
            $(ENG_DIR)/MenuScene.cpp \
            $(ENG_DIR)/Sprite.cpp \
            $(ENG_DIR)/SpaceShooterScene.cpp \
            $(ENG_DIR)/FontDemoScene.cpp \
            $(ENG_DIR)/ImageViewerScene.cpp \
            $(ENG_DIR)/ColorBlocksScene.cpp \
            $(ENG_DIR)/PatternScene.cpp

OBJS      = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(notdir $(SRCS)))
TARGET    = $(BIN_DIR)/ssd1963_app

PI_HOST  ?= pi@raspberry.local
PI_DIR   ?= /home/pi/src/ssd1963_photo_png_2026_sucess_rgb

$(shell mkdir -p $(OBJ_DIR) $(BIN_DIR))

.PHONY: all clean run help install-deps remote-build remote-run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(ENG_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)/*.o $(TARGET)

run: $(TARGET)
	sudo ./$(TARGET)

install-deps:
	bash script_tools_ssd1963_rpi/install_deps.sh

remote-build:
	ssh $(PI_HOST) "cd $(PI_DIR) && make -j4"

remote-run:
	ssh $(PI_HOST) "cd $(PI_DIR) && make run"

help:
	@echo "ARCADE PHOTO — Makefile"
	@echo "  all           Compilar"
	@echo "  run           Compilar + ejecutar (sudo)"
	@echo "  clean         Limpiar"
	@echo "  install-deps  Instalar libbcm2835"
	@echo "  remote-build  SSH a Pi y compilar"
	@echo "  remote-run    SSH, compilar y ejecutar"
