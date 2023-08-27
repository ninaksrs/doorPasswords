
const uint In1_pin = 16;
const uint In2_pin = 15;
const uint In3_pin = 14;
const uint In4_pin = 13;

uint Round_Value = 2048; // 4096

void init_port(int port, int dir)
{
    gpio_init(port);
    gpio_set_dir(port, dir);
}

void Reset_Motor()
{
    gpio_put(In1_pin, 0);
    gpio_put(In2_pin, 0);
    gpio_put(In3_pin, 0);
    gpio_put(In4_pin, 0);
}

int Step_Value[8][4] = {
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1},
};

void Step_run(int count, int value[][4])
{
    int dir = 1; // Clockwise
    int i;

    if (count < 0)
        dir = 0; // counterclockwise
    Reset_Motor();

    if (dir)
    {
        for (i = 0; i < count; i++)
        {
            gpio_put(In1_pin, value[i % 8][0]);
            gpio_put(In2_pin, value[i % 8][1]);
            gpio_put(In3_pin, value[i % 8][2]);
            gpio_put(In4_pin, value[i % 8][3]);
            sleep_ms(1);
        }
    }
    else
    {
        for (i = -count; i > 0; i--)
        {
            gpio_put(In1_pin, value[i % 8][0]);
            gpio_put(In2_pin, value[i % 8][1]);
            gpio_put(In3_pin, value[i % 8][2]);
            gpio_put(In4_pin, value[i % 8][3]);
            sleep_ms(1);
        }
    }
}

void offSMotor(int count)
{

    for (int i = 0; i < count; i++)
    {

        Step_run(-Round_Value, Step_Value);
        sleep_ms(1);
    }
}

void onSMotor(int count)
{

    for (int i = 0; i < count; i++)
    {

        Step_run(Round_Value, Step_Value);
        sleep_ms(1);
    }
}
/* int main()
{
    int Step_Value[8][4] = {
        {1, 0, 0, 0},
        {1, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 1},
        {0, 0, 0, 1},
        {1, 0, 0, 1}};

    // Init LED pin
    init_port(In1_pin, GPIO_OUT);
    init_port(In2_pin, GPIO_OUT);
    init_port(In3_pin, GPIO_OUT);
    init_port(In4_pin, GPIO_OUT);

    // Init serial port
    stdio_init_all();

    // Loop
    while (true)
    {
        Step_run(Round_Value, Step_Value);
        sleep_ms(1000);
        Step_run(-Round_Value, Step_Value);
        sleep_ms(1000);
    }
    return 0;
} */