/* main.c - STM32F401RE
   MPU6050 via I2C1 (PB8=SCL, PB9=SDA)
   UART2 on PA2(TX)/PA3(RX) for serial prints (115200)
   LEDs:
     PA0 - Power (always ON)
     PA1 - MPU activity indicator (blinks)
     PA2 - Motion detected
*/

#include "main.h"
#include <stdio.h>
#include <math.h>

I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart2;

#define MPU6050_ADDR        (0x68 << 1)
#define MOTION_THRESHOLD    0.15f
#define CALIBRATION_SAMPLES 100
#define FILTER_SIZE         5

/* Function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);
void Error_Handler(void);

void MPU6050_Init(void);
void MPU6050_Read_Accel_Gyro(int16_t *Acc, int16_t *Gyro);
void MPU6050_Calibrate(void);

/* Retarget printf to UART2 */
int __io_putchar(int ch)
{
    uint8_t c = (uint8_t)ch;
    HAL_UART_Transmit(&huart2, &c, 1, HAL_MAX_DELAY);
    return ch;
}

/* Global offsets */
float acc_offset[3] = {0}, gyro_offset[3] = {0};

/* Simple moving average buffers */
float accX_buf[FILTER_SIZE] = {0}, accY_buf[FILTER_SIZE] = {0}, accZ_buf[FILTER_SIZE] = {0};
int filter_index = 0;

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_USART2_UART_Init();

    printf("Starting MPU6050 Flight Controller\r\n");

    /* Power LED ON always */


    /* Initialize MPU6050 */
    MPU6050_Init();

    /* WHO_AM_I check */
    uint8_t who = 0;
    HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, 0x75, 1, &who, 1, 1000);
    if (who == 0x68)
        printf("MPU6050 detected (WHO_AM_I=0x%02X)\r\n", who);
    else
        printf("MPU6050 NOT detected! WHO=0x%02X\r\n", who);

    /* Calibrate sensor */
    MPU6050_Calibrate();

    int16_t Acc[3], Gyro[3];
    float accX_g, accY_g, accZ_g;

    while (1)
    {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1);  // MPU active blink

        /* Read sensor data */
        MPU6050_Read_Accel_Gyro(Acc, Gyro);

        /* Normalize accelerometer values */
        accX_g = ((float)Acc[0] - acc_offset[0]) / 16384.0f;
        accY_g = ((float)Acc[1] - acc_offset[1]) / 16384.0f;
        accZ_g = ((float)Acc[2] - acc_offset[2]) / 16384.0f;

        /* Apply simple moving average filter */
        accX_buf[filter_index] = accX_g;
        accY_buf[filter_index] = accY_g;
        accZ_buf[filter_index] = accZ_g;
        filter_index = (filter_index + 1) % FILTER_SIZE;

        float sumX = 0, sumY = 0, sumZ = 0;
        for (int i = 0; i < FILTER_SIZE; i++)
        {
            sumX += accX_buf[i];
            sumY += accY_buf[i];
            sumZ += accZ_buf[i];
        }

        accX_g = sumX / FILTER_SIZE;
        accY_g = sumY / FILTER_SIZE;
        accZ_g = sumZ / FILTER_SIZE;

        /* Calculate overall acceleration magnitude */
        float magnitude = sqrtf(accX_g*accX_g + accY_g*accY_g + accZ_g*accZ_g);

        /* Motion detection */
        if (HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, 0x3B, 1, (uint8_t*)Acc, 6, 1000) != HAL_OK)
         {

             HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET); // 🔴 Turn ON error LED
             // skip rest of loop
         }
         else
         {
             HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET); // 🟢 No error
         }


        /* Print normalized data */

       printf("%.2f %.2f %.2f \r\n",accX_g, accY_g, accZ_g);
//        printf("%.2f\r\n", magnitude);
     // printf("ACC[g]: X=%.2f Y=%.2f Z=%.2f | Magnitude=%.2f\r\n",  accX_g, accY_g, accZ_g, magnitude);

        HAL_Delay(300);
    }
}

/* ---------------- MPU6050 Functions ---------------- */

void MPU6050_Init(void)
{
    uint8_t data;

    /* Wake up the sensor */
    data = 0x00;
    HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, 0x6B, 1, &data, 1, 1000);

    /* Set accelerometer ±2g */
    data = 0x00;
    HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, 0x1C, 1, &data, 1, 1000);

    /* Set gyro ±250°/s */
    data = 0x00;
    HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, 0x1B, 1, &data, 1, 1000);

    HAL_Delay(50);
    printf("MPU6050 initialized\r\n");
}

void MPU6050_Read_Accel_Gyro(int16_t *Acc, int16_t *Gyro)
{
    uint8_t buf[14];
    if (HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, 0x3B, 1, buf, 14, 1000) != HAL_OK)
    {
        printf("I2C read error\r\n");
        return;
    }

    Acc[0] = (int16_t)((buf[0] << 8) | buf[1]);
    Acc[1] = (int16_t)((buf[2] << 8) | buf[3]);
    Acc[2] = (int16_t)((buf[4] << 8) | buf[5]);
    Gyro[0] = (int16_t)((buf[8] << 8) | buf[9]);
    Gyro[1] = (int16_t)((buf[10] << 8) | buf[11]);
    Gyro[2] = (int16_t)((buf[12] << 8) | buf[13]);
}

void MPU6050_Calibrate(void)
{
    int16_t Acc[3], Gyro[3];
    float acc_sum[3] = {0}, gyro_sum[3] = {0};

    printf("Calibrating MPU6050...\r\n");
    for (int i = 0; i < CALIBRATION_SAMPLES; i++)
    {
        MPU6050_Read_Accel_Gyro(Acc, Gyro);
        acc_sum[0] += Acc[0];
        acc_sum[1] += Acc[1];
        acc_sum[2] += Acc[2];
        gyro_sum[0] += Gyro[0];
        gyro_sum[1] += Gyro[1];
        gyro_sum[2] += Gyro[2];
        HAL_Delay(10);
    }

    for (int i = 0; i < 3; i++)
    {
        acc_offset[i] = acc_sum[i] / CALIBRATION_SAMPLES;
        gyro_offset[i] = gyro_sum[i] / CALIBRATION_SAMPLES;
    }

    printf("Calibration done.\r\n");
}

/* ---------------- Peripheral Init ---------------- */

static void MX_I2C1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    __HAL_RCC_I2C1_CLK_ENABLE();

    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&hi2c1) != HAL_OK) Error_Handler();
}

static void MX_USART2_UART_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    __HAL_RCC_USART2_CLK_ENABLE();

    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart2) != HAL_OK) Error_Handler();
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2, GPIO_PIN_RESET);
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 16;
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) Error_Handler();
}

void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);
        HAL_Delay(200);
    }
}
