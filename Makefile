# Detect OS
UNAME_S := $(OS)

# Build dirs
BUILD_DIR := build
TARGET_APP := Pentelka

CMAKE := cmake
CMAKE_BUILD := cmake --build
RM_RF := rm -rf
MKDIR_P := mkdir -p
DEPLOY :=
QML_DIR := src/qml

# Windows adjustments
ifeq ($(UNAME_S),Windows_NT)
    TARGET_APP := Pentelka.exe
    RM_RF := rmdir /s /q
    MKDIR_P := mkdir
    DEPLOY := windeployqt6 --qmldir $(QML_DIR)
endif

.DEFAULT_GOAL := default

# ---------------------------------------------------------
# Build + deploy
# ---------------------------------------------------------

default: cmake_build
ifeq ($(OS),Windows_NT)
	@echo "Deploying Qt DLLs..."
	$(DEPLOY) $(BUILD_DIR)/$(TARGET_APP)
endif

run: default
ifeq ($(OS),Windows_NT)
	build\$(TARGET_APP)
else
	./build/$(TARGET_APP)
endif

# ---------------------------------------------------------
# Build using CMake + Ninja
# ---------------------------------------------------------

cmake_build:
	$(CMAKE) -S src -B $(BUILD_DIR) -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	$(CMAKE_BUILD) $(BUILD_DIR)

# ---------------------------------------------------------
# Cleaning
# ---------------------------------------------------------

clean: clean_build
ifeq ($(OS),Windows_NT)
	del /q *.zip 2>NUL
else
	rm -f *.zip
endif

clean_build:
ifeq ($(OS),Windows_NT)
	@if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)
	@mkdir $(BUILD_DIR)
	@if exist $(TARGET_APP) del /q $(TARGET_APP)
else
	@rm -rf $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR)
	@rm -f $(TARGET_APP)
endif
