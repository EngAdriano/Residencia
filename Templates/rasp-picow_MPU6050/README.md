<p align="center">
  <img src="https://img.icons8.com/fluency/96/gyroscope.png" alt="MPU6050 Logo" width="120"/>
</p>

<h1 align="center">Controle e Monitoramento de Movimento com MPU6050 no RP2040</h1>

<p align="center">
  <b>Projeto em C para Raspberry Pi Pico W (RP2040): leitura de aceleração e rotação com sensor MPU6050, controle dinâmico de servo motor 9G SG90 e alerta visual em LCD 320x240.</b>
</p>

---

## 🎯 Objetivo

- Captar dados de movimento (aceleração e rotação) com o sensor **MPU6050** via I2C no RP2040.
- Exibir leituras de inclinação no monitor serial em tempo real.
- Ajustar a posição de um **Servo Motor 9G SG90** conforme o ângulo de inclinação detectado.
- Exibir alerta visual na tela **LCD 320x240** quando o ângulo ultrapassar um limite configurado.

---

## ⚙️ Funcionamento

- O MPU6050 fornece dados de acelerômetro e giroscópio para cálculo do ângulo de inclinação.
- O RP2040 processa esses dados para controlar o servo motor, ajustando sua posição dinamicamente.
- A tela LCD exibe as leituras e um alerta visual quando o ângulo ultrapassa o limite definido, facilitando o monitoramento.

---

## 🔌 Esquema de Ligações

| Componente       | RP2040 (Pico W)       |
|------------------|-----------------------|
| MPU6050 VCC      | 3.3V                  |
| MPU6050 GND      | GND                   |
| MPU6050 SDA      | GP4 (I2C SDA)         |
| MPU6050 SCL      | GP5 (I2C SCL)         |
| Servo Motor Sinal| GP2 (PWM)             |
| Servo Motor VCC  | 5V (ou fonte externa) |
| Servo Motor GND  | GND                   |
| LCD VCC          | 3.3V ou 5V (conforme LCD) |
| LCD GND          | GND                   |
| LCD SDA/SCL      | GP0/GP1 (ou conforme conexão SPI/I2C do LCD) |

---

## 💻 Estrutura do Projeto

- `main.c`: Inicializa sensores, coleta dados, controla servo e atualiza LCD.
- `mpu6050.c/h`: Driver para comunicação e leitura do MPU6050 via I2C.
- `servo.c/h`: Controle PWM do servo motor.
- `lcd.c/h`: Interface com LCD 320x240 para exibição de dados e alertas.

---

## 🖥️ Como Compilar e Executar

1. Clone o repositório:
git clone https://github.com/GabrFelps/rasp-picow_MPU6050.git

```
2. Configure o ambiente do SDK do Raspberry Pi Pico.
3. Compile o projeto:
cd rasp-picow_MPU6050
mkdir build && cd build
cmake ..
make
```

4. Copie o arquivo `.uf2` gerado para o Raspberry Pi Pico W.

5. Conecte o hardware conforme o esquema.

6. Abra o monitor serial para visualizar as leituras de inclinação.

7. Observe o movimento do servo conforme o ângulo detectado e o alerta na tela LCD quando ultrapassar o limite.
