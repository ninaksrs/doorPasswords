/**
 * Copyright (c) 2021 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#define remote_num0 0x16
#define remote_num1 0x0c
#define remote_num2 0x18
#define remote_num3 0x5e
#define remote_num4 0x08
#define remote_num5 0x1c
#define remote_num6 0x5a
#define remote_num7 0x42
#define remote_num8 0x52
#define remote_num9 0x4a
#define remote_power 0x45
#define remote_menu 0x47
#define remote_test 0x44
#define remote_plus 0x40
#define remote_minus 0x19
#define remote_return 0x43
#define remote_backward 0x07
#define remote_play 0x15
#define remote_forword 0x09
#define remote_c 0x0d

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "ssd1306_font.h"
#include "nec_receive.h"

#include "stepper_motor.h"
#include "OLED.h"

#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"

int Trs_KeyV(uint8_t data, int pos, uint8_t *buf);
int check_input(int *input, int *pswd);

void init_HW()
{
    // I2C is "open drain", pull ups to keep signal high when no data is being
    // sent
    i2c_init(i2c_default, SSD1306_I2C_CLK * 1000);
    gpio_set_function(PICO_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_I2C_SDA_PIN);
    gpio_pull_up(PICO_I2C_SCL_PIN);

    // run through the complete initialization process
    SSD1306_init();
}

int main()
{
    // IR receive
    PIO pio = pio0;
    uint8_t rx_address, rx_data;
    uint rx_gpio = 5;
    int rx_sm = nec_rx_init(pio, rx_gpio);

    if (rx_sm == -1)
    {
        printf("could not configure PIO\n");
        return -1;
    }

    // OLED
    uint x_pos = 5, y_pos = 8;
    init_HW();

    // Initialize render area for entire frame (SSD1306_WIDTH pixels by SSD1306_NUM_PAGES pages)

    calc_render_area_buflen(&frame_area);

    // zero the entire display
    uint8_t buf[SSD1306_BUF_LEN];
    memset(buf, 0, SSD1306_BUF_LEN);
    render(buf, &frame_area);

    // Init LED pin Motor
    init_port(In1_pin, GPIO_OUT);
    init_port(In2_pin, GPIO_OUT);
    init_port(In3_pin, GPIO_OUT);
    init_port(In4_pin, GPIO_OUT);
    sleep_ms(100);

    // Create Passwords
    int pswd[4] = {1, 2, 3, 4};
    int input[4] = {0};

    while (true)
    {
        /* // IR receive
        while (!pio_sm_is_rx_fifo_empty(pio, rx_sm))
        {
            uint32_t rx_frame = pio_sm_get(pio, rx_sm);

            if (nec_decode_frame(rx_frame, &rx_address, &rx_data))
                printf("\treceived: %02x, %02x\n", rx_address, rx_data);
            else
                printf("\treceived: %08x\n", rx_frame);
        } */

        // Show Display
        render(buf, &frame_area);

        /*  char *text[] = {
             "  Smart Lazy",
             "    Welcome"};

         int y = 0;
         for (int i = 0; i < count_of(text); i++)
         {
             WriteString(buf, 5, y, text[i]);
             y += 8;
         } */

        WriteString(buf, 5, 0, "  Smart Lazy");
        render(buf, &frame_area);

        // Prof's code
        get_input(input, buf);
        if (check_input(input, pswd))
            WriteString(buf, 5, 16, "Correct!");
        else
            WriteString(buf, 5, 16, "Wrong!");
        render(buf, &frame_area);
        // Remote with display

        if (rx_data == remote_return) // clear
        {
            WriteString(buf, 0, 0, "                   ");
            WriteString(buf, 5, 8, "                   ");
            WriteString(buf, 5, 16, "                   ");

            x_pos = 5;
            y_pos = 8;

            // TODO clear answer and getpwd
        }
        else if (rx_data == remote_plus)
        {
            WriteString(buf, 5, 8, "                   ");
            WriteString(buf, 5, 8, "  PWD Correct");
            onSMotor(1);
            x_pos = 5;
            y_pos = 8;
        }
        else if (rx_data == remote_minus)
        {
            WriteString(buf, 5, 8, "                   ");
            WriteString(buf, 5, 8, "  Door Closed");
            offSMotor(1);
            x_pos = 5;
            y_pos = 8;
        }

        else if (rx_data == remote_play)
        {

            if (check(passwords, answer) == 1)
            {
                WriteString(buf, 5, 8, "                   ");
                WriteString(buf, 5, 8, "   Correct PWD  ");
                onSMotor(1);
                sleep_ms(3000);
                offSMotor(1);
                sleep_ms(200);
            }
            else
            {
                WriteString(buf, 5, 8, "                   ");
                WriteString(buf, 5, 8, "   Wrong PWD   ");
            }
            x_pos = 5;
            y_pos = 8;

            // TODO clear answer and getpwd
        }

        else if (rx_data == remote_test)
        {
            /* WriteString(buf, 5, 8, "                   ");
            WriteString(buf, 5, 8, "    Welcome");
            x_pos = 5;
            y_pos = 8; */

            // when press test Botton

            int y = 8;

            for (int i = 0; i < count_of(pswd); i++)
            {
                WriteString(buf, 5, y, ((char *)pswd[i]));
                y += 8;
            }

            // still not works
            for (int i = 0; i < count_of(input); i++)
            {
                WriteString(buf, 5, y, ((char *)input[i]));
                y += 8;
            }
        }

        /*
       else if (rx_data == 0x07)
       {
           WriteString(buf, 5, 8, "                   ");
           WriteString(buf, 5, 8, "   Wrong PWD   ");
           x_pos = 5;
           y_pos = 8;
       }*/

        rx_data = 0;
        if (x_pos >= 120)
        {
            x_pos = 5;
            y_pos += 8;
        }
    }
    return 0;
}

int Trs_KeyV(uint8_t data, int pos, uint8_t *buf)
{
    int TrsData = -1;

    switch (data)
    {
    case remote_num0:
        TrsData = 0;
        WriteString(buf, 5 + (pos * 9), 8, "0");
        break;
    case remote_num1:
        TrsData = 1;
        WriteString(buf, 5 + (pos * 9), 8, "1");
        break;
    case remote_num2:
        TrsData = 2;
        WriteString(buf, 5 + (pos * 9), 8, "2");
        break;
    case remote_num3:
        TrsData = 3;
        WriteString(buf, 5 + (pos * 9), 8, "3");
        break;
    case remote_num4:
        TrsData = 4;
        WriteString(buf, 5 + (pos * 9), 8, "4");
        break;
    case remote_num5:
        TrsData = 5;
        WriteString(buf, 5 + (pos * 9), 8, "5");
        break;
    case remote_num6:
        TrsData = 6;
        WriteString(buf, 5 + (pos * 9), 8, "6");
        break;
    case remote_num7:
        TrsData = 7;
        WriteString(buf, 5 + (pos * 9), 8, "7");
        break;
    case remote_num8:
        TrsData = 8;
        WriteString(buf, 5 + (pos * 9), 8, "8");
        break;
    case remote_num9:
        TrsData = 9;
        WriteString(buf, 5 + (pos * 9), 8, "9");
        break;
    }
    render(buf, &frame_area);
    return TrsData;
}

int get_input(int *input, uint8_t *buf)
{
    PIO pio = pio0; // choose which PIO block to use (RP2040 has two: pio0 and pio1)
    int cnt = 0;
    uint8_t rx_address, rx_data;

    uint rx_gpio = 5; // choose which GPIO pin is connected to the IR detector

    // configure and enable the state machines
    int rx_sm = nec_rx_init(pio, rx_gpio); // uses one state machine and 9 instructions

    if (rx_sm == -1)
    {
        printf("could not configure PIO\n");
        return -1;
    }

    while (1)
    {

        while (!pio_sm_is_rx_fifo_empty(pio, rx_sm))
        {
            uint32_t rx_frame = pio_sm_get(pio, rx_sm);
            nec_decode_frame(rx_frame, &rx_address, &rx_data);
            cnt++;
        }

        input[cnt - 1] = Trs_KeyV(rx_data, cnt, buf);

        if (cnt == 4)
            return 0;
    }
}

int check_input(int *input, int *pswd)
{

    for (int i = 0; i < 4; i++)
        if (input[i] != pswd[i])
            return 0;

    return 1;
}

/* else if (rx_data == remote_num0)
        {
            WriteString(buf, x_pos, y_pos, "x"); // 0
            x_pos += 8;
        }
        else if (rx_data == remote_num1)
        {
            WriteString(buf, x_pos, y_pos, "x"); // 1
            x_pos += 8;
        }
        else if (rx_data == remote_num2)
        {
            WriteString(buf, x_pos, y_pos, "x"); // 2
            x_pos += 8;
        }
        else if (rx_data == remote_num3)
        {
            WriteString(buf, x_pos, y_pos, "x"); // 3
            x_pos += 8;
        }
        else if (rx_data == remote_num4)
        {
            WriteString(buf, x_pos, y_pos, "x"); // 4
            x_pos += 8;
        }
        else if (rx_data == remote_num5)
        {
            WriteString(buf, x_pos, y_pos, "x"); // 5
            x_pos += 8;
        }
        else if (rx_data == remote_num6)
        {
            WriteString(buf, x_pos, y_pos, "x"); // 6
            x_pos += 8;
        }
        else if (rx_data == remote_num7)
        {
            WriteString(buf, x_pos, y_pos, "x"); // 7
            x_pos += 8;
        }
        else if (rx_data == remote_num8)
        {
            WriteString(buf, x_pos, y_pos, "x"); // 8
            x_pos += 8;
        }
        else if (rx_data == remote_num9)
        {
            WriteString(buf, x_pos, y_pos, "x"); // 9
            x_pos += 8;
        } */