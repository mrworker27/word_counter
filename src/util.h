#ifndef INCLUDE_UTIL
#define INCLUDE_UTIL

int32_t itoa(uint32_t val, char * out) {
    int32_t len = 0;
    while (val > 0) {
        *(out + len) = val % 10 + '0';
        val /= 10;
        ++len;
    }
    for (int32_t i = 0; i < len / 2; ++i) {
        char tmp = out[i];
        out[i] = out[len - i - 1];
        out[len - i - 1] = tmp;
    }
    return len;
}

int32_t str_compare(char * first, char * second, int32_t len_first, int32_t len_second) {
    for (int32_t i = 0; i < len_first && i < len_second; ++i) {
        if (first[i] != second[i]) {
            return (int32_t) (first[i] - second[i]);
        }
    }
    return len_first - len_second;
}

#endif
