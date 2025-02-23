# compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2

# profiling flags
GPROF_FLAGS = -pg

# directories
SRC_DIR = src
BUILD_DIR = build
TEST_DIR = tests
BUILD_TEST_DIR = $(BUILD_DIR)/tests

# executable names
TARGET = cache_sim
TEST_TARGET = cache_test
PROF_TARGET = cache_sim_prof
TEST_PROF_TARGET = cache_test_prof

# source files
SRCS = $(SRC_DIR)/main.cpp $(SRC_DIR)/cli/arg_parser.cpp $(SRC_DIR)/cache/cache_config.cpp $(SRC_DIR)/cache/cache.cpp $(SRC_DIR)/memory/memory.cpp
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(filter-out $(SRC_DIR)/main.cpp, $(SRCS))) # exclude main.cpp for test build

# test files
TEST_SRCS = $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJS = $(patsubst $(TEST_DIR)/%.cpp, $(BUILD_TEST_DIR)/%.o, $(TEST_SRCS))

# default target compiles src and tests
all: $(TARGET) $(TEST_TARGET)

# build src executable
$(TARGET): $(OBJS) $(BUILD_DIR)/main.o
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(BUILD_DIR)/main.o

# build test executable (exclude main.o)
$(TEST_TARGET): $(TEST_OBJS) $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TEST_TARGET) $(TEST_OBJS) $(OBJS)

# rule to compile each .cpp file into an .o file inside build/
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# rule to compile test files
$(BUILD_TEST_DIR)/%.o: $(TEST_DIR)/%.cpp | $(BUILD_TEST_DIR)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I./catch2 -c $< -o $@

# ensures build directory exists before compilation
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR) $(BUILD_DIR)/cli $(BUILD_DIR)/cache $(BUILD_DIR)/memory

$(BUILD_TEST_DIR):
	mkdir -p $(BUILD_TEST_DIR)

clean:
	rm -f $(TARGET) $(TEST_TARGET) $(PROF_TARGET) $(TEST_PROF_TARGET) $(BUILD_DIR)/*.o $(BUILD_DIR)/cli/*.o $(BUILD_DIR)/cache/*.o $(BUILD_DIR)/memory/*.o $(BUILD_TEST_DIR)/*.o gmon.out

# run Clang-Tidy on all spp files under /src
TIDY_FLAGS = -checks='clang-analyzer-*,performance-*'

tidy:
	@clang-tidy $(TIDY_FLAGS) $(shell find $(SRC_DIR) -name '*.cpp') -fix-errors -- -std=c++17

# valgrind support
valgrind: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET) $(ARGS)

valgrind_test: $(TEST_TARGET)
	valgrind --leak-check=full --show-leak-kinds=all ./$(TEST_TARGET)

# profiling build targets
$(PROF_TARGET): $(OBJS) $(BUILD_DIR)/main.o
	$(CXX) $(CXXFLAGS) $(GPROF_FLAGS) -o $(PROF_TARGET) $(OBJS) $(BUILD_DIR)/main.o

$(TEST_PROF_TARGET): $(TEST_OBJS) $(OBJS)
	$(CXX) $(CXXFLAGS) $(GPROF_FLAGS) -o $(TEST_PROF_TARGET) $(TEST_OBJS) $(OBJS)

# gprof support
gprof-run: $(PROF_TARGET)
	rm -f gmon.out
	./$(PROF_TARGET) $(ARGS)
	gprof $(PROF_TARGET) gmon.out > gprof_report.txt
	cat gprof_report.txt

gprof-test-run: $(TEST_PROF_TARGET)
	rm -f gmon.out
	./$(TEST_PROF_TARGET)
	gprof $(TEST_PROF_TARGET) gmon.out > gprof_test_report.txt
	cat gprof_test_report.txt