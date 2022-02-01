#ifndef INPUT_BUFFER_H
#define INPUT_BUFFER_H

struct input_buffer
{
    char* text;
    int capacity;
    int length;
};


int buffer_new(int capacity, struct input_buffer *buffer);

void buffer_delete(struct input_buffer *buffer);

int buffer_append(struct input_buffer *buffer, char input);

void buffer_clear(struct input_buffer *buffer);

#endif // INPUT_BUFFER_H
