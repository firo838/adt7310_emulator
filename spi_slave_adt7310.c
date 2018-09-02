/*
*
* This is a ADT3710 emulator.
* Create by rosev.
* Aug. 4. 2018.
*
*/

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SPI_SLAVE_ADT3710_BUFFER_LEN 2

#define ADT3710_COMMAND_0x54		0x54

typedef struct {
	u_int8_t 0x00_reg;		/* 0x00 is status register.*/
	u_int8_t 0x01_reg;		/* 0x01 is configuration register.*/
	u_int8_t 0x02_reg[2];	/* 0x02 is */
	u_int8_t 0x03_reg[2];
	u_int8_t 0x04_reg[2];
	u_int8_t 0x04_reg[2];
	u_int8_t 0x05_reg[2];
	u_int8_t 0x06_reg[2];
	u_int8_t 0x07_reg[2];
} adt3710_reg_t;

typedef struct {
    adt3710_reg_t reg;
    u_int8_t buffer[0];
} adt3710_t;

adt3710_t *
adt3710_t_init(void)
{
    adt3710_t *handle;

    if((handle = (adt3710_t *)malloc(sizeof(adt3710_t) + SPI_SLAVE_ADT3710_BUFFER_LEN)) == NULL) {
        perror("malloc");
        return NULL;
    }
/*
 *  Initialize adt3710_t accordingly
 */
    handle->reg.0x00_reg = 0x80;
    handle->reg.0x01_reg = 0x00;
    handle->reg.0x02_reg[0] = 0x00;
    handle->reg.0x02_reg[1] = 0x00;
    handle->reg.0x03_reg[0] = 0xC0;
    handle->reg.0x03_reg[1] = 0x00;
    handle->reg.0x04_reg[0] = 0x49;
    handle->reg.0x04_reg[1] = 0x80;
    handle->reg.0x05_reg[0] = 0x05;
    handle->reg.0x05_reg[1] = 0x00;
    handle->reg.0x06_reg[0] = 0x20;
    handle->reg.0x06_reg[1] = 0x00;
    handle->reg.0x07_reg[0] = 0x05;
    handle->reg.0x07_reg[1] = 0x00;

    bzero(handle->buffer, SPI_SLAVE_ADT3710_BUFFER_LEN);
    return handle;
}

int
adt3710_tick(u_int8_t input)
{
/*
 *  Manipulate adt3710_t accordingly
 */
    return 0;
}

u_int8_t
tmp_gen()
{
	return 0x0020;
}

int
adt3710(adt3710_t *handle, u_int8_t input, u_int8_t **output)
{
    int length = 0;

    switch(input) {
    case SPI_SLAVE_COMMAND_0x54:
        handle->buffer[0] = handle->buffer[1];
        handle->buffer[1] = handle->reg.padc_msb;
        *output = handle->buffer;
        length = 1;
        break;
    case SPI_SLAVE_ADT3710_COMMAND_START_CONVERSIONS:  /* fall through */
    default:
        handle->buffer[0] = handle->buffer[1];
        handle->buffer[1] = 0x00;
        *output = handle->buffer;
        length = 1;
    }
    if(adt3710_tick(input) != 0) {
        length = -1;
    }
    return length;
}

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
    int s, cs, l, out_len, buffer_ptr = 0;
    u_int8_t in, *out, buffer[BUFSIZ];
    adt3710_t *handle;

    // Check options.
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

    if((handle = adt3710_init()) == NULL) {
        fprintf(stderr, "failed to initiallize adt3710 instance\n");
        exit(EXIT_FAILURE);
    }

    // Main loop.
    for(;;) {
        if((cs = accept(s, NULL, NULL)) == -1) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        fprintf(stderr, "connection established\n");
        for(;;) {
            if(read(cs, &in, 1) > 0) {
                //printf(" read: %02hhx\n", in);
                if((out_len = adt3710(handle, in, &out)) < 0) {
                    break;
                }
                if(out_len > 0) {
                    memmove(buffer + buffer_ptr, out, out_len);
                    buffer_ptr += out_len;
                }
                if(buffer_ptr > 0) {
                    if(write(cs, &buffer[0], 1) > 0) {
                        //printf("write: %02hhx\n", buffer[0]);
                        buffer_ptr--;
                        continue;
                    } else {
                        fprintf(stderr, "connection closed\n");
                        break;
                    }
                }
            } else {
                fprintf(stderr, "connection closed\n");
                break;
            }
        }
        close(cs);
    }
    return 0;

