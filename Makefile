CC = clang
INCLUDE_DIR=include
SRC_DIR=$(PWD)/src
CFLAGS=-I$(INCLUDE_DIR) -Wall -Wextra 
LD_FLAGS=-lmpv

SRCS=$(wildcard $(SRC_DIR)/*.c)
OBJS= $(notdir $(SRCS:.c=.o))

#test:
#	@echo $(SRC_DIR)
#	@echo $(CFLAGS)
#	@echo $(LD_FLAGS)
#	@echo $(SRCS)
#	@echo $(OBJS)
.PHONY: all
all: gift





%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -lmpv

.PHONY: gift
gift: $(OBJS)
	$(CC) $(CFLAGS) -o gift $(OBJS) $(LD_FLAGS)

.PHONY: clean
clean:
	rm -f $(OBJS) gift
