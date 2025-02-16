#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"

#define LED_RED 13
#define LED_BLUE 12
#define LED_GREEN 11
#define JOYSTICK_X 27
#define JOYSTICK_Y 26
#define BUTTON_JOYSTICK 22
#define BUTTON_A 5
#define BUTTON_B 6 // Definição do botão B na GPIO 6
#define I2C_SDA 14
#define I2C_SCL 15
#define DEADZONE 100 // Faixa de tolerância ao redor do valor central

static volatile bool led_state = false;
static volatile bool leds_enabled = true;
static volatile int border_style = 0;
static volatile bool show_squares = false; // Flag para mostrar quadrados

// Adicionando flags para debounce
static volatile bool debounce_joystick = false;
static volatile bool debounce_button_A = false;
static volatile bool debounce_button_B = false; // Adicionando debounce para o botão B

void debounce(uint gpio) {
    sleep_ms(50); 
    if (!gpio_get(gpio)) { // Se ainda estiver pressionado, espera até soltar
        while (!gpio_get(gpio)) {
            tight_loop_contents(); // Evita travamento da CPU
        }
    }
    sleep_ms(50);
}

// Callback único para os botões
void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == BUTTON_JOYSTICK && !debounce_joystick) {
        debounce_joystick = true;
        led_state = !led_state;
        gpio_put(LED_GREEN, led_state);
        border_style = led_state ? 1 : 0; // Ativa a borda quando pressionado
        printf("Botão do Joystick pressionado, estado do LED verde: %d\n", led_state);
    } 
    else if (gpio == BUTTON_A && !debounce_button_A) {
        debounce_button_A = true;
        leds_enabled = !leds_enabled;
        if (!leds_enabled) {
            pwm_set_gpio_level(LED_RED, 0);
            pwm_set_gpio_level(LED_BLUE, 0);
        }
        printf("Botão A pressionado, LEDs PWM %s\n", leds_enabled ? "ativados" : "desativados");
    } 
    else if (gpio == BUTTON_B && !debounce_button_B) {
        debounce_button_B = true;
        show_squares = !show_squares; // Alterna a exibição dos quadrados
        printf("Botão B pressionado, mostrar quadrados: %d\n", show_squares);
    }
}

void pwm_init_gpio(uint gpio) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(gpio);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 4.0);
    pwm_init(slice, &config, true);
    pwm_set_enabled(slice, true);
}

void setup_gpio(uint gpio, bool is_input, bool pull_up) {
    gpio_init(gpio);
    gpio_set_dir(gpio, is_input ? GPIO_IN : GPIO_OUT);
    if (pull_up) {
        gpio_pull_up(gpio);
    }
}

int main() {
    stdio_init_all();
    setup_gpio(LED_GREEN, false, false);

    pwm_init_gpio(LED_RED);
    pwm_init_gpio(LED_BLUE);

    adc_init();
    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);

    setup_gpio(BUTTON_JOYSTICK, true, true);
    setup_gpio(BUTTON_A, true, true);
    setup_gpio(BUTTON_B, true, true); // Configuração do botão B
    
    // Usa um único callback para todos os botões
    gpio_set_irq_enabled_with_callback(BUTTON_JOYSTICK, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    i2c_init(i2c1, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_t disp;
    ssd1306_init(&disp, 128, 64, false, 0x3C, i2c1);
    ssd1306_config(&disp);

    while (1) {
        // Reseta debounce quando o botão for solto
        if (debounce_joystick && gpio_get(BUTTON_JOYSTICK)) {
            debounce(BUTTON_JOYSTICK);
            debounce_joystick = false;
        }
        if (debounce_button_A && gpio_get(BUTTON_A)) {
            debounce(BUTTON_A);
            debounce_button_A = false;
        }
        if (debounce_button_B && gpio_get(BUTTON_B)) {
            debounce(BUTTON_B);
            debounce_button_B = false;
        }

        adc_select_input(1);
        uint16_t x_value = adc_read();
        adc_select_input(0);
        uint16_t y_value = adc_read();

        if (leds_enabled) {
            if (abs((int)x_value - 2048) <= DEADZONE) {
                pwm_set_gpio_level(LED_RED, 0);
            } else {
                pwm_set_gpio_level(LED_RED, ((abs((int)x_value - 2048)) * 65535) / 2048);
            }

            if (abs((int)y_value - 2048) <= DEADZONE) {
                pwm_set_gpio_level(LED_BLUE, 0);
            } else {
                pwm_set_gpio_level(LED_BLUE, ((abs((int)y_value - 2048)) * 65535) / 2048);
            }
        }

        ssd1306_fill(&disp, false);
        uint8_t square_x = (x_value * 120) / 4095;
        uint8_t square_y = ((4095 - y_value) * 56) / 4095;
        ssd1306_rect(&disp, square_y, square_x, 8, 8, true, true);

        // Ativa a borda quando o botão do joystick for pressionado
        if (border_style == 1) {
            ssd1306_rect(&disp, 0, 0, 128, 64, true, false);
        }

        // Desenha quadrados espalhados pelo display quando o botão B for pressionado
        if (show_squares) {
            for (uint8_t i = 10; i < 120; i += 20) {
                for (uint8_t j = 10; j < 60; j += 20) {
                    ssd1306_rect(&disp, j, i, 5, 5, true, true);
                }
            }
        }

        ssd1306_send_data(&disp);
        sleep_ms(50);
    }
}
