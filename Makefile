# Makefile — збірка, запуск та тести проєкту roguelike
# Працює і в Linux/macOS, і в Windows (MinGW: mingw32-make / make у cmd).
CC      ?= gcc
CFLAGS  = -std=gnu99 -Wall -Wextra -Iinclude -O2

SRC     = $(wildcard src/*.c)
OBJ     = $(patsubst src/%.c,build/%.o,$(SRC))
HDR     = $(wildcard include/*.h)
TARGET  = roguelike

# ---- Різні команди для Windows (cmd) та Unix (sh) ----
ifeq ($(OS),Windows_NT)
  EXE       = .exe
  # "if not exist" — аналог mkdir -p для cmd; слеші змінюємо на зворотні
  MKDIR     = if not exist $(subst /,\,$(1)) mkdir $(subst /,\,$(1))
  CLEAN_CMD = if exist build rmdir /s /q build & if exist $(TARGET).exe del /q $(TARGET).exe
  RUN_CMD   = .\$(TARGET).exe
  # cmd не має циклу for як у sh, тому будуємо ланцюжок "a && b && c"
  TEST_CMD  = $(foreach t,$(TEST_BIN),$(subst /,\,$(t)) &&) echo ALL TESTS PASSED
else
  EXE       =
  MKDIR     = mkdir -p $(1)
  CLEAN_CMD = rm -rf build $(TARGET)
  RUN_CMD   = ./$(TARGET)
  TEST_CMD  = for t in $(TEST_BIN); do echo "== $$t"; ./$$t || exit 1; done; echo ALL TESTS PASSED
endif

# Кожен tests/test_*.c — окрема програма з assert-ами
TEST_SRC = $(wildcard tests/test_*.c)
TEST_BIN = $(patsubst tests/%.c,build/tests/%$(EXE),$(TEST_SRC))

.PHONY: all run test clean

all: $(TARGET)$(EXE)

# Лінкуємо гру: усі модулі з src/ + точка входу з demo/
$(TARGET)$(EXE): $(OBJ) build/main.o
	$(CC) $(CFLAGS) -o $@ $^

build/%.o: src/%.c $(HDR)
	@$(call MKDIR,build)
	$(CC) $(CFLAGS) -c $< -o $@

build/main.o: demo/main.c $(HDR)
	@$(call MKDIR,build)
	$(CC) $(CFLAGS) -c $< -o $@

# Тести лінкуються з тими самими модулями гри (без main.o)
build/tests/%$(EXE): tests/%.c $(OBJ) $(HDR)
	@$(call MKDIR,build/tests)
	$(CC) $(CFLAGS) -o $@ $< $(OBJ)

run: $(TARGET)$(EXE)
	$(RUN_CMD)

test: $(TEST_BIN)
	@$(TEST_CMD)

clean:
	-@$(CLEAN_CMD)
