# 🚀 Simulação de Caldeira Industrial com Raspberry Pi Pico

## 📌 Descrição do Projeto
Este projeto implementa a simulação de uma **caldeira industrial** utilizando um **Raspberry Pi Pico**. O sistema permite o controle da pressão e temperatura por meio de um **joystick**, exibe os dados em um **display OLED SSD1306**, utiliza **LEDs indicadores**, uma **matriz de LEDs 5x5** para representar os níveis de pressão e temperatura, e um **buzzer** para alertas de segurança. 🔥⚙️

## 🎯 Funcionalidades Implementadas
✅ **Controle da Caldeira**:
   - 🔘 **Botão A**: Liga/Desliga a caldeira, desde que a temperatura esteja segura.
   - 🛑 **Botão B**: Alterna entre **modo normal** e **modo seguro**.

✅ **Modos de Operação**:
   - 🔄 **Modo Normal**: A temperatura e a pressão aumentam de forma gradual e podem ser ajustadas pelo joystick.
   - 🛡️ **Modo Seguro**: Mantém a temperatura e a pressão dentro de uma faixa predefinida, evitando riscos.

✅ **Monitoramento e Controle**:
   - 📟 **Display OLED**: Exibe a temperatura, pressão e status da caldeira.
   - 🔲 **Matriz de LEDs**: Representa os níveis de pressão visualmente.
   - 💡 **LEDs Indicadores**:
     - 🟢 **Verde**: Indica pressão baixa.
     - 🔵 **Azul**: Indica pressão moderada.
     - 🔴 **Vermelho**: Indica pressão crítica.
   - 🔊 **Buzzer**: Emite alerta sonoro se os valores atingirem limites críticos.

✅ **Interação por Joystick**:
   - 🕹️ O **eixo X** do joystick controla a abertura da válvula.
   - 🕹️ O **eixo Y** pode ser usado para futuros aprimoramentos no controle.

## 🛠 Componentes Utilizados
🔌 **Microcontrolador**: Raspberry Pi Pico
🎮 **Joystick**: ADC (GPIO 26 - X, GPIO 27 - Y)
📟 **Display OLED SSD1306**: I2C (GPIO 14 - SDA, GPIO 15 - SCL)
💡 **LEDs PWM**:
   - 🟢 Verde: GPIO 11
   - 🔵 Azul: GPIO 12
   - 🔴 Vermelho: GPIO 13
🟦 **Matriz de LEDs 5x5**: GPIO 7 (controlada via PIO)
🔘 **Botões**:
   - 🎛️ **Botão do Joystick**: GPIO 22
   - 🔘 **Botão A**: GPIO 5
   - 🔘 **Botão B**: GPIO 6
🔊 **Buzzer**: GPIO 10 e GPIO 21

## 🚀 Como Compilar e Executar
1️⃣ **🔧 Configurar o ambiente:**
   - Instale o SDK do Raspberry Pi Pico.
   - Configure o **CMake** e a **toolchain** do Pico.
   - Clone o repositório e navegue até a pasta do projeto.

2️⃣ **💻 Compilar e carregar o código:**
   - Compile o código fonte usando **CMake**.
   - Envie o **arquivo UF2** para o Raspberry Pi Pico.


## 🎥 Demonstração
📌 Assista ao vídeo de demonstração do projeto no link:  
[🎬 Google Drive - [[Link do Vídeo Aqui](https://drive.google.com/file/d/1a_sehKl0_nJgeCif6MDyCx3VX8Dtaaoz/view?usp=drive_link)]]


---

