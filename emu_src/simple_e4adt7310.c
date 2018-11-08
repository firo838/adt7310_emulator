#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SPI_SLAVE_MPL115A1_BUFFER_LEN 2

#define E4ADT7310_READ_CONTINUOUSLY         0x54
#define E4ADT7310_READ_THIGH                0x60

#define SLEEP_TIME                          1

int
get_server_socket(const char *const path)
{
    int s;
    struct sockaddr_un sun;

    memset(&sun, 0, sizeof(sun));
    sun.sun_family = AF_UNIX;
    strncpy(sun.sun_path, path, sizeof(sun.sun_path) - 1);

    if((s = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }
    unlink(path);
    if(bind(s, (struct sockaddr *)&sun, sizeof(struct sockaddr_un)) < 0) {
        perror("bind");
        return -1;
    }
    if(listen(s, SOMAXCONN) < 0) {
        perror("listen");
        return -1;
    }

    return s;
}

int
main(int argc, char *argv[])
{
    int s, cs;
    u_int8_t in, buffer[BUFSIZ];

    if(argc == 1) {
        if((s = get_server_socket("/tmp/spi")) == -1) {
            exit(EXIT_FAILURE);
        }
    } else if(argc == 2) {
        if((s = get_server_socket(argv[1])) == -1) {
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "usage: %s [/path/to/socket]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    for(;;) {
        if((cs = accept(s, NULL, NULL)) == -1) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        fprintf(stderr, "connection established\n");
        for(;;) {
            if(read(cs, &in, 1) > 0) {
                printf(" read: %02hhx\n", in);
                switch(in){
                    case E4ADT7310_READ_CONTINUOUSLY:   // read continuously.
                        break;
                    case E4ADT7310_READ_THIGH:          // read thigh
                        buffer[0] = 0x49;
                        buffer[1] = 0x80;
                        write(cs, &buffer[0], 1);
                        write(cs, &buffer[1], 1);
                        printf("write: %02hhx%02hhx\n", buffer[0],buffer[1]);
                        break;
                    default:
                        break;
                }
            }
            buffer[0] = 0x0D;
            buffer[1] = 0xC6;
            write(cs, &buffer[0], 1);
            write(cs, &buffer[1], 1);
            printf("write: %02hhx%02hhx\n", buffer[0],buffer[1]);
            sleep(SLEEP_TIME);
        }
        close(cs);
    }
    return 0;
}
