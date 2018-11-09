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
    handle->reg0    = 0x80;     // default 0x80 Resolution is 13bit, Mode is continuous read.
    handle->reg1    = 0x00;     // default 0x00
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
        #ifdef DEBUG_PRINT
              printf("gen_temp2 : i:%d , a = %f\n",i,a);  
        #endif
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
    srand((unsigned int)time(NULL));
	t = (rand()%10000)/1000 + 15.0;

    return t;
}

void
set_temp(adt7310_t *handle)
{
    /*
     *  Returning Temperature Value.
     */
    // u_int16_t temp = gen_temp2(handle);
    u_int16_t temp = gen_temp();

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
    int i;

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
                for(i = 0; i < 2; i++){
                    buffer[i] = handle->reg2[i];
                    write(cs, &buffer[i], 1);
                    #ifdef PRINT_SPI_COMM
                        printf("write : %02hhx (Temperature value)\n", buffer[i]);
                    #endif
                }
                handle->reg0 |= 0x80;   // status : RDY bit change to disable
            }else if(rw_flag == 1){
                // read mode is enabled
                // Note : Usage prohibited : Rewrite processing of status register(command byte : 0x00)
            }else{
                // another condition
                // write status
                buffer[0] = handle->reg0;
                write(cs, &buffer, 1);
                #ifdef PRINT_SPI_COMM
                    printf("write : %02hhx (status register)\n", buffer[0]);
                #endif
            }

            break;
        case REG_NAME_CONFIGURATION:
            // configuration register
            if(rw_flag == 0){
                // write mode is enabled
                // change config
                // read config command byte
                if(read(cs, &in, 1) > 0){
                #ifdef PRINT_SPI_COMM
                    printf("read  : %02hhx (Configuration : mode set)\n", in);
                #endif
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
                    if(write(cs, &buffer, 1) > 0){
                        #ifdef PRINT_SPI_COMM
                            printf("write : %02hhx (Configuration : write config)\n", buffer[0]);
                        #endif
                        handle->reg0 |= 0x80;   // status : RDY bit change to disable
                    }
                }  
                if((in & 0x60) == 0x40)  handle->reg1 = (handle->reg1 & 0x9F) | 0x40;   // set sps mode.
                if((in & 0x60) == 0x60)  handle->reg1 = (handle->reg1 & 0x9F) | 0x60;   // set shutdown mode.
                }
            }else{
                // read mode is enabled
                // write configuration register
                buffer[0] = handle->reg1;
                write(cs, &buffer, 1);
                #ifdef PRINT_SPI_COMM
                    printf("write : %02hhx (configuration register)\n", buffer[0]);
                #endif
            }

            break;
        case REG_NAME_TEMPERATIRE_VALUE:
            // write temperature value.
            // Both rw_flag is ok

            set_temp(handle);            

            for(i = 0; i < 2; i++){
                buffer[i] = handle->reg2[i];
                write(cs, &buffer[i], 1);
                #ifdef PRINT_SPI_COMM
                    printf("write : %02hhx (temperature register)\n", buffer[i]);
                #endif
            }
            handle->reg0 |= 0x80;   // status : RDY bit change to enable

            break;
        case REG_NAME_ID:
            // return id.
            // Both rw_flag is ok

            buffer[0] = handle->reg3;
            write(cs, &buffer[0], 1);
            #ifdef PRINT_SPI_COMM
                printf("write : %02hhx\n", buffer[0]);
            #endif
            
            break;
        case REG_NAME_TCRIT_SETPOINT:
            // return Tcrit Setpoint.

            if(rw_flag == 0){
                // write mode is enabled
                for(i = 0; i < 2; i++){
                    read(cs, &buffer[i], 1);
                    handle->reg4[i] = buffer[i];
                    #ifdef PRINT_SPI_COMM
                        printf("read  : %02hhx (Tcrit Setpoint)\n", buffer[i]);
                    #endif
                }
            }else{
                for(i = 0; i < 2; i++){
                    buffer[i] = handle->reg4[i];
                    write(cs, &buffer[i], 1);
                    #ifdef PRINT_SPI_COMM
                        printf("write : %02hhx (Tcrit Setpoint r)\n", buffer[i]);
                    #endif
                }
            }
            
            break;
        case REG_NAME_THYST_SETPOINT:
            // return Thyst Setpoint.

            if(rw_flag == 0){
                // write mode is enabled
                for(i = 0; i < 2; i++){
                    read(cs, &buffer[i], 1);
                    handle->reg5[i] = buffer[i];
                    #ifdef PRINT_SPI_COMM
                        printf("read  : %02hhx (Thyst Setpoint)\n", buffer[i]);
                    #endif
                }
            }else{
                for(i = 0; i < 2; i++){
                    buffer[i] = handle->reg5[i];
                    write(cs, &buffer[i], 1);
                    #ifdef PRINT_SPI_COMM
                        printf("write : %02hhx (Thyst Setpoint)\n", buffer[i]);
                    #endif
                }
            }
            
            break;
        case REG_NAME_THIGH_SETPOINT:
            // return Thigh Setpoint.

            if(rw_flag == 0){
                // write mode is enabled
                for(i = 0; i < 2; i++){
                    read(cs, &buffer[i], 1);
                    handle->reg6[i] = buffer[i];
                    #ifdef PRINT_SPI_COMM
                        printf("read  : %02hhx (Thigh Setpoint)\n", buffer[i]);
                    #endif
                }
            }else{
                for(i = 0; i < 2; i++){
                    buffer[i] = handle->reg6[i];
                    write(cs, &buffer[i], 1);
                    #ifdef PRINT_SPI_COMM
                        printf("write : %02hhx (Thigh Setpoint)\n", buffer[i]);
                    #endif
                }
            }
            
            break;
        case REG_NAME_TLOW_SETPOINT:
            // return Tlow Setpoint.

            if(rw_flag == 0){
                // write mode is enabled
                for(i = 0; i < 2; i++){
                    read(cs, &buffer[i], 1);
                    handle->reg7[i] = buffer[i];
                    #ifdef PRINT_SPI_COMM
                        printf("read  : %02hhx (Tlow Setpoint)\n", buffer[i]);
                    #endif
                }
            }else{
                for(i = 0; i < 2; i++){
                    buffer[i] = handle->reg7[i];
                    write(cs, &buffer[i], 1);
                    #ifdef PRINT_SPI_COMM
                        printf("write : %02hhx (Tlow Setpoint)\n", buffer[i]);
                    #endif
                }
            }
            
            break;
        default:
                #ifdef PRINT_SPI_COMM
                    printf("ADT7310 Through\n");
                #endif
            break;
    }

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

    unsigned int counter = 0;
    int sd_flag = -1;

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

    #ifdef DEBUG_PRINT
            printf("Call main func\n");
    #endif

    for(;;) {
        #ifdef DEBUG_PRINT
            printf("Call main : for : 1\n");
        #endif
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
            // set_temp(handle);
            // usleep(CONVERSION_TIME);

            // check shutdown mode
            // Shutdown the temperature measurement and conversion circuit.
            // Register read / write is possible even during shutdown.
            // sd_flag == 0 is shutdown mode, 1 is other mode.
            sd_flag = (handle->reg1 & 0x60) == 0x60 ? 0 : 1;

            // output
            // chech mode
            mode = handle->reg1 & 0x60;
            if((mode == 0x00) && ((handle->reg0 & 0x80) == 0x00) && sd_flag){
                // continuous mode
                set_temp(handle);
                usleep(CONVERSION_TIME);
            }else if((mode == 0x20) && ((handle->reg0 & 0x80) == 0x80) && sd_flag){
                // one shot mode.
                // One shot mode is processing at received the command byte in adt7310 function.
                set_temp(handle);
                usleep(CONVERSION_TIME);
            }else if(mode == 0x40 && sd_flag){
                // sps mode.
                // This process is skipped.
                if(counter == 100000000){
                    // Measure every 1 second and set the value.
                    set_temp(handle);
                    usleep(CONVERSION_TIME);
                    counter = 0;
                }
            }else if(mode == 0x60){
                // shutdown mode.
                // This process is skipped.
            }

            counter++;

            poll(&fds, 1, 0);

            if(fds.revents > 0) {
                // input
                if(read(cs, &in, 1) > 0){
                    #ifdef PRINT_SPI_COMM
                        printf("read  : %02hhx\n", in);
                    #endif
                    adt7310(handle, in, cs);
                }
            }
        }
        close(cs);
        fprintf(stderr, "connection closed\n");
    }
    return 0;
}
