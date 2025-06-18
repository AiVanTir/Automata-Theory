CC      = gcc
LEX     = flex
YACC    = bison
CFLAGS  = -O2 -Wall -I.    # -I. позволяет подключать заголовки из текущей директории
LDFLAGS = -lfl                       # для совместимости с Flex (yywrap и т.п.)

# Список объектных файлов, которые будут в итоговом бинарнике
OBJS = lex.yy.o \
       robot_lang.tab.o \
       ast.o \
       interp.o \
       maze.o \
       main.o

# Целевая задача «all» строит robot_interpreter
all: robot_interpreter

# Правило линковки итогового исполняемого файла
robot_interpreter: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

# Bison генерирует parser: robot_lang.tab.c и robot_lang.tab.h
# Флаг -d говорит: «сгенерируй *.tab.h», -v — дополнительный отчёт.
robot_lang.tab.c robot_lang.tab.h: robot_lang.y ast.h
	$(YACC) -d -v -Wcounterexamples robot_lang.y

# генерирует лексер: lex.yy.c, но ему нужен robot_lang.tab.h
lex.yy.c: robot_lang.l robot_lang.tab.h
	$(LEX) robot_lang.l

# компиляция C-файлов в .o
#    Правило «%.o: %.c» охватывает все .c → .o в проекте.
%.o: %.c
	$(CC) $(CFLAGS) -c $<

# Когда собираем main.o, автоматически учитывается, что
# он #include "robot_lang.tab.h" → значит, добавляем зависимость.
main.o: main.c robot_lang.tab.h

# ------------------------------------------------
# Clean: удаляем всё, что Bison/Flex и компилятор создали
clean:
	rm -f *.o lex.yy.c robot_lang.tab.c robot_lang.tab.h robot_lang.output robot_interpreter

.PHONY: all clean

