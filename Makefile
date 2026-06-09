NAME := ircserv

SRCS := $(addprefix src/, \
	main.cpp \
	Config.cpp \
	Server.cpp \
	Client.cpp \
	LineBuffer.cpp \
	Error.cpp \
	net/Socket.cpp \
	net/tcp.cpp \
	net/Connection.cpp \
	io/FdHandle.cpp \
	io/Epoll.cpp \
	io/SignalFd.cpp \
	signals.cpp \
)
OBJ_DIR := obj
OBJS    := $(patsubst src/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# --- parseconf (libcfg.a) ---------------------------------------------------
CFG_DIR := lib/parseconf
CFG_LIB := $(CFG_DIR)/libcfg.a
CFG_INC := $(CFG_DIR)/inc
# Sources of the lib: used as prerequisites so libcfg.a is rebuilt when they change.
CFG_SRCS := $(wildcard $(CFG_DIR)/src/*.cpp $(CFG_DIR)/inc/*.hpp)

CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++98 -Iinc/ -I$(CFG_INC)

RM := rm -f
MAKEFLAGS   += --no-print-directory

all : $(NAME)

# Build the binary; depends on our objects and on the static lib.
$(NAME) : $(CFG_LIB) $(OBJS)
	$(CXX) $(OBJS) $(CXXFLAGS) $(CFG_LIB) -o $(NAME)
	$(info CREATED $(NAME))

# Build (or rebuild) the library by recursing into its own Makefile.
# Depends on the lib's sources/headers so a change there triggers a rebuild.
$(CFG_LIB) : $(CFG_SRCS)
	$(MAKE) -C $(CFG_DIR)

$(OBJ_DIR)/%.o : src/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@
	$(info CREATED $@)

clean :
	$(RM) -r $(OBJ_DIR)
	$(MAKE) -C $(CFG_DIR) clean
	$(info DELETED objects files)

fclean : clean
	$(RM) $(NAME)
	$(info DELETED $(NAME))

re :
	$(MAKE) fclean
	$(MAKE) all

# Rebuild everything with debug symbols and no optimization (for gdb/VS Code).
# The target-specific CXXFLAGS also applies to the prerequisites (fclean all),
# and fclean forces a full recompile so the new flags take effect.
debug : CXXFLAGS += -g3 -O0
debug : fclean all

# Run the server under valgrind (memory + file descriptor checks).
# Depends on debug so line numbers show up; the server blocks until you Ctrl+C,
# at which point valgrind prints its leak and fd report.
VG_FLAGS := --leak-check=full --show-leak-kinds=all --track-fds=yes

valgrind : debug
	valgrind $(VG_FLAGS) ./$(NAME)

# Regenerate compile_commands.json (for clangd/LSP) from a clean build.
# Optional dev tool: requires `bear`, not needed for a normal build.
compile_commands :
	$(MAKE) fclean
	bear -- $(MAKE)

info-%:
	$(MAKE) --dry-run --always-make $* | grep -v "info"

.PHONY : all clean fclean re debug valgrind compile_commands info-
.SILENT :
