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

#define PRINT_SOCK_COMM

#define REG_NAME_STATSUS                0x00
#define REG_NAME_CONFIGURATION          0x01
#define REG_NAME_TEMPERATIRE_VALUE      0x02
#define REG_NAME_ID                     0x03
#define REG_NAME_TCRIT_SETPOINT         0x04
#define REG_NAME_THYST_SETPOINT         0x05
#define REG_NAME_THIGH_SETPOINT         0x06
#define REG_NAME_TLOW_SETPOINT          0x07

#define SLEEP_TIME                      1

typedef struct {
    u_int8_t reg0;      // status
    u_int8_t reg1;      // Configuration
    u_int8_t reg2[2];   // Temperature value
    u_int8_t reg3[2];   // ID
    u_int8_t reg4[2];   // TCRIT Setopoint
    u_int8_t reg5[2];   // THYST Setpoint
    u_int8_t reg6[2];   // THIGH Setpoint
    u_int8_t reg7[2];   // TLOW Setpoint
} adt7310_reg;

typedef struct {
    adt7310_reg reg;
    int read_flag;
} adt7310_t;

adt7310_t *adt7310_init(void);
int adt7310_tick(u_int8_t input);
void get_temp(adt7310_t *handle);
u_int16_t gen_temp();
u_int16_t gen_temp2(adt7310_t *handle);
float random_temp();
void set_temp(adt7310_t *handle);
void adt7310(adt7310_t *handle, u_int8_t input, int cs);
int get_server_socket(const char *const path);
