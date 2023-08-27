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
#include "hardware/pio.h"
#include "ssd1306_font.h"
#include "nec_receive.h"

#include "stepper_motor.h"
#include "OLED.h"
/* #include "basic_divices_gpio.h"
#include "dc_motor.h"
#include "buzzer_song.h" */

#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"

int check(char s[], char a[], char ch)
{
    int i;
    for (i = 0; s[i] != '\0'; i++)
    {
        if (s[i] == ch)
        {
            a[i] = ch;
        }
        else if (s[i] == ' ')
        {
            a[i] = ' ';
        }
    }
    // strcmp = 0 = equal.
    if (strcmp(s, a) == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
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
    /*  // BTN
     const uint btn = 3;
     uint countsound = 0; */

    // IR receive
    PIO pio = pio0;
    uint rx_gpio = 5;
    int rx_sm = nec_rx_init(pio, rx_gpio);
    uint8_t rx_address, rx_data;
    uint x_pos = 5, y_pos = 8;

    // OLED
    init_HW();

    if (rx_sm == -1)
    {
        printf("could not configure PIO\n");
        return -1;
    }

    // Initialize render area for entire frame (SSD1306_WIDTH pixels by SSD1306_NUM_PAGES pages)
    struct render_area frame_area = {
        start_col : 0,
        end_col : SSD1306_WIDTH - 1,
        start_page : 0,
        end_page : SSD1306_NUM_PAGES - 1
    };

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

    // Init Devices
    /*   init_soundSensor();
      initialize_pwm_channel(GPIO_DC);
      initBuzzer();
      init_port(btn, GPIO_IN);
      gpio_pull_up(btn); */

    // Create Passwords
    char passwords[] = {"23"};
    char answer[] = {0};
    char getpwd;
    int size_pwd = sizeof(answer) / sizeof(answer[0]);

    // maybe for dc motor
    /*  uint wrap;
     wrap = set_frequency(261);
     set_duty(0, wrap);
     pwm_set_enabled(SLICE_NUM, false);
     sleep_ms(100); */

    while (true)
    {
        // IR receive
        while (!pio_sm_is_rx_fifo_empty(pio, rx_sm))
        {
            uint32_t rx_frame = pio_sm_get(pio, rx_sm);

            if (nec_decode_frame(rx_frame, &rx_address, &rx_data))
                printf("\treceived: %02x, %02x\n", rx_address, rx_data);
            else
                printf("\treceived: %08x\n", rx_frame);
        }
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

        // Remote with display
        if (rx_data == 0x16)
        {
            WriteString(buf, x_pos, y_pos, "x"); // 0
            x_pos += 8;
        }
        else if (rx_data == 0x43) // clear
        {
            WriteString(buf, 0, 0, "                   ");
            WriteString(buf, 5, 8, "                   ");
            WriteString(buf, 5, 16, "                   ");

            x_pos = 5;
            y_pos = 8;
        }
        else if (rx_data == 0x40)
        {
            WriteString(buf, 5, 8, "                   ");
            WriteString(buf, 5, 8, "  PWD Correct");
            onSMotor(1);
            x_pos = 5;
            y_pos = 8;
        }
        else if (rx_data == 0x19)
        {
            WriteString(buf, 5, 8, "                   ");
            WriteString(buf, 5, 8, "  Door Closed");
            offSMotor(1);
            x_pos = 5;
            y_pos = 8;
        }
        else if (rx_data == 0x0c)
        {
            WriteString(buf, x_pos, y_pos, "x"); // 1
            x_pos += 8;
        }
        else if (rx_data == 0x18)
        {
            getpwd += '2';

            sleep_ms(100);

            WriteString(buf, x_pos, y_pos, "x"); // 2
            x_pos += 8;
        }
        else if (rx_data == 0x5e)
        {
            WriteString(buf, x_pos, y_pos, "x"); // 3
            x_pos += 8;
        }
        else if (rx_data == 0x08)
        {
            WriteString(buf, x_pos, y_pos, "x"); // 4
            x_pos += 8;
        }
        else if (rx_data == 0x1c)
        {
            WriteString(buf, x_pos, y_pos, "x"); // 5
            x_pos += 8;
        }
        else if (rx_data == 0x5a)
        {
            WriteString(buf, x_pos, y_pos, "x"); // 6
            x_pos += 8;
        }
        else if (rx_data == 0x42)
        {
            WriteString(buf, x_pos, y_pos, "x"); // 7
            x_pos += 8;
        }
        else if (rx_data == 0x52)
        {
            WriteString(buf, x_pos, y_pos, "x"); // 8
            x_pos += 8;
        }
        else if (rx_data == 0x4a)
        {
            WriteString(buf, x_pos, y_pos, "x"); // 9
            x_pos += 8;
        }
        else if (rx_data == 0x15)
        {
            /* if (check(passwords, answer, getpwd) == 1)
            {
                onSMotor(1);
                sleep_ms(3000);
                offSMotor(1);
                sleep_ms(200);
            } */
            WriteString(buf, 5, 8, "                   ");

            WriteString(buf, 5, 8, "  CHECKIN PWD"); // Play
            x_pos = 5;
            y_pos = 8;
        }
        else if (rx_data == 0x44)
        {
            WriteString(buf, 5, 8, "                   ");
            WriteString(buf, 5, 8, "    Welcome"); // 9
            x_pos = 5;
            y_pos = 8;
        }
        else if (rx_data == 0x07)
        {
            WriteString(buf, 5, 8, "                   ");
            WriteString(buf, 5, 8, "   Wrong PWD   "); // 9
            x_pos = 5;
            y_pos = 8;
        }
        // mycode ....

        rx_data = 0;
        if (x_pos >= 120)
        {
            x_pos = 5;
            y_pos += 8;
        }
    }
    return 0;
}

/* if (rx_data == 0x16)
        {
            WriteString(buf, x_pos, y_pos, "0");
            x_pos += 8;
            onSMotor(1);
        }
        else if (rx_data == 0x15)
        {
            WriteString(buf, x_pos, y_pos, " ");
            x_pos += 8;
        }
        else if (rx_data == 0x0c)
        {
            WriteString(buf, x_pos, y_pos, "1");
            x_pos += 8;
            offSMotor(1);
        }
        else if (rx_data == 0x18)
        {
            WriteString(buf, x_pos, y_pos, "2");
            x_pos += 8;
        }
        else if (rx_data == 0x5e)
        {
            WriteString(buf, x_pos, y_pos, "3");
            x_pos += 8;
        }
        else if (rx_data == 0x08)
        {
            WriteString(buf, x_pos, y_pos, "4");
            x_pos += 8;
        }
        else if (rx_data == 0x1c)
        {
            WriteString(buf, x_pos, y_pos, "5");
            x_pos += 8;
        }
        else if (rx_data == 0x5a)
        {
            WriteString(buf, x_pos, y_pos, "6");
            x_pos += 8;
        }
        else if (rx_data == 0x42)
        {
            WriteString(buf, x_pos, y_pos, "7");
            x_pos += 8;
        }
        else if (rx_data == 0x52)
        {
            WriteString(buf, x_pos, y_pos, "8");
            x_pos += 8;
        }
        else if (rx_data == 0x4a)
        {
            WriteString(buf, x_pos, y_pos, "9");
            x_pos += 8;
        } */
