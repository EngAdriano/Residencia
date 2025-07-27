/**
 * @file mpu6050_sdk.c
 * @author Daniel Quadros (dqsoft.blogspot@gmail.com)
 * @brief Exemplo com o sensor MPU6050
 * @version 1.0
 * @date 2023-07-30
 * 
 * @copyright Copyright (c) 2023, Daniel Quadros
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// Conexões do sensor
#define I2C_ID        i2c0
#define I2C_SCL_PIN   1
#define I2C_SDA_PIN   0

#define BAUD_RATE     400000   // 400kHz

// Classe para accessar o sensor
class MPU6050 {

  public:

    typedef struct {
      float x;
      float y;
      float z;
    } VECT_3D;

    // Endereço I2C padrão
    static const uint8_t I2C_ADDR = 0x68;

    // Identificação do dispositivo
    static const uint8_t ID = 0x68;

    // Construtor
    MPU6050 (i2c_inst_t *i2c, uint8_t addr = MPU6050::I2C_ADDR);

    // Métodos públicos
    void    begin(void);
    uint8_t getId(void);
    void    reset(void);
    void    getAccel(VECT_3D *vect, float scale = 16384.0);
    void    getGyro(VECT_3D *vect, float scale = 65.5);
    float   getTemp();

  private:
    
    // Endereços dos registradores
    static const int SMPLRT_DIV = 0x19;
    static const int CONFIG = 0x1A;
    static const int GYRO_CONFIG = 0x1B;
    static const int ACCEL_CONFIG = 0x1C;
    static const int ACCEL_OUT = 0x3B;
    static const int TEMP_OUT = 0x41;
    static const int GYRO_OUT = 0x43;
    static const int SIG_PATH_RESET = 0x68;
    static const int PWR_MGMT_1 = 0x6B;
    static const int PWR_MGMT_2 = 0x6C;
    static const int WHO_AM_I = 0x75;

    // Instância do I2C
    i2c_inst_t *i2c;

    // Endereço I2C do sensor
    uint8_t addr;

    // Identificação do Sensor
    uint8_t id;

    // Dados brutos
    int16_t raw[14];

    // Rotinas privativas
    void     readRaw(void);
    uint8_t  read8(uint8_t reg);
    void     write8(uint8_t reg, uint8_t val);
};

// Construtor
MPU6050::MPU6050 (i2c_inst_t *i2c, uint8_t addr) {

  this->i2c = i2c;
  this->addr = addr;
}

// Configura o sensor
void MPU6050::begin() {
  reset();
  this->id = read8(WHO_AM_I);
  write8(CONFIG, 0x00);
  sleep_ms(100);
  write8(ACCEL_CONFIG, 0x00);
  write8(GYRO_CONFIG, 0x08);
  write8(SMPLRT_DIV, 0x00);
  write8(PWR_MGMT_1, 0x01);
  write8(PWR_MGMT_2, 0x00);
  sleep_ms(20);
}

// Retorna o ID do sensor
uint8_t MPU6050::getId() {
  return id;
}

// Reinicia o sensor
void MPU6050::reset(void) {
  uint8_t val = read8(PWR_MGMT_1) | 0x80;
  write8(PWR_MGMT_1, val);
  while (val & 0x80) {
    val = read8(PWR_MGMT_1);
    sleep_ms(1);
  }
  val = read8(SIG_PATH_RESET) | 0x07;
  write8(SIG_PATH_RESET, val);
  while (val & 0x07) {
    val = read8(SIG_PATH_RESET);
    sleep_ms(1);
  }
}

// Fornece aceleração em g
void MPU6050::getAccel(VECT_3D *vect, float scale) {
  readRaw();
  vect->x = (float) raw[0] / scale;
  vect->y = (float) raw[1] / scale;
  vect->z = (float) raw[2] / scale;
}

// Lê giroscópio em graus por segundo
void MPU6050::getGyro(VECT_3D *vect, float scale) {
  readRaw();
  vect->x = (float) raw[4] / scale;
  vect->y = (float) raw[5] / scale;
  vect->z = (float) raw[6] / scale;
}

// Lê temperatura em C
float MPU6050::getTemp() {
  readRaw();
  return (float) raw[3]/340.0 + 36.53;
}

// Lê os dados brutos
void MPU6050::readRaw() {
  uint8_t data[14];

  // Seleciona o primeiro registrador
  uint8_t reg = ACCEL_OUT;
  i2c_write_blocking (i2c, addr, &reg, 1, true);

  // Lê os valores
  i2c_read_blocking (i2c, addr, data, 14, false);

  // Converte para int16
  for (int i = 0; i < 7; i++) {
    raw[i] = (data[2*i] << 8) | data[2*i+1];
  }
}

// Lê um valor de 8 bits de um registrador
uint8_t MPU6050::read8 (uint8_t reg)  {  
  uint8_t val[1];  
    
  // Seleciona o registrador
  i2c_write_blocking (i2c, addr, &reg, 1, true);

  // Lê o valor
  i2c_read_blocking (i2c, addr, val, 1, false);
  return val[0];
} 

// Escreve um valor de 8 bits em um registrador
void MPU6050::write8 (uint8_t reg, uint8_t val)  {  
  uint8_t aux[2];

  aux[0] = reg;
  aux[1] = val;
  i2c_write_blocking (i2c, addr, aux, 2, false);
} 


// Programa Principal
int main() {
  // Inicia stdio
  stdio_init_all();
  #ifdef LIB_PICO_STDIO_USB
  while (!stdio_usb_connected()) {
      sleep_ms(100);
  }
  #endif

  // Inicia a interface I2C
  uint baud = i2c_init (I2C_ID, BAUD_RATE);
  printf ("I2C @ %u Hz\n", baud);
  gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
  gpio_pull_up(I2C_SCL_PIN);
  gpio_pull_up(I2C_SDA_PIN);

  // Inicia o sensor
  MPU6050 sensor (I2C_ID);
  sensor.begin();

  // Mostra o ID do sensor
  printf ("ID do Sensor: %02X\n", sensor.getId());

  // Laço Principal
  MPU6050::VECT_3D data;
  while(1) {
    sleep_ms(2000);
    sensor.getAccel(&data);
    printf ("Aceleracao X:%.1f Y:%.1f Z:%.1f\n", data.x, data.y, data.z);
    sensor.getGyro(&data);
    printf ("Giroscopio X:%.1f Y:%.1f Z:%.1f\n", data.x, data.y, data.z);
    printf ("Temperatura: %.1f\n", sensor.getTemp());
    printf("\n");
  }
}
