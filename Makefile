CFLAGS = -g -O2 -lz
BIN = bin
OBJ = obj
VPATH = src
LIB = AIList.o ailist_main.o
OBJS = $(addprefix $(OBJ)/, $(LIB))

$(OBJ)/%.o: %.c
	cc -c $(CFLAGS) $< -o $@ 

ailist: $(OBJS)
	cc -o $(BIN)/ailist $(OBJS) $(CFLAGS)

all: $(OBJS)

$(OBJS): | $(OBJ) $(BIN)

$(OBJ):
	mkdir -p $(OBJ)

$(BIN):
	mkdir -p $(BIN)

.PHONY: clean
clean:
	rm -rf $(BIN)/*
	rm -rf $(OBJ)/*
