#Output executable
OUTPUT_DIR=./bin/
OUTPUT_FILE=regex
EXECUTABLE=$(OUTPUT_DIR)$(OUTPUT_FILE)

#Directory information
SOURCE_DIR=src
OBJ_DIR=obj

INSTALL_EXE_PATH = /usr/bin/
INSTALL_LIB_PATH = /usr/lib/

#Compiler settings
CC=gcc
CFLAGS=-c -Wall
LDFLAGS=

#Rules to find source code - NOTE: Look for a better way to scan directories. Nonrecursive works but is a bit ugly
SOURCES=$(shell find src/ -type f -name '*.cpp') $(shell find src/ -type f -name '*.c')
OBJECTS=$(patsubst %.cpp,obj/%.o,$(SOURCES))

all: preprocess $(SOURCES) $(EXECUTABLE)

install:
	@cp $(EXECUTABLE) $(INSTALL_EXE_PATH)$(OUTPUT_FILE)

remove:
	@rm $(INSTALL_EXE_PATH)$(OUTPUT_FILE)

clean:
	-@rm -r $(OBJ_DIR) $(EXECUTABLE)

#The executable rule compiles the set of objects into the target executable
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

#These rule tells the compiler to generate an object from the source code.
$(OBJECTS) : $(OBJ_DIR)

$(OBJ_DIR):
	-@mkdir -p $@

$(OBJ_DIR)/%.o: %.cpp
	-@mkdir -p $(@D)
	$(CC) $(CFLAGS) $< -o $@

#The preprocess rules will update the build number
preprocess:
	-@mkdir -p $(OUTPUT_DIR)
