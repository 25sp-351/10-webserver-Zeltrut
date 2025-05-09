CC = gcc
CFLAGS = -Wall -Wextra -g -pthread
SRC_DIR = .
OBJ_DIR = obj

SRC_FILES = $(SRC_DIR)/server.c $(SRC_DIR)/request.c $(SRC_DIR)/response.c
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))

TARGET = http_server

all: $(TARGET)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ_FILES)
	$(CC) $(CFLAGS) $(OBJ_FILES) -o $(TARGET)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)
