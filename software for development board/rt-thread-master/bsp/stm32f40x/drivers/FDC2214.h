#ifndef __FDC2214_H__
#define __FDC2214_H__


#include "stdio.h"
#include "DataType.h"
#include "sys.h"
#include "drv_i2c.h"

#define SWAP16(data)    (uint32_t)((((uint16_t)(data) & (0xFF<<0 ))<<8)|(((uint32_t)(data) & (0xFF<<8))>>8))
#define FDC2214_Addr                 (0x2A)
#define FDC2214_DATA_CH0             (0x00)
#define FDC2214_DATA_LSB_CH0         (0x01)
#define FDC2214_DATA_CH1             (0x02)
#define FDC2214_DATA_LSB_CH1         (0x03)
#define FDC2214_DATA_CH2             (0x04)
#define FDC2214_DATA_LSB_CH2         (0x05)
#define FDC2214_DATA_CH3             (0x06)
#define FDC2214_DATA_LSB_CH3         (0x07)
#define FDC2214_RCOUNT_CH0           (0x08)
#define FDC2214_RCOUNT_CH1           (0x09)
#define FDC2214_RCOUNT_CH2           (0x0A)
#define FDC2214_RCOUNT_CH3           (0x0B)
#define FDC2214_SETTLECOUNT_CH0      (0x10)
#define FDC2214_SETTLECOUNT_CH1      (0x11)
#define FDC2214_SETTLECOUNT_CH2      (0x12)
#define FDC2214_SETTLECOUNT_CH3      (0x13)
#define FDC2214_CLOCK_DIVIDERS_CH0   (0x14)
#define FDC2214_CLOCK_DIVIDERS_CH1   (0x15)
#define FDC2214_CLOCK_DIVIDERS_CH2   (0x16)
#define FDC2214_CLOCK_DIVIDERS_CH3   (0x17)
#define FDC2214_STATUS               (0x18)
#define FDC2214_ERROR_CONFIG         (0x19)
#define FDC2214_CONFIG               (0x1A)
#define FDC2214_MUX_CONFIG           (0x1B)
#define FDC2214_RESET_DEV            (0x1C)
#define FDC2214_DRIVE_CURRENT_CH0    (0x1E)
#define FDC2214_DRIVE_CURRENT_CH1    (0x1F)
#define FDC2214_DRIVE_CURRENT_CH2    (0x20)
#define FDC2214_DRIVE_CURRENT_CH3    (0x21)
#define FDC2214_MANUFACTURER_ID      (0x7E)
#define FDC2214_DEVICE_ID            (0x7F)

#define FDC2214_ID                   (0x3055)


typedef enum
{
    FDC2214_Channel_0 = 0x00, 
    FDC2214_Channel_1 = 0x01, 
    FDC2214_Channel_2 = 0x02, 
    FDC2214_Channel_3 = 0x03  
}FDC2214_channel_t;


typedef enum
{
    FDC2214_Channel_Sequence_0_1      = 0x00,
    FDC2214_Channel_Sequence_0_1_2    = 0x01,
    FDC2214_Channel_Sequence_0_1_2_3  = 0x02, 
}FDC2214_channel_sequence_t;


typedef enum
{
    FDC2214_Bandwidth_1M   = 0x01, //1MHz
    FDC2214_Bandwidth_3_3M = 0x04, //3.3MHz
    FDC2214_Bandwidth_10M  = 0x05, //10MHz
    FDC2214_Bandwidth_33M  = 0x07  //33MHz
}FDC2214_filter_bandwidth_t;


typedef enum
{
    FDC2214_Drive_Current_0_016 = 0x00, //0.016mA
    FDC2214_Drive_Current_0_018 = 0x01, //0.018mA
    FDC2214_Drive_Current_0_021 = 0x02, //0.021mA
    FDC2214_Drive_Current_0_025 = 0x03, //0.025mA
    FDC2214_Drive_Current_0_028 = 0x04, //0.028mA
    FDC2214_Drive_Current_0_033 = 0x05, //0.033mA
    FDC2214_Drive_Current_0_038 = 0x06, //0.038mA
    FDC2214_Drive_Current_0_044 = 0x07, //0.044mA
    FDC2214_Drive_Current_0_052 = 0x08, //0.052mA
    FDC2214_Drive_Current_0_060 = 0x09, //0.060mA
    FDC2214_Drive_Current_0_069 = 0x0A, //0.069mA
    FDC2214_Drive_Current_0_081 = 0x0B, //0.081mA
    FDC2214_Drive_Current_0_093 = 0x0C, //0.093mA
    FDC2214_Drive_Current_0_108 = 0x0D, //0.108mA
    FDC2214_Drive_Current_0_126 = 0x0E, //0.126mA
    FDC2214_Drive_Current_0_146 = 0x0F, //0.146mA
    FDC2214_Drive_Current_0_169 = 0x10, //0.169mA
    FDC2214_Drive_Current_0_196 = 0x11, //0.196mA
    FDC2214_Drive_Current_0_228 = 0x12, //0.228mA
    FDC2214_Drive_Current_0_264 = 0x13, //0.264mA
    FDC2214_Drive_Current_0_307 = 0x14, //0.307mA
    FDC2214_Drive_Current_0_356 = 0x15, //0.356mA
    FDC2214_Drive_Current_0_413 = 0x16, //0.413mA
    FDC2214_Drive_Current_0_479 = 0x17, //0.479mA
    FDC2214_Drive_Current_0_555 = 0x18, //0.555mA
    FDC2214_Drive_Current_0_644 = 0x19, //0.644mA
    FDC2214_Drive_Current_0_747 = 0x1A, //0.747mA
    FDC2214_Drive_Current_0_867 = 0x1B, //0.867mA
    FDC2214_Drive_Current_1_006 = 0x1C, //1.006mA
    FDC2214_Drive_Current_1_167 = 0x1D, //1.167mA
    FDC2214_Drive_Current_1_354 = 0x1E, //1.354mA
    FDC2214_Drive_Current_1_571 = 0x1F  //1.571mA
}FDC2214_drive_current_t;

void FDC2214_Write16(uint8_t Slve_Addr, uint8_t reg, uint16_t data);
uint16_t FDC2214_Read16(uint8_t Slve_Addr, uint8_t reg);
uint8_t FDC2214_GetChannelData(FDC2214_channel_t channel, uint32_t *data);
void FDC2214_SetRcount(FDC2214_channel_t channel, uint16_t rcount);
void FDC2214_SetSettleCount(FDC2214_channel_t channel, uint16_t count);
void FDC2214_SetChannelClock(FDC2214_channel_t channel, uint8_t frequency_select, uint16_t divider);
void FDC2214_SetINTB(uint8_t mode);
void FDC2214_SetActiveChannel(FDC2214_channel_t channel);
void FDC2214_SetSleepMode(uint8_t mode);
void FDC2214_SetCurrentMode(uint8_t mode);
void FDC2214_SetClockSource(uint8_t src);
void FDC2214_SetHighCurrentMode(uint8_t mode);
void FDC2214_SetMUX_CONFIG(uint8_t autoscan, FDC2214_channel_sequence_t channels, FDC2214_filter_bandwidth_t bandwidth);
void FDC2214_Reset(void);
void FDC2214_SetDriveCurrent(FDC2214_channel_t channel, FDC2214_drive_current_t current);
double FDC2214_CalculateFrequency(FDC2214_channel_t channel, uint32_t datax);
double FDC2214_CalculateCapacitance(double frequency, float inductance, float capacitance);
uint8_t FDC2214_Init(void);

float Cap_Calculate(uint32_t *data);

#endif
