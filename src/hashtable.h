#include "util.h"

// default MOD

int32_t MOD1  = 64000031;

uint32_t hash(char * word, int32_t len, uint32_t base, uint32_t modulo) {
    uint32_t hash = 0ll;
    for (int32_t i = 0; i < len; ++i) {
        hash += (1 + word[i] - 'a');
        hash *= base;
        // hash = hash mod 2^32
    }
    return hash % MOD1;
}

struct hash_table_node {
    char * word;
    uint32_t check_hash;
    int32_t cnt;
    int32_t len;
};

int32_t get_index(struct hash_table_node * table, char * word, int32_t len) {
    uint32_t index = hash(word, len, BASE1, MOD1);
    // first time we see this hash
    if (table[index].word == NULL) {
        return index;
    }
    uint32_t check = hash(word, len, BASE2, MOD2);
    if (table[index].check_hash == check && table[index].len == len) {
        if (str_compare(table[index].word, word, len, len) == 0) {
            return index;
        }
    }
    int32_t base = index + 1;
    while (base != index) {
        if (table[base].word == NULL) {
            return base;
        }
        if (table[base].check_hash != check || table[base].len != len) {
            ++base;
            // ez collision
        } else {
            if (str_compare(table[base].word, word, len, len) == 0) {
                return base;
            }
            // hard collision
            ++base;
        }
        if (base == MOD1) {
            base = 0;
        }
    }
    return -1;
}

int add_to_hash_table(struct hash_table_node * table, char * word, int32_t len) {
    int32_t index = get_index(table, word, len);
    if (index == -1) {
        return -1;
    }
    table[index].word = word;
    table[index].check_hash = hash(word, len, BASE2, MOD2);
    table[index].len = len;
    ++table[index].cnt;
    
    return 0;
}

int cmp(const void * first, const void * second) {
    int32_t cnt2 = ((struct hash_table_node *)(second))->cnt;
    int32_t cnt1 = ((struct hash_table_node *)(first))->cnt;

    int32_t diff = cnt2 - cnt1;

    int32_t len1 = ((struct hash_table_node *)(first))->len;
    int32_t len2 = ((struct hash_table_node *)(second))->len;
    
    if (diff != 0) {
        return diff;
    } else {
        char * word1 = ((struct hash_table_node *)(first))->word;
        char * word2 = ((struct hash_table_node *)(second))->word;
        return str_compare(word1, word2, len1, len2);
    }
}

