SRCDIR=src
OBJDIR=obj
BINDIR=bin

GLOBAL_INCLUDE=../include

CC=gcc
CXX=g++

DEFINES=-D_DEBUG

CFLAGS=-I$(SRCDIR) -I$(GLOBAL_INCLUDE) -g -Wno-overflow $(DEFINES)

TARGET=$(BINDIR)/bgsc

RWILDCARD=$(foreach d,$(wildcard $(1:=/*)),$(call RWILDCARD,$d,$2) $(filter $(subst *,%,$2),$d))

# SRC=$(call RWILDCARD,$(SRCDIR),*.c)
# SRC+=$(call RWILDCARD,$(SRCDIR),*.cpp)
# OBJS=$(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.c.o, $(SRC))
# OBJS+=$(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.cxx.o, $(SRC))

Sources = 	\
			src/Language/IfElseStatement.cpp \
			src/Binaries/CompiledScript.cpp \
			src/Language/Expression.cpp \
			src/Language/Language.cpp \
			src/Language/Variable.cpp \
			src/Language/Function.cpp \
			src/Language/Token.cpp \
			src/Language/Field.cpp \
			src/Language/Loop.cpp \
			src/ArgumentParser.cpp \
			src/SourceToken.cpp \
			src/TokenParser.cpp \
			src/StringUtil.cpp \
			src/Logging.cpp \
			src/Source.cpp \
			src/main.cpp

OBJS=$(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.cxx.o, $(Sources))


all: $(OBJS) $(TARGET)


$(OBJDIR)/%.cxx.o: $(SRCDIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) -c $(CFLAGS) -o $@ $<

$(OBJDIR)/%.c.o: $(SRCDIR)/%.c
	@mkdir -p $(@D)
	$(CC) -c $(CFLAGS) -o $@ $<

$(TARGET): $(OBJS)
	@mkdir -p $(@D)
	$(CXX) -o $@ $^

.PHONY: clean
clean:
	@rm -rf $(OBJDIR) $(BINDIR)