CC = gcc.exe
CFLAGS = -Wall -Werror -Wshadow
TARGET = WerFault_catcher.exe
SRC = main.c common/functions.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS)

clean:
	del $(TARGET)


