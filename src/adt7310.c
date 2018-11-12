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

#include "adt7310.h"

adt7310_t *
adt7310_init(void)
{
    adt7310_t *handle;

    if((handle = (adt7310_t *)malloc(sizeof(adt7310_t))) == NULL) {
        perror("malloc");
        return NULL;
    }

/*
 *  Initialize adt7310_t accordingly
 */
    handle->reg0    = 0x80;     // default 0x80 status RDY bit is enabled.
    handle->reg1    = 0x00;     // default 0x00 Resolution is 13bit, Mode is continuous read.
    handle->reg2[0] = 0x00;     // default 0x0000
    handle->reg2[1] = 0x00;     
    handle->reg3    = 0xC0;     // default 0xC0
    handle->reg4[0] = 0x49;     // default 0x4980
    handle->reg4[1] = 0x80;
    handle->reg5[0] = 0x00;     // default 0x0005
    handle->reg5[1] = 0x05;
    handle->reg6[0] = 0x20;     // default 0x2000
    handle->reg6[1] = 0x00;
    handle->reg7[0] = 0x05;     // default 0x0500
    handle->reg7[1] = 0x00;

    return handle;
}

int
adt7310_tick(u_int8_t input)
{
    /*
     *  Manipulate adt7310_t accordingly.
     */
    return 0;
}

u_int16_t
gen_temp()
{
    /*
     * The basic mode is 16 bit mode. Therefore, 
     * it generates temperature for 16 bit.
     */
    return 0x0DC6;
}

u_int16_t
gen_temp2(adt7310_t *handle)
{   
    u_int16_t temp = 0x0000;    // default
    int integer, i, fraction;
    char fra[8];
    float a, t = random_temp();

    // res_flag : 0 is 13 bit mode, 1 is 16 bit mode.
    int res_flag = -1;
    // check resolution
    if((handle->reg1 & 0x80) == 0x00){
        // 13bit mode
        res_flag = 0;
    }else{
        // 16bit mode
        res_flag = 1;
    }

    integer = t;
    a = (t - integer);
	
	for(i = 0; i < 8; i++){
		a = a * 2;
	    if(a >= 1){
    		fra[i] = '1';
			a -= 1;
		}else{
    		fra[i] = '0';
	    }
    }
	fraction = strtol(fra, NULL, 2); // convert str to long

    // res_flag 13bit
    if(res_flag == 0){
        // 13bit mode processing.
        temp = integer << 6;
        temp |= fraction >> 1;
        temp &= 0xfff8;
    }
    // res_flag 16bit
    if(res_flag == 1){
        // 16bit mode processing.
	    temp = integer << 6;
        temp |= fraction >> 1;
    }

    return temp;
}

float
random_temp(){
    float a, t;
	t = (rand()%10000)/1000 + 15.0;

    return t;
}

void
set_temp(adt7310_t *handle)
{
    /*
     *  Returning Temperature Value.
     */
    // u_int16_t temp = gen_temp();
    u_int16_t temp = gen_temp2(handle);

    handle->reg2[0] = (u_int8_t)(temp >> 8);
    handle->reg2[1] = (u_int8_t)(temp & 0xff);

    // Set when temperature reading becomes possible. RDY = 0 is read ok.
    handle->reg0 &= 0x7f;   // status : RDY bit change to enable
}

void
adt7310(adt7310_t *handle, u_int8_t input, int cs)
{
    u_int8_t in = 0x00;
    u_int8_t buffer[2];
    int rw_flag = -1;
    int cnt_flag = -1;

    // write is 0, read is 1.
    if((input & 0x40) == 0x40){
        rw_flag = 1;
    }else{
        rw_flag = 0;
    }

    // check continuous mode.
    if((input & 0x04) == 0x04){
        handle->reg1 |= 0x04;
    }
    // continuous mode is 1.
    if((handle->reg1 & 0x60) == 0x00){
        cnt_flag = 1;
    }else{
        cnt_flag = 0;
    }

    // check target 
    u_int8_t tgt_reg = (input & 0x38) >> 3;

    switch(tgt_reg){
        case REG_NAME_STATSUS:
            // write status register.
            if(cnt_flag == 1 && rw_flag == 0){
                // continuous mode is enabled and write mode is enabled
                // 0x00 default processing
                // command byte -> 0x00 -> 0x00の順で受け取り，writeする
                if(read(cs ,&buffer[0], 1) > 0){
                    e4adt7310_log("read", "read (deafult 0x00)", buffer[0], 1);
                    if(buffer[0] == 0x00){
                        buffer[0] = handle->reg2[0];                
                        write(cs, &buffer[0], 1);
                        e4adt7310_log("write", "Temperature value : read (deafult 0x00)", buffer[0], 1);
                        // The second read method
                        if(read(cs, &buffer[1], 1) > 0){
                            e4adt7310_log("read", "read (deafult 0x00)", buffer[0], 2);
                            if(buffer[1] == 0x00){
                                write(cs, &buffer[1], 1);
                                e4adt7310_log("write", "Temperature value : read (deafult 0x00)", buffer[1], 2);
                            }else{
                                // adt7310(handle, buffer[1], cs);
                            }
                        }
                    }
                }
            }else if(rw_flag == 1){
                // read mode is enabled
                // Note : Usage prohibited : Rewrite processing of status register(command byte : 0x00)
            }else{
                // another condition
                // write status
                buffer[0] = handle->reg0;
                write(cs, &buffer, 1);
                e4adt7310_log("write", "status register : read", buffer[0], 0);
            }
            
            handle->reg0 |= 0x80;   // status : RDY bit change to disable

            break;
        case REG_NAME_CONFIGURATION:
            // configuration register
            if(rw_flag == 0){
                // write mode is enabled
                // change config
                // read config command byte
                if(read(cs, &in, 1) > 0){
                    e4adt7310_log("read", "configuration register : set", in, 0);

                    // change resolution.
                    if((in & 0x80) == 0x80)  handle->reg1 |= 0x80;
                    if((in & 0x80) == 0x00)  handle->reg1 &= 0x7f;

                    // change mode.
                    if((in & 0x60) == 0x00)  handle->reg1 = (handle->reg1 & 0x9F) | 0x00;   // set continuous mode.
                    if((in & 0x60) == 0x20){
                        handle->reg1 = (handle->reg1 & 0x9F) | 0x20;   // set one-shot mode.
                        // set_temp(handle);
                        handle->reg0 &= 0x7f;   // status : RDY bit change to enable
                        buffer[0] = handle->reg1;
                        if(write(cs, &buffer[0], 1) > 0){
                            e4adt7310_log("write", "configuration register : set", buffer[0], 0);
                            handle->reg0 |= 0x80;   // status : RDY bit change to disable
                        }
                    }  
                    if((in & 0x60) == 0x40)  handle->reg1 = (handle->reg1 & 0x9F) | 0x40;   // set sps mode.
                    if((in & 0x60) == 0x60)  handle->reg1 = (handle->reg1 & 0x9F) | 0x60;   // set shutdown mode.
                }
            }else{
                // read mode is enabled
                // write configuration register
                if(read(cs, &buffer[0], 1) > 0){
                    e4adt7310_log("read", "configuration register : read", in, 0);
                    buffer[0] = handle->reg1;
                    write(cs, &buffer[0], 1);
                    e4adt7310_log("write", "configuration register : read", buffer[0], 0);
                }
            }

            break;
        case REG_NAME_TEMPERATIRE_VALUE:
            // write temperature value.
            // Both rw_flag is ok

            set_temp(handle);            

            if(read(cs, &buffer[0], 1) > 0){
                e4adt7310_log("read", "read (deafult 0x00)", buffer[0], 1);
                if(buffer[0] == 0x00){
                    buffer[0] = handle->reg2[0];
                    write(cs, &buffer[0], 1);
                    e4adt7310_log("write", "temperature register : read", buffer[0], 1);
                    if(read(cs, &buffer[1], 1) > 0){
                        e4adt7310_log("read", "read (deafult 0x00)", buffer[0], 2);
                        if(buffer[1] == 0x00){
                            write(cs, &buffer[1], 1);
                            e4adt7310_log("write", "temperature register : read", buffer[1], 2);
                        }else{
                            // adt7310(handle, buffer[1], cs);
                        }
                    }
                }
            }
            
            handle->reg0 |= 0x80;   // status : RDY bit change to enable

            break;
        case REG_NAME_ID:
            // return id.
            // Both rw_flag is ok

            if(read(cs, &buffer[0], 1) > 0){
                e4adt7310_log("read", "read (deafult 0x00)", buffer[0], 0);
                if(buffer[0] == 0x00){
                    buffer[0] = handle->reg3;
                    write(cs, &buffer[0], 1);
                    e4adt7310_log("write", "ID register : read", buffer[0], 0);
                }
            }
            
            break;
        case REG_NAME_TCRIT_SETPOINT:
            // return Tcrit Setpoint.

            if(rw_flag == 0){
                // write mode is enabled
                if(read(cs, &buffer[0], 1) > 0){
                    e4adt7310_log("read", "read (deafult 0x00)", buffer[0], 0);
                    handle->reg4[0] = buffer[0];
                    if(read(cs, &buffer[1], 1) > 0){
                        e4adt7310_log("read", "Tcrit Setpoint : set", buffer[1], 0);
                        handle->reg4[1] = buffer[1];
                    }
                }
            }else{
                if(read(cs, &buffer[0], 1) > 0){
                    e4adt7310_log("read", "read (deafult 0x00)", buffer[0], 0);
                    if(buffer[0] == 0x00){
                        buffer[0] = handle->reg4[0];
                        write(cs, &buffer[0], 1);
                        e4adt7310_log("write", "Tcrit Setpoint : read", buffer[0], 0);
                        if(read(cs, &buffer[0], 1) > 0){
                            e4adt7310_log("read", "read (deafult 0x00)", buffer[0], 0);
                            if(buffer[1] == 0x00){
                                buffer[1] = handle->reg4[1];
                                write(cs, &buffer[1], 1);
                                e4adt7310_log("write", "Tcrit Setpoint : read", buffer[1], 0);
                            }
                        }
                    }
                }
            }
            
            break;
        case REG_NAME_THYST_SETPOINT:
            // return Thyst Setpoint.

            if(rw_flag == 0){
                // write mode is enabled
                if(read(cs, &buffer[0], 1) > 0){
                    handle->reg5[0] = buffer[0];
                    e4adt7310_log("read", "Thyst Setpoint : set", buffer[0], 0);
                    handle->reg5[0] = buffer[0];
                    if(read(cs, &buffer[1], 1) > 0){
                        e4adt7310_log("read", "Thyst Setpoint : set", buffer[1], 0);
                        handle->reg5[1] = buffer[1];
                    } 
                }
            }else{
                if(read(cs, &buffer[0], 1) > 0){
                    e4adt7310_log("read", "read (deafult 0x00)", buffer[0], 0);
                    if(buffer[0] == 0x00){
                        buffer[0] = handle->reg5[0];
                        write(cs, &buffer[0], 1);
                        e4adt7310_log("write", "Thyst Setpoint : read", buffer[0], 0);
                        if(read(cs, &buffer[0], 1) > 0){
                            e4adt7310_log("read", "read (deafult 0x00)", buffer[0], 0);
                            if(buffer[1] == 0x00){
                                buffer[1] = handle->reg5[1];
                                write(cs, &buffer[1], 1);
                                e4adt7310_log("write", "Thyst Setpoint : read", buffer[1], 0);
                            }
                        }
                    }
                }
            }
            
            break;
        case REG_NAME_THIGH_SETPOINT:
            // return Thigh Setpoint.

            if(rw_flag == 0){
                // write mode is enabled
                if(read(cs, &buffer[0], 1) > 0){
                    handle->reg6[0] = buffer[0];
                    e4adt7310_log("read", "Thigh Setpoint : set", buffer[0], 0);
                    handle->reg6[0] = buffer[0];
                    if(read(cs, &buffer[1], 1) > 0){
                        e4adt7310_log("read", "Thigh Setpoint : set", buffer[1], 0);
                        handle->reg6[1] = buffer[1];
                    }
                } 
            }else{
                if(read(cs, &buffer[0], 1) > 0){
                    e4adt7310_log("read", "Tread (deafult 0x00)", buffer[0], 0);
                    if(buffer[0] == 0x00){
                        buffer[0] = handle->reg6[0];
                        write(cs, &buffer[0], 1);
                        e4adt7310_log("write", "Thigh Setpoint : read", buffer[0], 0);
                        if(read(cs, &buffer[0], 1) > 0){
                            e4adt7310_log("read", "read (deafult 0x00)", buffer[0], 0);
                            if(buffer[0] == 0x00){
                                buffer[1] = handle->reg6[1];
                                write(cs, &buffer[1], 1);
                                e4adt7310_log("write", "Thigh Setpoint : read", buffer[1], 0);
                            }
                        }
                    }
                }
            }
            
            break;
        case REG_NAME_TLOW_SETPOINT:
            // return Tlow Setpoint.

            if(rw_flag == 0){
                // write mode is enabled
                if(read(cs, &buffer[0], 1) > 0){
                    e4adt7310_log("read", "Tlow Setpoint : set", buffer[0], 0);
                    handle->reg7[0] = buffer[0];
                    if(read(cs, &buffer[1], 1) > 0){
                        e4adt7310_log("read", "Tlow Setpoint : set", buffer[1], 0);
                        handle->reg7[1] = buffer[1];
                    }
                } 
            }else{
                if(read(cs, &buffer[0], 1) > 0){
                    e4adt7310_log("read", "read (deafult 0x00)", buffer[0], 0);
                    if(buffer[0] == 0x00){
                        buffer[0] = handle->reg7[0];
                        write(cs, &buffer[0], 1);
                        e4adt7310_log("write", "Tlow Setpoint : read", buffer[0], 0);
                        if(read(cs, &buffer[0], 1) > 0){
                            e4adt7310_log("read", "read (deafult 0x00)", buffer[0], 0);
                            if(buffer[0] == 0x00){
                                buffer[1] = handle->reg7[1];
                                write(cs, &buffer[1], 1);
                                e4adt7310_log("write", "Tlow Setpoint : read", buffer[1], 0);
                            }
                        }
                    }
                }
            }
            
            break;
        default:
                #ifdef PRINT_SPI_COMM_DEBUG
                    printf("ADT7310 Through\n");
                #endif
            break;
    }
}

void e4adt7310_log(char *rw, char *s, u_int8_t buffer, int c)
{
    #ifdef PRINT_SPI_COMM_DEBUG
        printf("%5s : %02hhx (",rw, buffer);
        printf("%s)",s);
        if(c != 0)  printf(" %d",c);
        printf("\n");
    #endif
    #ifdef PRINT_SPI_COMM
        printf("write : %02hhx\n", buffer);
        printf("%5s : %02hhx\n",rw, buffer);
    #endif
    #ifdef DEBUG_PRINT
        printf("%s=%x\n",s,buffer);
    #endif
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
    int s, cs, l, i;
    u_int8_t in, *out, buffer[BUFSIZ], mode;
    adt7310_t *handle;

    struct pollfd fds;
    int ret = -1;

    int sd_flag = -1;

    time_t timer = time(&timer);
    time_t sps_timer;

    srand(SEED);

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
    if((handle = adt7310_init()) == NULL) {
        fprintf(stderr, "failed to initiallize adt7310 instance\n");
        exit(EXIT_FAILURE);
    }

    for(;;) {
        if((cs = accept(s, NULL, NULL)) == -1) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        fds.fd = cs;
        fds.events = POLLIN;

        fprintf(stderr, "connection established\n");
        for(;;) {
            /*
             * The ADT 7310 requires 240 microseconds to repel the obtained temperature data to digital data.
             */

            /* 
             * check shutdown mode
             * Shutdown the temperature measurement and conversion circuit.
             * Register read / write is possible even during shutdown.
             * sd_flag == 0 is shutdown mode, 1 is other mode.
             */

            sd_flag = (handle->reg1 & 0x60) == 0x60 ? 0 : 1;

            // output
            // chech mode
            mode = handle->reg1 & 0x60;
            if((mode == 0x00) && ((handle->reg0 & 0x80) == 0x00) && sd_flag){
                // continuous mode
                set_temp(handle);
                e4adt7310_log("", "Mode : Continuous : handle->reg1", handle->reg1, 0);
                usleep(CONVERSION_TIME);
            }else if((mode == 0x20) && ((handle->reg0 & 0x80) == 0x80) && sd_flag){
                // One-shot mode.
                // One-shot mode is processing at received the command byte in adt7310 function.
                set_temp(handle);
                e4adt7310_log("", "Mode : One-Shot : handle->reg1", handle->reg1, 0);
                usleep(CONVERSION_TIME);
            }else if(mode == 0x40 && sd_flag){
                // 1 SPS mode.
                // This process is skipped.
                time(&sps_timer);
                if(difftime(sps_timer, timer) > 1.0){
                    timer = sps_timer;
                    set_temp(handle);
                    e4adt7310_log("", "Mode : 1 SPS : handle->reg1", handle->reg1, 0);
                    usleep(CONVERSION_TIME);
                }
            }else if(mode == 0x60){
                // shutdown mode.
                // This process is skipped.
                e4adt7310_log("", "Mode : Shutdown : handle->reg1", handle->reg1, 0);
            }

            poll(&fds, 1, 0);

            if(fds.revents > 0) {
                // input (read from spi)
                if(read(cs, &in, 1) > 0){
                    e4adt7310_log("read", "main() for loop", in, 0);
                    adt7310(handle, in, cs);
                }
            }
        }
        close(cs);
        fprintf(stderr, "connection closed\n");
    }
    return 0;
}
