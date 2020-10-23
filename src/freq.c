#include <time.h>

#include <stdlib.h>
#include <stdio.h>

#include <string.h>

#include <inttypes.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <unistd.h>

#include "constants.h"
#include "util.h"
#include "hashtable.h"
#include "fastio.h"

extern int32_t MOD1;

int feed_table(buffer_t * inp_env_buffer, struct hash_table_node * table) {
    char * word;
    int8_t opened = 0;
    int32_t word_len = 0;
    for (int32_t i = inp_env_buffer->pos; i < inp_env_buffer->len; ++i) {
        inp_env_buffer->data[i] = tolower(inp_env_buffer->data[i]);
        if (!opened) {
            if (isalpha(inp_env_buffer->data[i])) { 
                word = &inp_env_buffer->data[i];
                opened = 1;
                word_len = 1;
            }
        } else {
            if (!isalpha(inp_env_buffer->data[i])) {
                if (word_len > 0) {
                    if (add_to_hash_table(table, word, word_len) == -1) {
                        return -1;
                    }
                }
                opened = 0;
                word_len = 0;
            } else {
                ++word_len;
            }
        }
    }
    return 0;
}

int count_words(buffer_t * inp_env_buffer) {
    int8_t opened = 0;
    int32_t cnt = 0;
    for (int32_t i = inp_env_buffer->pos; i < inp_env_buffer->len; ++i) {
        if (!opened) {
            if (isalpha(inp_env_buffer->data[i])) { 
                opened = 1;
            }
        } else {
            if (!isalpha(inp_env_buffer->data[i])) {
                if (opened) {
                    ++cnt;
                }
                opened = 0;
            }
        }
    }
    return cnt;
}

int dump_table(struct hash_table_node * table, buffer_t * out) {
    // shift all empty buckets to end
    int32_t end = MOD1 - 1, begin = 0;
    for (; begin < MOD1 && begin < end; ++begin) {
        if (table[begin].cnt == 0) {
            while (table[end].cnt == 0) {
                --end;
            }
            if (end <= begin) {
                break;
            }
            table[begin] = table[end];
            --end;
        }
    }
    // sort all non-empty buckets
    qsort(table, begin, sizeof(struct hash_table_node), cmp);
    
    char number[10];
    for (int32_t i = 0; i < begin ; ++i) {
        if (table[i].cnt > 0) { 
            int32_t number_len = itoa(table[i].cnt, number);
            if (write_to_buffer(out, number, number_len) == -1) {
                // here and below - write failed
                return -1;
            }
            if (write_to_buffer(out, " ", 1) == -1) {
                return -1;
            }
            if (write_to_buffer(out, table[i].word, table[i].len) == -1) {
                return -1;
            }
            if (write_to_buffer(out, "\n", 1) == -1) {
                return -1;
            }
        }
    }
}

int32_t lower_bound_prime(int32_t bound) {
    char * sieve = calloc(bound + 1, sizeof(char));
    int32_t res;
    for (int64_t i = 2; i <= bound; ++i) {
        if (sieve[i] == 0) {
            res = i;
            for (int64_t j = i * i; j <= bound; j += i) {
                sieve[j] = 1; 
            }
        }
    }
    free(sieve);
    return res;
}

int main(int arc, char ** argv) {
    clock_t begin = clock();
    
    char * inp_file = argv[1];
    char * out_file = argv[2];
    
    int inp_fd = open(inp_file, O_RDONLY);
    int out_fd = open(out_file, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    
    if (inp_fd == -1) {
        fprintf(stderr, "Cannot open inp file\n");
        return -1;
    }
    
    if (out_fd == -1) {
        fprintf(stderr, "Cannot open out file\n");
        return -1;
    }
    
    buffer_t * buffer_inp = calloc(1, sizeof(buffer_t));
    buffer_t * buffer_out = calloc(1, sizeof(buffer_t));
    
    if (buffer_inp == NULL || buffer_out == NULL) {
        fprintf(stderr, "Cannot allocate memory for buffer_t\n");
        return -1;
    }
    struct stat file_stats;
    
    if (fstat(inp_fd, &file_stats) == -1) {
        fprintf(stderr, "Cannot read file stats\n");
        
        free(buffer_inp);
        free(buffer_out);
        return -1;
    }
    size_t inp_size = file_stats.st_size;

    buffer_inp->size = inp_size;

    buffer_inp->data = calloc(inp_size, sizeof(char));

    size_t out_size = inp_size;
    while (buffer_out->data == NULL && out_size > 0) {
        buffer_out->data = calloc(out_size, sizeof(char));
        buffer_out->size = out_size;
        out_size /= 2;
    }
    
    if (buffer_inp->data == NULL || buffer_out->data == NULL) {
        fprintf(stderr, "Cannot allocate memory for input / output\n");
        free(buffer_inp->data);
        free(buffer_out->data);

        free(buffer_inp);
        free(buffer_out);
        return -1;
    }

    buffer_inp->fd = inp_fd;
    buffer_out->fd = out_fd;
    
    // reading from file to buffer
    if (read_to_buffer_from_file(buffer_inp) == -1) {
        fprintf(stderr, "read error\n");
        
        free(buffer_inp->data);
        free(buffer_out->data);

        free(buffer_inp);
        free(buffer_out);

        return -1;
    }
    
    // optimal hash table size choice
    
    int candidate = lower_bound_prime(BUCKETS_PER_WORD * count_words(buffer_inp));
    
    if (candidate < MOD1) {
        MOD1 = candidate;
    }
    
    struct hash_table_node * hash_table = calloc(MOD1, sizeof(struct hash_table_node));
    
    if (hash_table == NULL) {
        fprintf(stderr, "Cannot allocate memory for hash_table\n");
    } else if (feed_table(buffer_inp, &hash_table[0]) == -1) {
        fprintf(stderr, "Error while writing to hash table\n");
    } else if (dump_table(hash_table, buffer_out) == -1) {
        fprintf(stderr, "Error while writing output\n");
    } else if (write_from_buffer_to_file(buffer_out) == -1) {
        fprintf(stderr, "Error while writing output\n");
    }
    
    // free dynamic memory
    
    free(buffer_inp->data);
    free(buffer_out->data);

    free(buffer_inp);
    free(buffer_out);

    free(hash_table);

    clock_t end = clock();
    fprintf(stderr, "time: %.7f\n", (double) (end - begin) / CLOCKS_PER_SEC);
    return 0;
}
