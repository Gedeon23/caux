CC = clang
SRC = main.c
TARGET = caux

$(TARGET): $(SRC)
	$(CC) -o $(TARGET) $(SRC)
