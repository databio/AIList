CFLAGS = -g -O2
BIN = bin
OBJ = obj
VPATH = src
LIB = AIList.o gsail.o
OBJS = $(addprefix $(OBJ)/, $(LIB))

$(OBJ)/%.o: %.c
	cc -c $(CFLAGS) $< -o $@ 

gsail: $(OBJS)
	cc -o $(BIN)/gsail $(OBJS) $(CFLAGS)

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
