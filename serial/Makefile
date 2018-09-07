#
include ../make.in
OBJDIR = $(shell pwd)/src
LIBDIR = ./lib

INC= -I./inc
INC += -I../inc
INC += -I../tcp_comm/inc
INC += -I../config/inc
INC += -I../protocol/inc
INC+=-I../SDL/include
INC+=-I../SDL/src
INC+=-I../SDL/src/thread
INC+=-I../SDL/src/timer
INC+=-I../SDL/src/thread/pthread
CFLAGS = -g -ldl -lm $(INC)
OUTPUT = $(LIBDIR)/libserial.a
OBJS =  serial.o uart_commx.o
OBJS := $(OBJS:%.o=$(OBJDIR)/%.o)
#----------------------------------------
all: $(OUTPUT)

clean:
	@rm -f $(OBJDIR)/*.o $(OUTPUT)

distclean:
	@rm -f *~

#install:
#	@cp ./lib/libtel.a ../PLUM_APP/lib

#deinstall:
#	@rm -f ../PLUM_APP/lib/libtel.a
#----------------------------------------
$(OUTPUT): $(OBJS)
	@$(AR) cr $@ $(OBJS)

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(INCS)
