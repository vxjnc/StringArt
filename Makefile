CXX := g++

ifeq ($(PROFILE), 1)
    OPT_FLAGS := -g -fno-omit-frame-pointer
    MSG_MODE := "for PROFILING"
else
    OPT_FLAGS := 
    MSG_MODE := "for OPTIMIZATION"
endif

CXXFLAGS := -std=c++20 -O3 -march=native -mtune=native -Wall -Wextra -Wpedantic -I. -Iinclude $(OPT_FLAGS)
LDLIBS :=

TARGET := string_art
BUILD_DIR := build

SRCS := $(shell find . -name "*.cpp")
OBJS := $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(SRCS))
DEPS := $(OBJS:.o=.d)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "Linking $< to $(TARGET) $(MSG_MODE)"
	@$(CXX) $(LDFLAGS) $(OBJS) -o $@ $(LDLIBS)

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling $<"
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

clean:
	@echo "Removing build directory ($(BUILD_DIR)) and generated binary ($(TARGET))..."
	rm -rf $(BUILD_DIR) $(TARGET)