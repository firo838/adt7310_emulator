/*
*
* This is a ADT3710 emulator.
* Create by rosev.
* Aug. 4. 2018.
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
    handle->reg.reg0 = 0x80;        // default 0x80 Resolution is 13bit, Mode is continuous read.
    handle->reg.reg1 = 0x00;        // default 0x00
    handle->reg.reg2[0] = 0x00;     // default 0x0000
    handle->reg.reg2[1] = 0x00;
    handle->reg.reg3[0] = 0x00;     // default 0x00C0
    handle->reg.reg3[1] = 0xC0;
    handle->reg.reg4[0] = 0x49;     // default 0x4980
    handle->reg.reg4[1] = 0x80;
    handle->reg.reg5[0] = 0x00;     // default 0x0005
    handle->reg.reg5[1] = 0x05;
    handle->reg.reg6[0] = 0x20;     // default 0x2000
    handle->reg.reg6[1] = 0x00;
    handle->reg.reg7[0] = 0x05;     // default 0x0500
    handle->reg.reg7[1] = 0x00;
    
    handle->read_flag = -1;

    return handle;
}

int
adt7310_tick(u_int8_t input)
{
/*
 *  Manipulate mpl115a1_t accordingly
 */
    return 0;
}

void
get_temp(adt7310_t *handle)
{
    /*
     *  Returning Temperature Value.
     */
    handle->reg.reg2[0] = 0x0D;
    handle->reg.reg2[1] = 0xC6;
}

u_int16_t
gen_temp()
{
    /*
     * 基本が16bitモードなので，16bit用の温度を生成する
     */
    return 0x0DC6;
}

u_int16_t
gen_temp2(adt7310_t *handle)
{   
    u_int16_t temp = 0x0DC6;    // default
    int integer, i, fraction;
    char fra[8];
    float a, t = random_temp();

    // res_flag : 0 is 13 bit mode, 1 is 16 bit mode.
    int res_flag = -1;
    // check resolution
    if((handle->reg.reg1 & 0x80) == 0x00){
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
		printf("i:%d , a = %f\n",i,a);
    }
	fraction = strtol(fra, NULL, 2);

    // res_flag == 0
    if(res_flag == 0){
        // 13bit mode processing.
        temp = integer << 6;
        temp |= fraction >> 1;
        temp &= 0xfff8;
    }
    // res_flag == 1
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
    
    if((handle->reg.reg1 & 0x80) == 0x80){
        // 16bit mode.
        u_int16_t temp = gen_temp();
        handle->reg.reg2[0] = (u_int8_t)(temp >> 8);
        handle->reg.reg2[1] = (u_int8_t)(temp & 0xff);
    }else{
        // 13bit mode.
        // 13bit用の処理を追記する予定
        u_int16_t temp = gen_temp();
        handle->reg.reg2[0] = (u_int8_t)(temp >> 8);
        handle->reg.reg2[1] = (u_int8_t)(temp & 0xff);
    }

    // 温度読み出しが可能になったらセットする RDY = 0 is read ok.
    handle->reg.reg0 &= 0x7f;
}

void
adt7310(adt7310_t *handle, u_int8_t input, int cs)
{
    u_int8_t in = 0x00;
    u_int8_t buffer[2];
    int rw_flag = -1;
    int i;

    // read is 1, write is 0.
    if((input & 0x40) == 0x40){
        rw_flag = 1;
    }else{
        rw_flag = 0;
    }
    // check continuous mode.
    // continuous mode is 1.
    int cnt_flag = -1;
    if((input & 0x04) == 0x04){
        cnt_flag = 1;
    }else{
        cnt_flag = 0;
    }
    // check target reg.
    u_int8_t tgt_reg = (input & 0x38) >> 3;
    switch(tgt_reg){
        case REG_NAME_STATSUS:
            // write status register.
            buffer[0] = handle->reg.reg0;
            write(cs, &buffer, 1);
            #ifdef PRINT_SOCK_COMM
                printf("write : %02hhx¥n", buffer);
            #endif

            break;
        case REG_NAME_CONFIGURATION:
            // read_flagがenableの時，必ずしも下のread関数が実行されない（configuration Byteが送られてこない）事があるはず
            if(read(cs, &in, 1) > 0){
                #ifdef PRINT_SOCK_COMM
                    printf("read : %02hhx¥n", in);
                #endif
                // change resolution.
                if((in & 0x80) == 0x80)  handle->reg.reg1 |= 0x80;
                if((in & 0x80) == 0x00)  handle->reg.reg1 &= 0x7f;

                // change mode.
                if((in & 0x60) == 0x00)  handle->reg.reg1 = (handle->reg.reg1 & 0x9F) | 0x00;   // set continuous mode.
                if((in & 0x60) == 0x20){
                    handle->reg.reg1 = (handle->reg.reg1 & 0x9F) | 0x20;   // set one shot mode.
                    set_temp(handle);
                    handle->reg.reg0 &= 0x7f;
                    buffer[0] = handle->reg.reg1;
                    if(write(cs, &buffer, 1) > 0){
                        #ifdef PRINT_SOCK_COMM
                            printf("write : %02hhx¥n", buffer);
                        #endif
                        handle->reg.reg0 |= 0x80;
                    }
                }  
                if((in & 0x60) == 0x40)  handle->reg.reg1 = (handle->reg.reg1 & 0x9F) | 0x40;   // set sps mode.
                if((in & 0x60) == 0x60)  handle->reg.reg1 = (handle->reg.reg1 & 0x9F) | 0x60;   // set shutdown mode.
            }

            break;
        case REG_NAME_TEMPERATIRE_VALUE:
            // write configuration register and set value.

            set_temp(handle);

            for(i = 0; i < 2; i++){
                buffer[i] = handle->reg.reg2[i];
                write(cs, &buffer[i], 1);
                #ifdef PRINT_SOCK_COMM
                    printf("write : %02hhx¥n", buffer);
                #endif
            }

            break;
        case REG_NAME_ID:
            //  return id.

            for(i = 0; i < 2; i++){
                buffer[i] = handle->reg.reg3[i];
                write(cs, &buffer[i], 1);
                #ifdef PRINT_SOCK_COMM
                    printf("write : %02hhx¥n", buffer);
                #endif
            }
            
            break;
        case REG_NAME_TCRIT_SETPOINT:
            // return Tcrit Setpoint.

            for(i = 0; i < 2; i++){
                buffer[i] = handle->reg.reg4[i];
                write(cs, &buffer[i], 1);
                #ifdef PRINT_SOCK_COMM
                    printf("write : %02hhx¥n", buffer);
                #endif
            }
            
            break;
        case REG_NAME_THYST_SETPOINT:
            // return Thyst Setpoint.

            for(i = 0; i < 2; i++){
                buffer[i] = handle->reg.reg5[i];
                write(cs, &buffer[i], 1);
                #ifdef PRINT_SOCK_COMM
                    printf("write : %02hhx¥n", buffer);
                #endif
            }
            
            break;
        case REG_NAME_THIGH_SETPOINT:
            // return Thigh Setpoint.

            for(i = 0; i < 2; i++){
                buffer[i] = handle->reg.reg6[i];
                write(cs, &buffer[i], 1);
                #ifdef PRINT_SOCK_COMM
                    printf("write : %02hhx¥n", buffer);
                #endif
            }
            
            break;
        case REG_NAME_TLOW_SETPOINT:
            // return Tlow Setpoint.

            for(i = 0; i < 2; i++){
                buffer[i] = handle->reg.reg7[i];
                write(cs, &buffer[i], 1);
                #ifdef PRINT_SOCK_COMM
                    printf("write : %02hhx¥n", buffer);
                #endif
            }
            
            break;
        default:
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
    int s, cs, l, i, out_len, buffer_ptr = 0;
    u_int8_t in, *out, buffer[BUFSIZ], mode;
    adt7310_t *handle;

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
        fprintf(stderr, "failed to initiallize mpl115a1 instance\n");
        exit(EXIT_FAILURE);
    }

    for(;;) {
        if((cs = accept(s, NULL, NULL)) == -1) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        fprintf(stderr, "connection established\n");
        for(;;) {
            // 入力受付
            if(read(cs, &in, 1) > 0){
                #ifdef PRINT_SOCK_COMM
                    printf("read : %02hhx¥n", in);
                #endif
                adt7310(handle, in, cs);
            }

            // 出力処理
            // chech mode
            mode = handle->reg.reg1 & 0x60;
            if((mode == 0x00) && ((handle->reg.reg0 & 0x80) == 0x00)){
                // continuous mode.
                set_temp(handle);

                for(i = 0; i < 2; i++){
                    buffer[i] = handle->reg.reg2[i];
                    write(cs, &buffer[i], 1);
                    #ifdef PRINT_SOCK_COMM
                        printf("write : %02hhx¥n", buffer);
                    #endif
                    handle->reg.reg0 |= 0x80;
                }
            }else if((mode == 0x20) && ((handle->reg.reg0 & 0x80) == 0x80)){
                // one shot mode.
            }else if(mode == 0x40){
                // sps mode.
            }else if(mode == 0x60){
                // shutdown mode.
            }
            sleep(SLEEP_TIME);
        }
        close(cs);
    }
    return 0;
}
