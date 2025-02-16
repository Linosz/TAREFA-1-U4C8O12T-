Controle de LED e Display com Joystick no Raspberry Pi Pico W
Este projeto utiliza um joystick analógico para controlar LEDs RGB e um display OLED SSD1306 via comunicação I2C. Ele foi desenvolvido para rodar no Raspberry Pi Pico W.

📌 Funcionalidades
O botão do joystick liga/desliga o LED verde e ativa uma borda no display.

O botão A ativa/desativa os LEDs vermelho e azul (PWM).

O botão B cria vários quadrados no display para desafiar o usuário.

O eixo X do joystick controla o brilho do LED vermelho.

O eixo Y do joystick controla o brilho do LED azul.

O display OLED exibe um quadrado que se move conforme o joystick.

🛠️ Componentes Necessários

Raspberry Pi Pico W

Joystick analógico com botão

LEDs RGB (ou LEDs individuais)

Resistor (caso necessário para LEDs)

Display OLED SSD1306 (I2C)

Jumpers e protoboard

🏗️ Ligações
Componente	Pino Raspberry Pi Pico W
*LED Vermelho	GP13
*LED Azul	GP12
*LED Verde	GP11
*Joystick X	GP27 (ADC1)
*Joystick Y	GP26 (ADC0)
*Botão Joystick	GP22
*Botão A	GP5
*Botão B	GP6
*Display SDA	GP14
*Display SCL	GP15


📦 Bibliotecas Necessárias

Antes de compilar o código, instale as seguintes bibliotecas:

Pico SDK (para desenvolvimento no Raspberry Pi Pico)

hardware/pwm.h (para controle PWM dos LEDs)

hardware/adc.h (para leitura do joystick)

hardware/i2c.h (para comunicação I2C com o display)

ssd1306.h (para controle do display OLED)
