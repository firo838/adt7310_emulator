/*
* ADT7310 emulator.
*
* MIT Lisence
*
* Copyright <2018> < rosev : https://rosev838.github.io >
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*/
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// #define DEBUG_PRINT
#define PRINT_SPI_COMM_DEBUG
//#define PRINT_SPI_COMM

#define REG_NAME_STATSUS                0x00
#define REG_NAME_CONFIGURATION          0x01
#define REG_NAME_TEMPERATIRE_VALUE      0x02
#define REG_NAME_ID                     0x03
#define REG_NAME_TCRIT_SETPOINT         0x04
#define REG_NAME_THYST_SETPOINT         0x05
#define REG_NAME_THIGH_SETPOINT         0x06
#define REG_NAME_TLOW_SETPOINT          0x07

#define SLEEP_TIME                      1
#define CONVERSION_TIME                 240
#define SEED                            10

typedef struct {
    u_int8_t reg0;      // status
    u_int8_t reg1;      // Configuration
    u_int8_t reg2[2];   // Temperature value
    u_int8_t reg3;      // ID
    u_int8_t reg4[2];   // TCRIT Setopoint
    u_int8_t reg5[2];   // THYST Setpoint
    u_int8_t reg6[2];   // THIGH Setpoint
    u_int8_t reg7[2];   // TLOW Setpoint
} adt7310_t;

adt7310_t *adt7310_init(void);
int adt7310_tick(u_int8_t input);
u_int16_t gen_temp();
u_int16_t gen_temp2(adt7310_t *handle);
float random_temp();
void set_temp(adt7310_t *handle);
void adt7310(adt7310_t *handle, u_int8_t input, int cs);
int get_server_socket(const char *const path);
void e4adt7310_log(char *rw, char *s, u_int8_t buffer, int c);
