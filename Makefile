NAME := ircserv

SRCS := $(addprefix src/, \
	main.cpp \
	Config.cpp \
)
OBJ_DIR := obj
OBJS    := $(patsubst src/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# --- parseconf (libcfg.a) ---------------------------------------------------
CFG_DIR := lib/parseconf
CFG_LIB := $(CFG_DIR)/libcfg.a
CFG_INC := $(CFG_DIR)/inc

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
$(CFG_LIB) :
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

info-%:
	$(MAKE) --dry-run --always-make $* | grep -v "info"

.PHONY : all clean fclean re info-
.SILENT :
