# Compilador y herramientas
CC = gcc
AR = ar

# Directorios
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
INC_DIR = include

# Archivos fuente y objeto
SRCS = main.c $(SRC_DIR)/tftgl.c
OBJS = $(OBJ_DIR)/main.o $(OBJ_DIR)/tftgl.o
TARGET = $(BIN_DIR)/tft_ssd1963_app

# Flags de compilación y enlace
CFLAGS = -I$(INC_DIR) -I/opt/vc/include
LDFLAGS = -L/opt/vc/lib -lbcm2835 -lEGL -lGLESv2

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Regla general para compilar .c a .o en obj/
$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(OBJ_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

$(OBJ_DIR)/tftgl.o: $(SRC_DIR)/tftgl.c $(INC_DIR)/tftgl.h
	@mkdir -p $(OBJ_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -rf $(OBJ_DIR)/*.o $(TARGET)
