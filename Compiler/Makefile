SRCDIR=src
OBJDIR=obj
BINDIR=bin

GLOBAL_INCLUDE=../include

CC=gcc
CXX=g++

DEFINES=-D_DEBUG

CFLAGS=-I$(SRCDIR) -I$(GLOBAL_INCLUDE) -g -Wno-overflow $(DEFINES)

TARGET=$(BINDIR)/bgsc

Sources = 	\
			src/Language/IfElseStatement.cpp \
			src/Binaries/NameLookupTable.cpp \
			src/Binaries/CompiledScript.cpp \
			src/Language/Expression.cpp \
			src/Binaries/Compiler.cpp \
			src/Language/Language.cpp \
			src/Language/Variable.cpp \
			src/Language/Function.cpp \
			src/Language/Token.cpp \
			src/Language/Field.cpp \
			src/Language/Loop.cpp \
			src/ArgumentParser.cpp \
			src/Preprocessor.cpp \
			src/SourceToken.cpp \
			src/TokenParser.cpp \
			src/StringUtil.cpp \
			src/FileUtil.cpp \
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