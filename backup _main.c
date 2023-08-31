/**
 * Copyright (c) 2021 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#define pswd_rang 4
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
#define remote_forward 0x09
#define remote_c 0x0d

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
// #include "raspberry26x32.h"
#include "ssd1306_font.h"
#include "OLED.h"

#include "nec_receive.h"
#include "hardware/pio.h"
#include "hardware/clocks.h" // for clock_get_hz()

struct render_area frame_area = {
    start_col : 0,
    end_col : SSD1306_WIDTH - 1,
    start_page : 0,
    end_page : SSD1306_NUM_PAGES - 1
};

int Trs_KeyV(uint8_t data, int pos, uint8_t *buf)
{
    int TrsData = -1;
    int pos_first = 0;
    int x_bit = 8;
    int current_pos = pos * x_bit;

    WriteString(buf, 5, 8, "                   ");
    switch (data)
    {
    case remote_num0:
        TrsData = 0;
        WriteString(buf, pos_first + current_pos, 8, "0");
        break;
    case remote_num1:
        TrsData = 1;
        WriteString(buf, pos_first + current_pos, 8, "1");
        break;
    case remote_num2:
        TrsData = 2;
        WriteString(buf, pos_first + current_pos, 8, "2");
        break;
    case remote_num3:
        TrsData = 3;
        WriteString(buf, pos_first + current_pos, 8, "3");
        break;
    case remote_num4:
        TrsData = 4;
        WriteString(buf, pos_first + current_pos, 8, "4");
        break;
    case remote_num5:
        TrsData = 5;
        WriteString(buf, pos_first + current_pos, 8, "5");
        break;
    case remote_num6:
        TrsData = 6;
        WriteString(buf, pos_first + current_pos, 8, "6");
        break;
    case remote_num7:
        TrsData = 7;
        WriteString(buf, pos_first + current_pos, 8, "7");
        break;
    case remote_num8:
        TrsData = 8;
        WriteString(buf, pos_first + current_pos, 8, "8");
        break;
    case remote_num9:
        TrsData = 9;
        WriteString(buf, pos_first + current_pos, 8, "9");
        break;
    }
    if (pos > 4)
    {
        pos_first = 0;
        current_pos = 5;
    }

    return TrsData;
}

int check_input(int input[], int pswd[])
{
    bool chk;
    for (int i = 0; i < 4; i++)
        if (input[i] != pswd[i])
            chk = 0;
        else
            chk = 1;
    return chk;
}
void clear_input(int input[])
{
    for (int i = 0; i < 4; i++)
    {
        input[i] = -1;
    }
    sleep_ms(100);
}

int main()
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

    // Initialize render area for entire frame (SSD1306_WIDTH pixels by SSD1306_NUM_PAGES pages)

    calc_render_area_buflen(&frame_area);

    // zero the entire display
    uint8_t buf[SSD1306_BUF_LEN];
    memset(buf, 0, SSD1306_BUF_LEN);
    render(buf, &frame_area);

    // IR
    PIO pio = pio0; // choose which PIO block to use (RP2040 has two: pio0 and pio1)
    uint8_t rx_address, rx_data;

    uint rx_gpio = 5; // choose which GPIO pin is connected to the IR detector

    // configure and enable the state machines
    int rx_sm = nec_rx_init(pio, rx_gpio); // uses one state machine and 9 instructions
    if (rx_sm == -1)
    {
        printf("could not configure PIO\n");
        return -1;
    }
    uint x_pos = 5, y_pos = 8;

    int pswd[4] = {1, 2, 3, 4};
    // int input[4] = {0};
    int input[4] = {1, 2, 3, 4};
    int cnt = 0;

    sleep_ms(100);

    while (true)
    {
        render(buf, &frame_area);
        WriteString(buf, 5, 0, "TEST");

        while (!pio_sm_is_rx_fifo_empty(pio, rx_sm))
        {
            uint32_t rx_frame = pio_sm_get(pio, rx_sm);
            nec_decode_frame(rx_frame, &rx_address, &rx_data);

            int setkey[] = {
                remote_num0,
                remote_num1,
                remote_num2,
                remote_num3,
                remote_num4,
                remote_num5,
                remote_num6,
                remote_num7,
                remote_num8,
                remote_num9,
            };
            for (int i = 0; i < 10; i++)
            {
                if (rx_data == setkey[i])
                {
                    cnt++;
                    input[cnt - 1] = Trs_KeyV(rx_data, cnt, buf);
                }
                else if (cnt > 4)
                {
                    WriteString(buf, 5, 8, "count overflow");
                    clear_input(input);
                    rx_data = 0;
                    cnt = 0;
                    x_pos = 5;
                    y_pos = 8;
                }
            }
        }

        if (rx_data == remote_play)
        {
            if (check_input(input, pswd) == 1)
            {
                WriteString(buf, 5, 8, "                   ");
                WriteString(buf, 5, 8, "  Correct   ");
            }
            else
            {
                WriteString(buf, 5, 8, "                   ");
                WriteString(buf, 5, 8, "  Wrong    ");
            }

            clear_input(input);
            rx_data = 0;
            cnt = 0;
            x_pos = 5;
            y_pos = 8;
        }
        if (rx_data == remote_return) // clear
        {
            WriteString(buf, 0, 0, "                   ");
            WriteString(buf, 5, 8, "                   ");
            WriteString(buf, 5, 16, "                   ");

            clear_input(input);
            rx_data = 0;
            cnt = 0;
            x_pos = 5;
            y_pos = 8;
        }

        rx_data = 0;
        if (x_pos >= 120)
        {
            x_pos = 5;
            y_pos += 8;
        }
    }
    return 0;
}