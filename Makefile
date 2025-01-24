CXX = g++
CXX_FLAGS = -Wall -Wextra -Werror -std=c++17 -fPIC -pthread
LIB_FLAG = -llogger

SOURCE_DIR = src
BUILD_DIR = build
APP_DIR = src
LIBRARY_DIR = src/logger
MONITORING_DIR = src/monitoring
MULTITHREADING_DIR = src/multithreading
TEST_DIR = tests

LIBRARY_NAME = liblogger.so
APP_TARGET = app
TEST_TARGET = test

LIB_HEADERS = $(LIBRARY_DIR)/*.h
LIB_SOURCES = $(LIBRARY_DIR)/*.cpp
MONITORING_SOURCES = $(MONITORING_DIR)/*.cpp
MULTITHREADING_SOURCES = $(MULTITHREADING_DIR)/*.cpp
APP_SOURCES = $(APP_DIR)/main.cpp
TEST_SOURCES = $(TEST_DIR)/*.cpp

APP_BIN = $(BUILD_DIR)/$(APP_TARGET)
TEST_BIN = $(BUILD_DIR)/$(TEST_TARGET)
LIBRARIES = $(BUILD_DIR)/$(LIBRARY_NAME)

INSTALL_LIB_DIR = /usr/local/lib
INSTALL_INCLUDE_DIR = /usr/local/include/logger

.PHONY: all library test clean trash app install uninstall

all: trash library app test

app: trash
	$(CXX) $(CXX_FLAGS) $(APP_SOURCES) $(MONITORING_SOURCES) $(MULTITHREADING_SOURCES) -o $(APP_BIN) $(LIB_FLAG)

library: trash
	$(CXX) $(CXX_FLAGS) -shared $(LIB_SOURCES) -o $(LIBRARIES)

test: trash
	$(CXX) $(CXX_FLAGS) $(TEST_SOURCES) $(MONITORING_SOURCES) $(MULTITHREADING_SOURCES) -o $(TEST_BIN) $(LIB_FLAG)

install: library
	@sudo mkdir -p $(INSTALL_LIB_DIR)
	@sudo mkdir -p $(INSTALL_INCLUDE_DIR)
	@sudo cp $(LIBRARIES) $(INSTALL_LIB_DIR)
	@sudo cp $(LIB_HEADERS) $(INSTALL_INCLUDE_DIR)
	@sudo ldconfig
	@echo "Installed: $(LIBRARIES) to $(INSTALL_LIB_DIR)"
	@echo "Installed headers: $(LIB_HEADERS) to $(INSTALL_INCLUDE_DIR)"

uninstall:
	@sudo rm -f $(INSTALL_LIB_DIR)/$(LIBRARY_NAME)
	@sudo rm -rf $(INSTALL_INCLUDE_DIR)
	@sudo ldconfig
	@echo "Uninstalled: $(LIBRARY_NAME) from $(INSTALL_LIB_DIR)"
	@echo "Uninstalled headers from $(INSTALL_INCLUDE_DIR)"

trash:
	@mkdir -p $(BUILD_DIR)
	@echo "Created build dir"

clean:
	@rm -rf $(BUILD_DIR) *.txt
	@echo "Cleaned!"
