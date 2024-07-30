CC := clang
CFLAGS := -Wall -Wextra -std=c99 -g -fPIC -O3
LDFLAGS := -shared
LUA_INCLUDE := /opt/homebrew/Cellar/lua/5.4.7/include/lua
LUA_LIB := /opt/homebrew/Cellar/lua/5.4.7/lib

INSTALL_DIR := $(HOME)/.local/share/stankysock
TARGET := bin/stankysock.so
SRC := src/yabai_socket.c src/parser.c src/stankysock.c

.PHONY: all clean install uninstall test

all: $(TARGET)

$(TARGET): $(SRC) | bin
	$(CC) $(CFLAGS) -I$(LUA_INCLUDE) -L$(LUA_LIB) $^ -o $@ $(LDFLAGS) -llua

clean:
	rm -rf bin

bin:
	mkdir -p bin

install: $(TARGET)
	mkdir -p $(INSTALL_DIR)
	cp $(TARGET) $(INSTALL_DIR)
	@echo "Installed $(TARGET) to $(INSTALL_DIR)"

uninstall:
	rm -f $(INSTALL_DIR)/$(notdir $(TARGET))
	@echo "Uninstalled $(notdir $(TARGET)) from $(INSTALL_DIR)"

test: $(TARGET)
	lua test.lua
