SRCDIR=src
INCDIR=include
OBJDIR=obj
BINDIR=bin

GLOBAL_INCLUDE=../include

CC=gcc

DEFINES=-D_DEBUG

CFLAGS=-I$(INCDIR) -I$(GLOBAL_INCLUDE) -Wno-overflow -g $(DEFINES)

TARGET=$(BINDIR)/bgs

Sources = 	\
			src/baranium/runtime.c \
			src/baranium/bcpu.c \
			src/test.c

OBJS=$(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.c.o, $(Sources))

all: $(OBJS) $(TARGET)


$(OBJDIR)/%.c.o: $(SRCDIR)/%.c
	@mkdir -p $(@D)
	$(CC) -O3 -o $@ $< $(CFLAGS) -c

$(TARGET): $(OBJS)
	@mkdir -p $(@D)
	$(CC) -O3 -o $@ $^ $(CFLAGS)

.PHONY: clean
clean:
	@rm -rf $(OBJDIR) $(BINDIR)