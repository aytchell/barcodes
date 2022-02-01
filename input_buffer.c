#include "input_buffer.h"
#include <string.h>
#include <stdlib.h>

int buffer_new(int capacity, struct input_buffer *buffer)
{
    buffer->text = malloc(capacity);
    buffer->capacity = (buffer->text == NULL) ? 0 : capacity;
    buffer_clear(buffer);

    return buffer->text != NULL;
}

void buffer_delete(struct input_buffer *buffer)
{
    free(buffer->text);

    buffer->text = NULL;
    buffer->length = 0;
    buffer->capacity = 0;
}

int buffer_append(struct input_buffer *buffer, char input)
{
    if (buffer->capacity == 0 || buffer->text == NULL) 
        return -1;

    if (buffer->length >= (buffer->capacity - 1))
        return -1;

    buffer->text[buffer->length] = input;
    ++buffer->length;
    return 0;
}

void buffer_clear(struct input_buffer *buffer)
{
    memset(buffer->text, 0, buffer->capacity);
    buffer->length = 0;
}
