CC = gcc
LD = gcc
INC_DIR = include 
SRC_DIR = src
OBJ_DIR = objects
CFLAGS = -g -Wall -I$(INC_DIR)
LIBRARY_PATH = lib/
# set linker flags to indicate where the greetings library can be found and
# what it is called (remember naming conventions: libgreetings.so for shared
# libraries and libgreetings.a for static libraries)
LDFLAGS = -L$(LIBRARY_PATH) -lrequest
SRCS = $(SRC_DIR)/server.c
OBJS = $(OBJ_DIR)/server.o
SHARED = server
RM = /bin/rm


all:
	@echo "choose target: server | clean"


shared: $(SHARED)


# build executable, which is dynamically linked
$(SHARED): $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -o $(SHARED)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS)
        # delete dynamically linked executable if it exists
	@if [ -e $(SHARED) ]; then \
		$(RM) $(SHARED) ; \
	fi;

