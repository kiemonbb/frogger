OBJS = main.c

OBJ_NAME = frogger

all:$(OBJS)
	gcc $(OBJS) -lncurses -o $(OBJ_NAME)