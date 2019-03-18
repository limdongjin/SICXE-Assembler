SOURCES=main.c command.c command_mapping.c command_shell.c command_execute.c history.c memory.c state.c util.c dir.c
TARGET=main.out

all: $(TARGET)

$(TARGET): $(SOURCES)
	gcc -std=gnu99 -W -Wall $(SOURCES) -o $(TARGET)

clean:
	rm -f $(TARGET)