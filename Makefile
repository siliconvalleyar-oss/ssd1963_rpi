CXX       = g++
CXXFLAGS  = -Iinclude -I/tmp/bcm2835-install/include -std=c++11 -Wall -Wextra -O2
VERSION  := $(shell git describe --tags --abbrev=0 2>/dev/null || echo "0.0.0")
CXXFLAGS += -DVERSION_TAG=\"$(VERSION)\"
LDFLAGS   = -L/tmp/bcm2835-install/lib -lbcm2835 -lm

SRC_DIR   = src
OBJ_DIR   = obj
BIN_DIR   = bin

# Proyecto driver limpio: ssd1963.cpp es el driver avanzado (se mantiene),
# main.cpp es el demo unificado (merge de los snapshots de tmp/).
SRCS      = $(SRC_DIR)/main.cpp \
            $(SRC_DIR)/ssd1963.cpp \
            $(SRC_DIR)/tftgl_ads7843.cpp

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
