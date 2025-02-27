#include "hardware/clocks.h"
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "pio_matrix.pio.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define I2C_ADDR 0x3C

// LEDs (PWM)
#define LED_VERDE 11
#define LED_AZUL 12
#define LED_VERMELHO 13

// Joystick (ADC)
#define JOYSTICK_X 26
#define JOYSTICK_Y 27

// Botões
#define BUTTON_JOY 22
#define BUTTON_A 5
#define BUTTON_B 6

// Buzzer
#define BUZZER_PIN_1 10
#define BUZZER_PIN_2 21

// Matriz de LEDs 5x5
#define MATRIX 7
#define NUM_PIXELS 25

ssd1306_t display;

// Variáveis globais para controle dos botões e estados da caldeira
volatile bool boiler_on = false;
volatile bool safe_mode = false;
volatile bool buttonA_pressed_flag = false;
volatile bool buttonB_pressed_flag = false;

// Variáveis para debounce (em microsegundos)
volatile uint64_t last_buttonA_time = 0;
volatile uint64_t last_buttonB_time = 0;

void update_matrix(const double *pattern, PIO pio, uint sm)
{
    uint32_t value;
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        value = ((uint32_t)(pattern[i] * 5) << 8);
        pio_sm_put_blocking(pio, sm, value);
    }
}

void setup_pwm(uint gpio)
{
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice, 4095);
    pwm_set_chan_level(slice, pwm_gpio_to_channel(gpio), 0);
    pwm_set_enabled(slice, true);
}

void set_led_pwm(uint gpio, uint16_t level)
{
    uint slice = pwm_gpio_to_slice_num(gpio);
    pwm_set_chan_level(slice, pwm_gpio_to_channel(gpio), level);
}

// Callback para as interrupções dos botões A e B
void gpio_callback(uint gpio, uint32_t events)
{
    if (gpio == BUTTON_A)
    {
        buttonA_pressed_flag = true;
    }
    else if (gpio == BUTTON_B)
    {
        buttonB_pressed_flag = true;
    }
}

void setup()
{
    stdio_init_all();
    sleep_ms(3000);
    printf("Iniciando setup...\n");

    setup_pwm(LED_AZUL);
    setup_pwm(LED_VERDE);
    setup_pwm(LED_VERMELHO);

    PIO pio = pio0;
    uint offset = pio_add_program(pio, &pio_matrix_program);
    uint sm = pio_claim_unused_sm(pio, true);
    pio_matrix_program_init(pio, sm, offset, MATRIX);

    adc_init();
    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);

    gpio_init(BUTTON_JOY);
    gpio_set_dir(BUTTON_JOY, GPIO_IN);
    gpio_pull_up(BUTTON_JOY);

    // Configuração do botão A com interrupção
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    // Configuração do botão B com interrupção (usa o mesmo callback)
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);
    gpio_set_irq_enabled(BUTTON_B, GPIO_IRQ_EDGE_FALL, true);

    gpio_init(BUZZER_PIN_1);
    gpio_set_dir(BUZZER_PIN_1, GPIO_OUT);
    gpio_put(BUZZER_PIN_1, 0);

    gpio_init(BUZZER_PIN_2);
    gpio_set_dir(BUZZER_PIN_2, GPIO_OUT);
    gpio_put(BUZZER_PIN_2, 0);

    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init(&display, 128, 64, false, I2C_ADDR, I2C_PORT);
    ssd1306_config(&display);
    ssd1306_fill(&display, false);
    ssd1306_send_data(&display);

    printf("Display inicializado.\n");
    printf("Setup completo.\n");
}

int main()
{
    setup();

    // Parâmetros da caldeira
    float temperature = 200.0;
    const float max_temperature = 400.0;
    const float critical_temperature = 350.0;
    float pressure = 50.0;
    const float max_pressure = 200.0;
    const float critical_pressure = 180.0;

    // Valores ideais de trabalho no modo seguro
    const float safe_temperature_base = 250.0;
    const float safe_pressure_base = 80.0;

    // Temperatura máxima para desligamento seguro
    const float safe_shutdown_temperature = 230.0;

    uint16_t adc_val = 0;
    bool valve_open = false;

    // Variáveis para variar os targets no modo seguro
    static int safe_counter = 0;
    static float safe_temp_target = safe_temperature_base;
    static float safe_press_target = safe_pressure_base;

    while (1)
    {
        uint64_t now = time_us_64(); // tempo atual em microsegundos

        // Processamento das interrupções dos botões com debounce de 300ms (300000us)
        if (buttonA_pressed_flag)
        {
            if (now - last_buttonA_time >= 300000)
            {
                // Botão A: Liga/Desliga a caldeira
                if (!boiler_on)
                {
                    boiler_on = true;
                }
                else
                {
                    if (safe_mode || temperature <= safe_shutdown_temperature)
                    {
                        boiler_on = false;
                    }
                }
                last_buttonA_time = now;
            }
            buttonA_pressed_flag = false;
        }

        if (buttonB_pressed_flag)
        {
            if (now - last_buttonB_time >= 300000)
            {
                // Botão B: Alterna o modo de trabalho automático e seguro
                safe_mode = !safe_mode;
                last_buttonB_time = now;
            }
            buttonB_pressed_flag = false;
        }

        if (boiler_on)
        {
            if (safe_mode)
            {
                // No modo seguro, atualiza os targets com pequenas variações, mantendo-os dentro de uma faixa segura.
                safe_counter++;
                if (safe_counter > 50)
                {                                                                   // Aproximadamente a cada 5 segundos (com sleep_ms(100))
                    safe_temp_target = safe_temperature_base + ((rand() % 11) - 5); // variação de -5 a +5
                    safe_press_target = safe_pressure_base + ((rand() % 11) - 5);   // variação de -5 a +5
                    safe_counter = 0;
                }
                // Ajusta gradualmente a temperatura e pressão para os targets atuais
                if (temperature < safe_temp_target)
                    temperature += 0.5;
                else if (temperature > safe_temp_target)
                    temperature -= 0.5;

                if (pressure < safe_press_target)
                    pressure += 1.0;
                else if (pressure > safe_press_target)
                    pressure -= 1.0;
            }
            else
            {
                // Modo normal: controle via joystick (válvula)
                adc_select_input(1);
                adc_val = adc_read();
                int valve_val = (adc_val * 100) / 4095;
                valve_open = (valve_val > 50);

                if (valve_open)
                {
                    temperature -= 5.0;
                    pressure -= 10.0;
                    if (temperature < 150.0)
                        temperature = 150.0;
                    if (pressure < 10.0)
                        pressure = 10.0;
                }
                else
                {
                    temperature += 2.0;
                    pressure += 5.0;
                    if (temperature > max_temperature)
                        temperature = max_temperature;
                    if (pressure > max_pressure)
                        pressure = max_pressure;
                }
            }
        }
        else
        {
            // Caldeira desligada: diminui gradualmente temperatura e pressão
            if (temperature > 150.0)
            {
                temperature -= 1.0;
                if (temperature < 150.0)
                    temperature = 150.0;
            }
            if (pressure > 10.0)
            {
                pressure -= 2.0;
                if (pressure < 10.0)
                    pressure = 10.0;
            }
        }

        // Atualiza o display somente se a caldeira estiver ligada
        if (boiler_on)
        {
            ssd1306_fill(&display, false);
            ssd1306_rect(&display, 0, 0, 127, 63, true, false);

            char buffer[50];
            sprintf(buffer, "Temp: %.1fC", temperature);
            ssd1306_draw_string(&display, buffer, 5, 5);

            sprintf(buffer, "Press: %.1fbar", pressure);
            ssd1306_draw_string(&display, buffer, 5, 15);

            // No modo normal, aciona alerta se ultrapassar os limites críticos.
            // No modo seguro, os valores não chegam perto dos limites.
            if (!safe_mode && (temperature >= critical_temperature || pressure >= critical_pressure))
            {
                ssd1306_draw_string(&display, "! ALERTA !", 25, 32);
                gpio_put(BUZZER_PIN_1, 1);
                gpio_put(BUZZER_PIN_2, 1);
            }
            else
            {
                gpio_put(BUZZER_PIN_1, 0);
                gpio_put(BUZZER_PIN_2, 0);
            }
            ssd1306_send_data(&display);
        }
        else
        {
            // Caldeira desligada: limpa o display
            ssd1306_fill(&display, false);
            ssd1306_send_data(&display);
        }

        // Atualiza a matriz de LEDs (exemplo baseado na pressão)
        int rows = (int)(pressure / 40);
        if (rows > 5)
            rows = 5;
        double pattern[NUM_PIXELS];
        for (int i = 0; i < NUM_PIXELS; i++)
        {
            int row = i / 5;
            pattern[i] = (row < rows) ? 1.0 : 0.0;
        }
        update_matrix(pattern, pio0, 0);

        // Controle dos LEDs via PWM:
        // LED Verde acende para nível baixo (rows == 1)
        // LED Azul acende para nível médio (rows == 2 ou 3)
        // LED Vermelho acende para nível crítico (rows >= 4)
        set_led_pwm(LED_VERDE, (rows == 1) ? 2048 : 0);
        set_led_pwm(LED_AZUL, ((rows == 2) || (rows == 3)) ? 1024 : 0);
        set_led_pwm(LED_VERMELHO, (rows >= 4) ? 2048 : 0);

        printf("Boiler: %s | Safe Mode: %s | Joystick ADC: %d | Temp: %.1f C | Press: %.1f bar\n",
               boiler_on ? "ON" : "OFF",
               safe_mode ? "ON" : "OFF",
               adc_val, temperature, pressure);
        sleep_ms(100);
    }
    return 0;
}
