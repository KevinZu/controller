include make.in


SDK_DIR =./serial
TCP_DIR =./tcp_comm
PROTO_DIR = ./protocol
CFG_DIR = ./config
SCHE_DIR = ./sys_sche
SDL_DIR = ./SDL
CAN_DIR = ./can


LIBS = $(SDK_DIR)/lib/libserial.a
LIBS += $(TCP_DIR)/lib/libtcp.a
LIBS += $(PROTO_DIR)/lib/libproto.a
LIBS += $(CFG_DIR)/lib/libcfg.a
LIBS += $(SCHE_DIR)/lib/libsche.a
LIBS += $(SDL_DIR)/lib/libsdl.a
LIBS += $(CAN_DIR)/lib/libcan.a

INC =  -I$(SDK_DIR)/inc
INC += -I$(TCP_DIR)/inc
INC += -I$(PROTO_DIR)/inc
INC += -I$(CFG_DIR)/inc
INC += -I$(SCHE_DIR)/inc
INC += -I$(CAN_DIR)/inc
INC += -I$(SDL_DIR)/include
INC += -I$(SDL_DIR)/src
INC += -I$(SDL_DIR)/src/thread
INC += -I$(SDL_DIR)/src/timer
INC += -I$(SDL_DIR)/src/thread/pthread
INC += -I./inc


OBJS =  main.o 
OUTPUT = main.out
all:$(OUTPUT)
#----------------------------------------
$(OUTPUT): $(OBJS) $(LIBS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)
$(LIBS):
	@make -C $(SDL_DIR) --no-print-directory;
	@make -C $(SDK_DIR) --no-print-directory;
	@make -C $(TCP_DIR) --no-print-directory;
	@make -C $(PROTO_DIR) --no-print-directory;
	@make -C $(CFG_DIR) --no-print-directory;
	@make -C $(SCHE_DIR) --no-print-directory;
	@make -C $(CAN_DIR) --no-print-directory;
#$(OBJDIR)/%.o: $(SRCDIR)/%.c
#	$(CC) $(CFLAGS) -c $< -o $@ -I$./telsdk/inc
$(OBJS):main.c
	$(CC) $(CFLAGS) -c $< -o $@ $(INC) #-I./serial/inc
#----------------------------------------

clean:
	@make -C $(SDK_DIR) clean --no-print-directory;
	@make -C $(TCP_DIR) clean --no-print-directory;
	@make -C $(PROTO_DIR) clean --no-print-directory;
	@make -C $(CFG_DIR) clean --no-print-directory;
	@make -C $(SCHE_DIR) clean --no-print-directory;
	@make -C $(SDL_DIR) clean --no-print-directory;
	@make -C $(CAN_DIR) clean --no-print-directory;
	@rm -f main.out;
	@rm -f main.o;
	@echo "Make clean done"
