# Makefile - збірка, запуск і тести (тільки Windows, MinGW)
CFLAGS = -std=gnu99 -Wall -Wextra -Iinclude -O2

SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c,build/%.o,$(SRC))
HDR = $(wildcard include/*.h)
TARGET = roguelike.exe

TEST_SRC = $(wildcard tests/test_*.c)
TEST_BIN = $(patsubst tests/%.c,build/tests/%.exe,$(TEST_SRC))

.PHONY: all run test clean

all: $(TARGET)

# Гра: усі модулі з src/ і точка входу з demo/
$(TARGET): $(OBJ) build/main.o
	$(CC) $(CFLAGS) -o $@ $^

build/%.o: src/%.c $(HDR)
	@if not exist build mkdir build
	$(CC) $(CFLAGS) -c $< -o $@

build/main.o: demo/main.c $(HDR)
	@if not exist build mkdir build
	$(CC) $(CFLAGS) -c $< -o $@

# Кожен тест лінкується з модулями гри
build/tests/%.exe: tests/%.c $(OBJ) $(HDR)
	@if not exist build\tests mkdir build\tests
	$(CC) $(CFLAGS) -o $@ $< $(OBJ)

run: $(TARGET)
	.\$(TARGET)

# Запуск усіх тестів ланцюжком: a.exe && b.exe && ...
test: $(TEST_BIN)
	@$(foreach t,$(TEST_BIN),$(subst /,\,$(t)) &&) echo ALL TESTS PASSED

clean:
	-@if exist build rmdir /s /q build
	-@if exist $(TARGET) del /q $(TARGET)
