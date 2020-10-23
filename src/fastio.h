typedef struct {
    char * data;
    int32_t pos;
    int32_t len;
    int32_t size;
    int fd;
} buffer_t;

int read_to_buffer_from_file(buffer_t * buffer) {
    while (buffer->len < buffer->size) {
        ssize_t cur_progress = read(buffer->fd, &buffer->data[buffer->len], buffer->size - buffer->len);
        if (cur_progress == 0) {
            return 0;
        }
        if (cur_progress == -1) {
            return -1;
        }
        buffer->len += cur_progress;
    }
    return 1;
}

int write_from_buffer_to_file(buffer_t * buffer) {
    while (buffer->pos < buffer->len) {
        ssize_t cur_progress = write(buffer->fd, &buffer->data[buffer->pos], buffer->len - buffer->pos);
        if (cur_progress == 0) {
            return 0;
        }
        if (cur_progress == -1) {
            return -1;
        }
        buffer->pos += cur_progress;
    }
    return 1;
}

int write_to_buffer(buffer_t * buffer, char * from, int32_t len) {
    if (buffer->len + len > buffer->size) {
        if (write_from_buffer_to_file(buffer) == -1) {
            return -1;
        }
        buffer->len = 0;
        buffer->pos = 0;
    }
    memcpy(&buffer->data[buffer->len], from, len);
    buffer->len += len;
    return 0;
}

