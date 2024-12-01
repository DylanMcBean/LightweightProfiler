#
# Makefile for C++ Projects
#

MAKEFLAGS += --no-print-directory

# Directory Paths
SRC_DIR       = ../src
INC_DIR       = ../include
TESTS_IN_DIR  = ../tests/in
TESTS_OUT_DIR = ../tests/out
BUILD_DIR     = ../build/bin
RELEASE_DIR   = $(BUILD_DIR)/release
OBJ_DIR       = $(RELEASE_DIR)/obj
DEBUG_DIR     = $(BUILD_DIR)/debug
FUZZ_DIR      = $(BUILD_DIR)/fuzz
DEBUG_OBJ_DIR = $(DEBUG_DIR)/obj

# Compiler and Flags
CC            = clang++
AFL_CC        = afl-clang-fast++
CFLAGS        = -Wall -c -O3 -I$(INC_DIR) -std=c++20 -Wextra -Werror=return-type
LD_FLAGS      = -lm -lpthread
DEBUG_CFLAGS  = $(CFLAGS) -g -O0
FUZZ_CFLAGS   = -g -fsanitize=address,undefined -I$(INC_DIR) -std=c++20 -Wextra -Werror=return-type -O3 -fno-omit-frame-pointer
FUZZ_LD_FLAGS = $(LD_FLAGS) -fsanitize=address,undefined

ifdef PROFILING
CFLAGS       += -DPROFILING=1
DEBUG_CFLAGS += -DPROFILING=1
endif

# List of Source and Dependency Files
SRC_FILES := $(shell find $(SRC_DIR) -name "*.cpp")
DEP_FILES := $(wildcard $(INC_DIR)/*.hpp $(INC_DIR)/**/*.hpp)

# List of Object Files
OBJ_FILES       := $(patsubst $(SRC_DIR)/%, $(OBJ_DIR)/%, $(SRC_FILES:.cpp=.o))
DEBUG_OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp, $(DEBUG_OBJ_DIR)/%.o, $(SRC_FILES))
FUZZ_OBJ_FILES  := $(patsubst $(SRC_DIR)/%.cpp, $(FUZZ_DIR)/%.o, $(SRC_FILES))

# Main Target
all: $(RELEASE_DIR)/main

$(RELEASE_DIR)/main: $(OBJ_FILES)
	@mkdir -p $(RELEASE_DIR)
	$(CC) $(OBJ_FILES) $(LD_FLAGS) -o $@

# Debug Target
debug: $(DEBUG_DIR)/main

$(DEBUG_DIR)/main: $(DEBUG_OBJ_FILES)
	@mkdir -p $(DEBUG_DIR)
	$(CC) $(DEBUG_OBJ_FILES) $(LD_FLAGS) -o $@

# Fuzz Target
fuzz: $(FUZZ_DIR)/main

$(FUZZ_DIR)/main: $(FUZZ_OBJ_FILES)
	@mkdir -p $(FUZZ_DIR)
	$(AFL_CC) $(FUZZ_OBJ_FILES) $(FUZZ_LD_FLAGS) -o $@

# Compile Source Files
$(OBJ_FILES): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEP_FILES)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile Source Files with Debugging Enabled
$(DEBUG_OBJ_FILES): $(DEBUG_OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEP_FILES)
	@mkdir -p $(dir $@)
	$(CC) $(DEBUG_CFLAGS) -c $< -o $@

# Compile Source Files with Fuzzing Enabled
$(FUZZ_OBJ_FILES): $(FUZZ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEP_FILES)
	@mkdir -p $(dir $@)
	$(AFL_CC) $(FUZZ_CFLAGS) -c $< -o $@

# Generate Assembly Files
asm: $(DEBUG_OBJ_FILES:.o=.s)

$(DEBUG_OBJ_DIR)/%.s: $(DEBUG_OBJ_DIR)/%.o
	objdump -S $< > $@

# Clean Build Files
clean:
	rm -rf $(BUILD_DIR)

# Run Target
run:
	@if [ -f $(RELEASE_DIR)/main ]; then \
		echo -e "\033[1;37;44mRelease Mode\033[0m"; \
		$(MAKE) run_target RUN_TARGET=$(RELEASE_DIR)/main RUN_ARGS="$(ARGS)"; \
	elif [ -f $(DEBUG_DIR)/main ]; then \
		echo -e "\033[1;37;46mDebug Mode\033[0m"; \
		$(MAKE) run_target RUN_TARGET=$(DEBUG_DIR)/main RUN_ARGS="$(ARGS)"; \
	elif [ -f $(FUZZ_DIR)/main ]; then \
		echo -e "\033[1;37;45mFuzz Mode\033[0m"; \
		afl-fuzz -i $(TESTS_IN_DIR) -o $(TESTS_OUT_DIR) $(FUZZ_DIR)/main @@; \
	else \
		echo "No Builds found. Try 'make all', 'make debug', or 'make fuzz'."; \
	fi

# Run with Memory Check
run_target:
	@if [ "$(MEMCHECK)" = "leaks" ]; then \
		$(MAKE) memcheck_leaks RUN_TARGET=$(RUN_TARGET) RUN_ARGS="$(RUN_ARGS)"; \
	elif [ "$(MEMCHECK)" = "massif" ]; then \
		$(MAKE) memcheck_massif RUN_TARGET=$(RUN_TARGET) RUN_ARGS="$(RUN_ARGS)"; \
	elif [ "$(MEMCHECK)" = "cachegrind" ]; then \
		$(MAKE) memcheck_cachegrind RUN_TARGET=$(RUN_TARGET) RUN_ARGS="$(RUN_ARGS)"; \
	else \
		time $(RUN_TARGET) $(RUN_ARGS); \
	fi

# Memory Check Options
memcheck_leaks:
	@echo -e "\033[1;33mRunning valgrind (Leak Check) on $(RUN_TARGET)\033[0m"
	@command -v valgrind >/dev/null 2>&1 || { echo "Valgrind is not installed. Aborting."; exit 1; }
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s $(RUN_TARGET) $(RUN_ARGS)

memcheck_massif:
	@echo -e "\033[1;33mRunning valgrind (Massif - Memory Usage) on $(RUN_TARGET)\033[0m"
	@command -v valgrind >/dev/null 2>&1 || { echo "Valgrind is not installed. Aborting."; exit 1; }
	valgrind --tool=massif $(RUN_TARGET) $(RUN_ARGS)
	@echo -e "\033[1;33mMemory usage profile saved to massif.out.<pid>\033[0m"
	@echo "Use ms_print to analyze the profile."

memcheck_cachegrind:
	@echo -e "\033[1;33mRunning valgrind (Cachegrind - Cache Usage) on $(RUN_TARGET)\033[0m"
	@command -v valgrind >/dev/null 2>&1 || { echo "Valgrind is not installed. Aborting."; exit 1; }
	valgrind --tool=cachegrind $(RUN_TARGET) $(RUN_ARGS)
	@echo -e "\033[1;33mCache usage profile saved to cachegrind.out.<pid>\033[0m"
	@echo "Use cg_annotate to analyze the profile."

# Fuzz Testing with Monitoring
fuzztest:
	@sudo sh -c "echo core > /proc/sys/kernel/core_pattern"
	@sudo sh -c "echo performance | tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor"
	@if [ -f $(FUZZ_DIR)/main ]; then \
		afl-fuzz -D -i $(TESTS_IN_DIR) -o $(TESTS_OUT_DIR) -M fuzzer -- $(FUZZ_DIR)/main @@ & \
		nohup python3 -u monitor_fuzzing.py --output-dir $(TESTS_OUT_DIR)/fuzzer > monitor.log 2>&1 & \
		echo "Fuzzing and monitoring started in the background. Check monitor.log for output."; \
	else \
		echo "No Fuzz Build found. Run 'make fuzz' first."; \
	fi

# Dependency Generation
deps:
	$(CC) -MM $(SRC_FILES) -I$(INC_DIR) > .depend
-include .depend

# Default Target
.DEFAULT_GOAL := all

.PHONY: clean run asm fuzztest memcheck_leaks memcheck_massif memcheck_cachegrind