# ==============================================================================
# ----------------------------- libftpp Makefile -------------------------------
# ==============================================================================

# ==============================================================================
# Variables
# ==============================================================================

NAME        = libftpp.a

CXX         = c++
CXXFLAGS    = -Wall -Wextra -Werror -std=c++11

# Debugging flags (Uncomment or move to a dev rule if needed)
CXXFLAGS   += -g3 -O0

AR          = ar
ARFLAGS     = rcs

# ==============================================================================
# Sources and Objects
# ==============================================================================

# As you create your modules in their folders, add the .cpp files here.
# Example: SRCS = data_structures/data_buffer.cpp design_patterns/memento.cpp
SRCS        = 

OBJS        = $(SRCS:.cpp=.o)

# ==============================================================================
# Rules
# ==============================================================================

all: $(NAME)

# Create the static library using ar
$(NAME): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^
	@echo "=================="
	@echo "$(NAME) compiled successfully!"

# Compile .cpp files to .o object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	@echo "=================="
	@echo "Cleaning object files..."
	rm -f $(OBJS)
	@find . -name "*.dSYM" -delete -print
	@find . -name "*~" -delete -print

fclean: clean
	@echo "=================="
	@echo "Removing $(NAME)..."
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re