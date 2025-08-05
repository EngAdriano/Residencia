
#include "stdio.h"
#include "stdlib.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "vl53l0x_api.h"
#include "vl53l0x_rp2040.h"


VL53L0X_RangingMeasurementData_t gRangingData;
VL53L0X_Dev_t gVL53L0XDevice;

#define VL53L0X_GPIO_IRQ    16
#define RED_LED_PIN         13
#define GREEN_LED_PIN       11
#define BLUE_LED_PIN        12

volatile bool vl53l0x_irq_data_ready=false;

void gpioirq_cb(uint gpio, uint32_t event_mask) {
    VL53L0X_Error Status;
    uint8_t data_ready=0;
    if (gpio == VL53L0X_GPIO_IRQ) {
        if (event_mask & GPIO_IRQ_EDGE_FALL) {
            gpio_acknowledge_irq(VL53L0X_GPIO_IRQ, GPIO_IRQ_EDGE_RISE);
            vl53l0x_irq_data_ready=true;
        }
    }
}

void core1__interrupt_task() {
    VL53L0X_Error Status;
    gpio_init(VL53L0X_GPIO_IRQ);

    gpio_set_irq_enabled_with_callback(VL53L0X_GPIO_IRQ, GPIO_IRQ_EDGE_FALL, true, gpioirq_cb);
    Status = VL53L0X_SetDeviceMode(&gVL53L0XDevice, VL53L0X_DEVICEMODE_CONTINUOUS_RANGING); 
    if (Status != VL53L0X_ERROR_NONE) {
        printf("VL52L01 Device mode error\n");
        return;
    }
    Status = VL53L0X_StartMeasurement(&gVL53L0XDevice);
    while(1) {
        if (vl53l0x_irq_data_ready) {
            vl53l0x_irq_data_ready=false;
            Status=VL53L0X_GetRangingMeasurementData(&gVL53L0XDevice, &gRangingData);
            if (Status == VL53L0X_ERROR_NONE && gRangingData.RangeStatus == VL53L0X_ERROR_NONE) { 
                printf("Ranging data:%4d mm\n", gRangingData.RangeMilliMeter);
                
            }
            sleep_ms(10); // 
            VL53L0X_ClearInterruptMask(&gVL53L0XDevice, VL53L0X_REG_SYSTEM_INTERRUPT_GPIO_NEW_SAMPLE_READY);
        }
   }

    VL53L0X_StopMeasurement(&gVL53L0XDevice);
    
}

VL53L0X_Error singleRanging(VL53L0X_Dev_t *pDevice, uint16_t *MeasuredData) {
    VL53L0X_Error Status;
    Status = VL53L0X_SingleRanging(pDevice, MeasuredData);
    /* 
    if (Status == VL53L0X_ERROR_NONE) 
        printf("Measured distance: %d\n",*MeasuredData);
    else 
        printf("measure error\n");
    */
    return Status;
}

VL53L0X_Error continuousRanging(VL53L0X_Dev_t *pDevice, uint16_t *ContinuousData, uint16_t *validCount) {
    uint32_t sum=0;

    uint16_t MeasuredData=0;
    VL53L0X_Error Status;
    sum=0;
    Status = VL53L0X_ContinuousRanging(pDevice, ContinuousData, 16, validCount);
    for (int i = 0; i < *validCount; i++) {
        sum += ContinuousData[i];
    }
    if (*validCount > 0) {
        MeasuredData = sum/(*validCount);
        printf("Average continuous measured distance: %4d,\n" 
                "\tmeasuerd count: %d, valid count: %d\n\n",MeasuredData, 16, *validCount);
    }    else {  
        printf("measure error\n");
    }
    return Status;

}

int main(void)
{
    VL53L0X_Error Status = VL53L0X_ERROR_NONE;
    VL53L0X_Dev_t *pDevice = &gVL53L0XDevice;

    stdio_init_all();
    
    gpio_init(RED_LED_PIN);
    gpio_init(GREEN_LED_PIN);
    gpio_init(BLUE_LED_PIN);

    gpio_set_dir_out_masked(1 << RED_LED_PIN|1 << GREEN_LED_PIN|1 << BLUE_LED_PIN);

    
    pDevice->I2cDevAddr      =  0x29; 
    pDevice->comms_type      =  1;  
    pDevice->comms_speed_khz =  400;

    Status = VL53L0X_dev_i2c_default_initialise(pDevice, VL53L0X_DEFAULT_MODE);
 
    // interrupt ranging test
    //multicore_launch_core1(core1__interrupt_task);
    //while (1);

    uint16_t continuousRingingValue[32];
    uint16_t validCount;
    uint16_t ranging_value=32;
    while(1) { 
        
        //continuousRanging(pDevice, continuousRingingValue, &validCount);
       
        Status = singleRanging(pDevice, &ranging_value);
        if (Status == VL53L0X_ERROR_NONE) {
            gpio_put_masked(1 << RED_LED_PIN|1 << GREEN_LED_PIN|1 << BLUE_LED_PIN, 0);
            if (ranging_value < 200) 
                gpio_put(RED_LED_PIN, true);
            else if (ranging_value < 400)
                    gpio_put(BLUE_LED_PIN, true);
                else 
                    gpio_put(GREEN_LED_PIN, true);
        }
        
       
    }

    return 0;
}

