TARGET=executavel
CC=g++
CURSES=-lncursesw
PTHREAD=-pthread
CCFLAGS=$(DEBUG) $(OPT) $(WARN)
LD=g++
OBJS=main.cpp
all: main.cpp
	$(LD) $(OBJS) -o $(TARGET) $(PTHREAD) $(CURSES)
