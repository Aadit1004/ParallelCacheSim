# compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2

# directories
SRC_DIR = src
BUILD_DIR = build

# executable name
TARGET = cache_sim

# source files
SRCS = $(SRC_DIR)/main.cpp $(SRC_DIR)/cli/arg_parser.cpp $(SRC_DIR)/cache/cache_config.cpp
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS)) # convert .cpp to .o in build/

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# rule to compile each .cpp file into an .o file inside build/
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ensures build directory exists before compilation
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR) $(BUILD_DIR)/cli $(BUILD_DIR)/cache

clean:
	rm -f $(TARGET) $(BUILD_DIR)/*.o $(BUILD_DIR)/cli/*.o $(BUILD_DIR)/cache/*.o