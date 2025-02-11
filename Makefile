# compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2

# directories
SRC_DIR = src
BUILD_DIR = build

# executable name
TARGET = cache_sim

# source files
SRCS = $(SRC_DIR)/main.cpp
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS)) # Convert .cpp to .o in build/

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# rule to compile each .cpp file into an .o file inside build/
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ensures build directory exists before compilation
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -f $(TARGET) $(BUILD_DIR)/*.o