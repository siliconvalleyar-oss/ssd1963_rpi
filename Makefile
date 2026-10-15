CXX       = g++
CXXFLAGS  = -Iinclude -std=c++11 -Wall -Wextra -O2
VERSION  := $(shell git describe --tags --abbrev=0 2>/dev/null || echo "0.0.0")
CXXFLAGS += -DVERSION_TAG=\"$(VERSION)\"
LDFLAGS   = -lbcm2835 -lm

SRC_DIR   = src
OBJ_DIR   = obj
BIN_DIR   = bin

# Juego principal: Pac-Man arcade
SRCS      = $(SRC_DIR)/main_pacman.cpp \
            $(SRC_DIR)/ssd1963.cpp     \
            $(SRC_DIR)/pacman.cpp

OBJS      = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(notdir $(SRCS)))
TARGET    = $(BIN_DIR)/pacman_game

PI_HOST ?= pi@raspi.local
PI_DIR   ?= /home/pi/src/ssd1963_rpi

$(shell mkdir -p $(OBJ_DIR) $(BIN_DIR))

.PHONY: all clean run help install-deps remote-build remote-run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)/*.o $(TARGET)

run: $(TARGET)
	sudo ./$(TARGET)

install-deps:
	bash script_tools_ssd1963_rpi/install_deps.sh

remote-build:
	ssh $(PI_HOST) "cd $(PI_DIR) && git pull && git branch && make clean && make -j4"

remote-run:
	ssh $(PI_HOST) "cd $(PI_DIR) && make run"

help:
	@echo "Targets:"
	@echo "  make all           - Build $(TARGET)"
	@echo "  make clean         - Remove build artifacts"
	@echo "  make run           - Build and run locally (with sudo)"
	@echo "  make install-deps  - Install libbcm2835 and tools"
	@echo "  make remote-build  - SSH to Pi and build (remoto)"
	@echo "  make remote-run    - SSH to Pi, build, and run"
	@echo "  make help          - Show this help"
