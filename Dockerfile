FROM frolvlad/alpine-gcc

WORKDIR /home/word_counter

COPY . .

RUN apk add make && \
    make build

CMD ./freq in out 2> /dev/null && cat out
