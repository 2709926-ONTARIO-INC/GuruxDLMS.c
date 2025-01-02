# Project name (generate executable with this name)
TARGET   = gurux.dlms.simple.server.bin

CC       = gcc

# Compiling flags here
CFLAGS   = -std=gnu99 -fstack-protector-all -Wall -I. -IcJSON/include

LINKER   = gcc -o

# Linking flags here
LFLAGS   = -L../development/lib 

# Change these to set the proper directories where each file should be
SRCDIR   = src
OBJDIR   = obj
BINDIR   = bin
CJSONDIR = cJSON

SOURCES  := $(wildcard $(SRCDIR)/*.c) $(wildcard $(CJSONDIR)/*.c)
INCLUDES := $(wildcard $(SRCDIR)/*.h) $(wildcard $(CJSONDIR)/include/*.h)
OBJECTS  := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(filter $(SRCDIR)/%, $(SOURCES))) \
            $(patsubst $(CJSONDIR)/%.c, $(OBJDIR)/%.o, $(filter $(CJSONDIR)/%, $(SOURCES)))
rm       = rm -f

# Compile rule for src files
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

# Compile rule for cJSON files
$(OBJDIR)/%.o: $(CJSONDIR)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

# Link the target
$(BINDIR)/$(TARGET): $(OBJECTS)
	@$(LINKER) $@ $(LFLAGS) $(OBJECTS) -lgurux_dlms_c -lpthread -lm
	@echo "Linking complete!"

# Build targets for single-phase and three-phase
single_phase: CFLAGS += -DSINGLE_PHASE
single_phase: $(BINDIR)/$(TARGET)

three_phase: CFLAGS += -DTHREE_PHASE
three_phase: $(BINDIR)/$(TARGET)

# Cleanup rules
.PHONY: clean
clean:
	@$(rm) $(OBJECTS)
	@echo "Cleanup complete!" 
	@echo $(OBJECTS)

.PHONY: remove
remove: clean
	@$(rm) $(BINDIR)/$(TARGET)
	@echo "Executable removed!"