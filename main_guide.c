/**
 * Copyright (c) 2021 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "raspberry26x32.h"
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

    switch (data)
    {
    case 0x16:
        TrsData = 0;
        WriteString(buf, 5 + (pos * 9), 8, "0");
        break;
    case 0x0c:
        TrsData = 1;
        WriteString(buf, 5 + (pos * 9), 8, "1");
        break;
    case 0x18:
        TrsData = 2;
        WriteString(buf, 5 + (pos * 9), 8, "2");
        break;
    case 0x5e:
        TrsData = 3;
        WriteString(buf, 5 + (pos * 9), 8, "3");
        break;
    case 0x08:
        TrsData = 4;
        WriteString(buf, 5 + (pos * 9), 8, "4");
        break;
    case 0x1c:
        TrsData = 5;
        WriteString(buf, 5 + (pos * 9), 8, "5");
        break;
    case 0x5a:
        TrsData = 6;
        WriteString(buf, 5 + (pos * 9), 8, "6");
        break;
    case 0x42:
        TrsData = 7;
        WriteString(buf, 5 + (pos * 9), 8, "7");
        break;
    case 0x52:
        TrsData = 8;
        WriteString(buf, 5 + (pos * 9), 8, "8");
        break;
    case 0x4a:
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

    int pswd[4] = {1, 2, 3, 4};
    int input[4] = {0};

    while (true)
    {

        WriteString(buf, 5, 0, "TEST");
        render(buf, &frame_area);

        get_input(input, buf);
        if (check_input(input, pswd))
            WriteString(buf, 5, 16, "Correct!");
        else
            WriteString(buf, 5, 16, "Wrong!");
        render(buf, &frame_area);
    }

    return 0;
}
