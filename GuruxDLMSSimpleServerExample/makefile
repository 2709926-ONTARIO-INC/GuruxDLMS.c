# project name (generate executable with this name)
TARGET   = gurux.dlms.simple.server.bin

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

$(BINDIR)/$(TARGET): $(OBJECTS)
	@$(LINKER) $@ $(LFLAGS) $(OBJECTS) -lgurux_dlms_c -lpthread -lm
	@echo "Linking complete!"

# Compile rule for src files
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

# Compile rule for cJSON files
$(OBJDIR)/%.o: $(CJSONDIR)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

.PHONEY: clean
clean:
	@$(rm) $(OBJECTS)
	@echo "Cleanup complete!" 
	@echo $(OBJECTS)

.PHONEY: remove
remove: clean
	@$(rm) $(BINDIR)/$(TARGET)
	@echo "Executable removed!"