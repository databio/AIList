BIN = bin
OBJ = obj
VPATH = src
LIB = AIList.o
OBJS = $(addprefix $(OBJ)/, $(LIB))

$(OBJ)/%.o: %.c
	cc -c $(CFLAGS) $< -o $@ 

cbs_r: $(OBJS)
	cc -o $(BIN)/ailist $(OBJS)

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
