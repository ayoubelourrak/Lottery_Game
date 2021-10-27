### MAKEFILE CONFIGURATION

# Compiler and flags
CC	= gcc
FLAGS	= -Wall -lpthread -std=gnu99

# Directories
OBJDIR     = build
SRCDIR     = SOURCE

# Object files
SHARED_OBJFILES = utility.o network.o
CLIENT_OBJFILES = client_main.o help.o request.o $(SHARED_OBJFILES)
SERVER_OBJFILES = server_main.o response.o estrazioni.o $(SHARED_OBJFILES)

# Binary files
CLIENT = lotto_client
SERVER = lotto_server 

### MAKEFILE EXECUTION

# Builds the executables: default rule
.PHONY: all
all : $(CLIENT) $(SERVER)

# Deletes objectfile and executables
.PHONY: clean
clean:
	rm -r $(OBJDIR)/* $(SERVER) $(CLIENT)

# Build generic .o file from .c file
$(OBJDIR)/%.o: $(SRCDIR)/*/%.c
	$(CC) $(FLAGS) -c $< -o $@

# Build CLIENT
$(CLIENT): $(addprefix $(OBJDIR)/,$(CLIENT_OBJFILES))
	${CC} -o $@ $^ $(FLAGS)

# Build SERVER
$(SERVER): $(addprefix $(OBJDIR)/,$(SERVER_OBJFILES))
	${CC} -o $@ $^ $(FLAGS)


