#include "../../include/user_control/init_UART.h"
#include "../../system.h"

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>//Atlas20250930
#include <sys/time.h>//Atlas20250930

// #include "../../include/canbus.h"
#include "../../8368-P/Demo/canbus.h"

#define UART_IDX_SPEED 2
#define UART_SPEED_SIZE 2

static uart_pattern_t uartPattern;
static uart_pattern_t uartPattern_ir;

static int fdUART, res, res1;

#define RECEIVE_CHARS 8 // 9

char buf[RECEIVE_CHARS] = "0";

struct timeval caltime[2];
float meterageEach;
float beforeSpeed;
float dtime = 0.0f;//Atlas20250926
static float calculate_time(struct timeval start, struct timeval end)
{
    //float dtime;
    dtime = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) * 0.000001;
    // printf("dtime %f\n", dtime);
    return dtime;
}

// int uart_fd_ir;
// int mode = 0;
// int keypad = 0;
// int frame_count = 0;
// int steer_flag = 0;
// int keypad_flag_count = 0;
static void *stm32_read_trans_thread(void *canBus)
{
    //    char *uartModeTmp = (char *)uartMode;
    can_bus_info_t canData;

    can_bus_info_t *canBusTmp = (can_bus_info_t *)canBus;

    char uartDevice[] = "/dev/ttyUSB0";
    // char uartDevice_ir[] = "/dev/ttyS3";

    uart_pattern_t uartPatternTmp = {
        uartDevice,
        0,
        921600,
        8,
        'N',
        1};

    // uart_pattern_t uartPatternTmp_ir = {
    //     uartDevice_ir,
    //     0,
    //     9600,
    //     8,
    //     'N',
    //     1};
    int speed_count = 0;
    uartPattern = uartPatternTmp;
    // uartPattern_ir = uartPatternTmp_ir;

    fdUART = init_uart_dev(&uartPattern);
    // uart_fd_ir = init_uart_dev(&uartPattern_ir);
    char ch[1];

    while (1)
    {
        res = read(fdUART, buf, RECEIVE_CHARS);
        // res1 = read(uart_fd_ir, ch, 1);

        if (res == RECEIVE_CHARS)
        {
            // AVM_LOGI("fdUART = %d res = %d\n", fdUART, res);
            // canBusTmp->keypad = buf[0];
            //            AVM_LOGI("buf = %c\n", buf[0]);
            //            AVM_LOGI("canBusTmp->keypad = %c\n", canBusTmp->keypad);
            //    AVM_LOGI("buf[0]= 0x%x ", buf[0]);

            //    AVM_LOGI("0x%x ", buf[1]);
            //    AVM_LOGI("0x%x ", buf[2]);
            //    AVM_LOGI("0x%x ", buf[3]);
            //    AVM_LOGI("0x%x ", buf[4]);
            //    AVM_LOGI("0x%x ", buf[5]);
            //    AVM_LOGI("0x%x ", buf[6]);
            //    AVM_LOGI("0x%x \n", buf[7]);
            //    AVM_LOGI("0x%x \n", buf[8]);

            //  AVM_LOGI("0x%x\n", buf[9]);

            // *uartModeTmp = buf[0];
            // AVM_LOGI("*uartModeTmp = 0x%x\n", *uartModeTmp);

            // 0x300
            canData.gear = (buf[1]);
            canBusTmp->gear = canData.gear;

            if (canBusTmp->gear == 0 || canBusTmp->gear == 16)
            {
                if (speed_count > 500)
                {
                    // 0x302
                    gettimeofday(&caltime[1], NULL);
                    dtime = calculate_time(caltime[0], caltime[1]);

                    // canData.speed = (buf[2] << 4) | (buf[3] >> 4);
                    canData.speed = (buf[2]);
                    canBusTmp->speed = (double)(canData.speed);

                    meterageEach = (beforeSpeed + canBusTmp->speed) * dtime / (2 * 3.6) * 0.95;

                    canBusTmp->meterage = canBusTmp->meterage + meterageEach;

                    beforeSpeed = canBusTmp->speed;
                    gettimeofday(&caltime[0], NULL);
                }
                speed_count++;
            }
            else
            {
                speed_count = 0;
                canBusTmp->meterage = 0;
                canBusTmp->speed = 0;
            }

            // 0x307
            //  canData.steer =(buf[4] << 8) | (buf[5]);

            canData.steer = (buf[4] << 8) | (buf[5]);
            int steer_temp = ((buf[4] << 8) | (buf[5])) & 0x200;
            // printf("steer_temp = %d\n",steer_temp);
            if (steer_temp == 0)
            {
                canBusTmp->steer = (double)(canData.steer) * 0.1002 * -1;
            }
            else
            {
                canBusTmp->steer = (double)(4095 - canData.steer) * 0.1002;
            }

            // 0x309
            canData.turningSignal = (buf[6] & 0x30) >> 4;
            canBusTmp->turningSignal = canData.turningSignal;

            // canBusTmp->keypad = buf[7];

            // AVM_LOGI("canBusTmp->gear = 0x%x\n", canBusTmp->gear);
            // AVM_LOGI("canBusTmp->speed = %.3f\n", canBusTmp->speed);
            // AVM_LOGI("canBusTmp->steer = %.3f\n", canBusTmp->steer);
            // AVM_LOGI("canBusTmp->turningSignal = 0x%x\n\n", canBusTmp->turningSignal);
        }
        else
        {
            //			AVM_LOGI("read no uart2\n");
        }

        // if (res1 > 0)
        // {

        //     // if (ch[0] == 1)
        //     //     mode = 1;
        //     // else if (ch[0] == 253 && mode == 1)
        //     //     mode = 2;

        //     if (ch[0] == 27)
        //     {
        //         // canBusTmp->keypad = 0x32;
        //         keypad = 1;
        //         mode = 0;
        //         canBusTmp->keypad = 0x32;
        //     }
        //     if (ch[0] == 26)
        //     {
        //         // canBusTmp->keypad = 0x33;
        //         keypad = 2;
        //         mode = 0;
        //         canBusTmp->keypad = 0x33;
        //     }
        //     if (ch[0] == 4)
        //     {
        //         // canBusTmp->keypad = 0x34;
        //         keypad = 3;
        //         mode = 0;
        //         canBusTmp->keypad = 0x34;
        //     }
        //     if (ch[0] == 6)
        //     {
        //         // canBusTmp->keypad = 0x35;
        //         keypad = 4;
        //         mode = 0;
        //         canBusTmp->keypad = 0x35;
        //     }
        //     if (ch[0] == 206)
        //     {
        //         // canBusTmp->keypad = 0x36;
        //         keypad = 5;
        //         mode = 0;
        //     }
        //     if (ch[0] == 152)
        //     {
        //         // canBusTmp->keypad = 0x37;
        //         keypad = 6;
        //         mode = 0;
        //     }
        //     if (ch[0] == 131)
        //     {
        //         // canBusTmp->keypad = 0x38;
        //         keypad = 7;
        //         mode = 0;
        //     }
        //     if (ch[0] == 156)
        //     {
        //         // canBusTmp->keypad = 0x39;
        //         keypad = 8;
        //         mode = 0;
        //     }
        //     if (ch[0] == 135)
        //     {
        //         // canBusTmp->keypad = 0x40;
        //         keypad = 9;
        //         mode = 0;
        //     }
        //     if (ch[0] == 130)
        //     {
        //         // canBusTmp->keypad = 0x41;
        //         keypad = 10;
        //         mode = 0;
        //     }
        //     printf("ch[0] = %d\n", ch[0]);
        //     // printf("keypad = %d\n", keypad);
        //     keypad_flag_count = 100;
        //     // printf("mode = %d\n",mode);
        // }
        // else
        // {
        //     if(keypad_flag_count == 0)
        //         canBusTmp->keypad = 0x00;

        //     if(keypad_flag_count > 0)
        //         keypad_flag_count --;
        // }
        /*
        int numW = write(fdUART, buf, sizeof buf);
                if(numW > 0)
                {

                    printf("numW %d\n", numW);
                }
                else
                {
                    perror("Transmit error");
                }
        */
        usleep(1000 * 5);
    }
}
// static void * stm32_read_trans_thread1(void *canBus)
// {
// //    char *uartModeTmp = (char *)uartMode;
//     can_bus_info_t canData;

//     can_bus_info_t* canBusTmp = (can_bus_info_t *)canBus;

//     // char uartDevice[] = "/dev/ttyS7";
//     char uartDevice_ir[] = "/dev/ttyUSB0";

//     // uart_pattern_t uartPatternTmp = {
//     //     uartDevice,
//     //     0,
//     //     921600,
//     //     8,
//     //     'N',
//     //     1
//     // };

//     uart_pattern_t uartPatternTmp_ir = {
//         uartDevice_ir,
//         0,
//         9600,
//         8,
//         'N',
//         1};

//     // uartPattern = uartPatternTmp;
//     uartPattern_ir = uartPatternTmp_ir;

//     // fdUART = init_uart_dev(&uartPattern);
//     uart_fd_ir = init_uart_dev(&uartPattern_ir);
//     char ch[1];

//     while (1)
//     {
//         // res = read(fdUART, buf, RECEIVE_CHARS);
//         res1 = read(uart_fd_ir, ch, 1);

//         if (res1 > 0)
//         {

//             // if (ch[0] == 1)
//             //     mode = 1;
//             // else if (ch[0] == 253 && mode == 1)
//             //     mode = 2;

//             if (ch[0] == 27)
//             {
//                 // canBusTmp->keypad = 0x32;
//                 keypad = 1;
//                 mode = 0;
//                 canBusTmp->keypad = 0x32;
//             }
//             if (ch[0] == 26)
//             {
//                 // canBusTmp->keypad = 0x33;
//                 keypad = 2;
//                 mode = 0;
//                 canBusTmp->keypad = 0x33;
//             }
//             if (ch[0] == 4)
//             {
//                 // canBusTmp->keypad = 0x34;
//                 keypad = 3;
//                 mode = 0;
//                 canBusTmp->keypad = 0x34;
//             }
//             if (ch[0] == 6)
//             {
//                 // canBusTmp->keypad = 0x35;
//                 keypad = 4;
//                 mode = 0;
//                 canBusTmp->keypad = 0x35;
//             }
//             if (ch[0] == 5)
//             {
//                 // canBusTmp->keypad = 0x36;
//                 keypad = 5;
//                 mode = 0;
//                 canBusTmp->keypad = 0x30;
//             }
//             if (ch[0] == 18)
//             {
//                 // canBusTmp->keypad = 0x37;
//                 keypad = 6;
//                 mode = 0;
//                 canBusTmp->keypad = 0x31;
//             }
//             if (ch[0] == 131)
//             {
//                 // canBusTmp->keypad = 0x38;
//                 keypad = 7;
//                 mode = 0;
//             }
//             if (ch[0] == 156)
//             {
//                 // canBusTmp->keypad = 0x39;
//                 keypad = 8;
//                 mode = 0;
//             }
//             if (ch[0] == 135)
//             {
//                 // canBusTmp->keypad = 0x40;
//                 keypad = 9;
//                 mode = 0;
//             }
//             if (ch[0] == 130)
//             {
//                 // canBusTmp->keypad = 0x41;
//                 keypad = 10;
//                 mode = 0;
//             }
//             printf("ch[0] = %d\n", ch[0]);
//             // printf("keypad = %d\n", keypad);
//             // keypad_flag_count = 3000;
//             // printf("mode = %d\n",mode);
//         }
//         else
//         {
//             // if(keypad_flag_count == 0 )
//             //     canBusTmp->keypad = 0x00;

//             // if(keypad_flag_count > 0)
//             //     keypad_flag_count --;
//         }
// /*
// int numW = write(fdUART, buf, sizeof buf);
//         if(numW > 0)
//         {

//             printf("numW %d\n", numW);
//         }
//         else
//         {
//             perror("Transmit error");
//         }
// */
//         usleep(10* 5);
//     }
// }
void stm32_read_trans_thread_start(can_bus_info_t *canBus)
{
    pthread_t threadSTM32Transmit;
    pthread_create(&threadSTM32Transmit, NULL, &stm32_read_trans_thread, canBus);
    // pthread_create(&threadSTM32Transmit, NULL, &stm32_read_trans_thread1, canBus);
}
