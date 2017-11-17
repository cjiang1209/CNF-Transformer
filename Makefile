EXEC = cnf
SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
DIR = /

CXX = g++
CFLAGS = -Wall
CFLAGS = -Wall

#COPTIMIZE = -O3

LEXER = flex
PARSER = bison

.PHONY: release debug profile help clean

# Target
release debug profile: $(EXEC)

# Compile options
release: CFLAGS += $(COPTIMIZE) -DNDEBUG -std=c++11
debug: CFLAGS += -O0 -g3 -std=c++11
profile: CFLAGS += $(COPTIMIZE) -pg -DNDEBUG -std=c++11
profile: LFLAGS += -pg

# Directory
release: DIR = Release/
debug: DIR = Debug/
profile: DIR = Profile/

# Build
%.o: %.cpp lexer.cpp parser.cpp
	@echo Compiling: $@
	@mkdir -p $(DIR)
	$(CXX) $(CFLAGS) -c $< -o $(addprefix $(DIR),$@)
	@echo ""

lexer.o: lexer.cpp
	@echo Compiling: $@
	@mkdir -p $(DIR)
	$(CXX) $(CFLAGS) -c $< -o $(addprefix $(DIR),$@)
	@echo ""

parser.o: parser.cpp
	@echo Compiling: $@
	@mkdir -p $(DIR)
	$(CXX) $(CFLAGS) -c $< -o $(addprefix $(DIR),$@)
	@echo ""

lexer.cpp: lexer.l parser.cpp
	@echo Lexer: $@
	$(LEXER) -o $@ $<
	@echo ""

parser.cpp: parser.y
	@echo Parser: $@
	$(PARSER) -d -o $@ $<
	@echo ""

# Link
$(EXEC): $(OBJS) lexer.o parser.o
	@echo Linking $^
	$(CXX) $(LFLAGS) $(addprefix $(DIR),$^) -o $(addprefix $(DIR),$@)
	@echo ""

# Clean
clean:
	rm -f lexer.cpp
	rm -f parser.hpp
	rm -f parser.cpp
	rm -f Release/*
	rm -f Debug/*
	rm -f Profile/*