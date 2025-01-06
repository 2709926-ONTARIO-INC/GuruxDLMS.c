# project name (generate executable with this name)
TARGET   = gurux.dlms.server.bin

CC       = gcc

# compiling flags here
CFLAGS   = -std=gnu99 -fstack-protector-all -Wall -I. -IcJSON/include

LINKER   = gcc -o

# linking flags here
LFLAGS   = -L../development/lib 

# change these to set the proper directories where each files should be

SRCDIR   = src
OBJDIR   = obj
BINDIR   = bin
CJSONDIR = cJSON

SOURCES  := $(wildcard $(SRCDIR)/*.c) $(wildcard $(CJSONDIR)/*.c)
INCLUDES := $(wildcard $(SRCDIR)/*.h) $(wildcard $(CJSONDIR)/include/*.h)
OBJECTS  := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(filter $(SRCDIR)/%, $(SOURCES))) \
            $(patsubst $(CJSONDIR)/%.c, $(OBJDIR)/%.o, $(filter $(CJSONDIR)/%, $(SOURCES)))
rm       = rm -f

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

$(OBJDIR)/%.o: $(CJSONDIR)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

$(BINDIR)/$(TARGET): $(OBJECTS)
	@$(LINKER) $@ $(LFLAGS) $(OBJECTS) -lgurux_dlms_c -lpthread -lm
	@echo "Linking complete!"

# Build targets for single-phase and three-phase
single_phase: CFLAGS += -DSINGLE_PHASE
single_phase: $(BINDIR)/$(TARGET)

three_phase: CFLAGS += -DTHREE_PHASE
three_phase: $(BINDIR)/$(TARGET)

.PHONEY: clean
clean:
	@$(rm) $(OBJECTS)
	@echo "Cleanup complete!" 
	@echo $(OBJECTS)

.PHONEY: remove
remove: clean
	@$(rm) $(BINDIR)/$(TARGET)
	@echo "Executable removed!"
