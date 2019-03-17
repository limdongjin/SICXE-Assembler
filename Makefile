SOURCES=main.c command.c command_mapping.c command_shell.c command_execute.c
TARGET=main.out

all: $(TARGET)

$(TARGET): $(SOURCES)
	gcc -std=gnu99 -W -Wall $(SOURCES) -o $(TARGET)

clean:
	rm -f $(TARGET)