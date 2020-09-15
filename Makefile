CC = gcc
LD = gcc
INC_DIR = db_server/include 
SRC_DIR = db_server/src
OBJ_DIR = db_server/objects
CFLAGS = -g -Wall -I$(INC_DIR)
LIBRARY_PATH = db_server/lib/
# set linker flags to indicate where the greetings library can be found and
# what it is called (remember naming conventions: libgreetings.so for shared
# libraries and libgreetings.a for static libraries)
LDFLAGS = -L$(LIBRARY_PATH) -lrequest
SRCS = $(SRC_DIR)/server.c
OBJS = $(OBJ_DIR)/server.o
SHARED = server
RM = /bin/rm


all:
	@echo "choose target: [static | shared]"


shared: $(SHARED)


# build executable, which is dynamically linked
$(SHARED): $(OBJS)
	# this needs to be manually written into each new console in order to make the program find librequest.so
	#export LD_LIBRARY_PATH=/home/seclab/Desktop/database/db_server/lib:$LD_LIBRARY_PATH
	$(LD) $(OBJS) $(LDFLAGS) -o $(SHARED)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS)
        # delete dynamically linked executable if it exists
	@if [ -e $(SHARED) ]; then \
		$(RM) $(SHARED) ; \
	fi;
        # delete statically linked executable if it exists
	@if [ -e $(STATIC) ]; then \
		$(RM) $(STATIC) ; \
	fi;
