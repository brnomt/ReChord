/* battery.h — battery / charging (fuel gauge via SAR-ADC + charger). */
#ifndef BATTERY_H
#define BATTERY_H

#include "typedef.h"

/* ---- Battery struct ---- */
typedef struct {
    uint16 Batt_Level;       /* 0-100 */
    uint16 Batt_Value;       /* raw ADC value */
    uint16 Batt_LowCnt;      /* low-battery counter */
    uint8  Batt_LowPower;    /* low-power flag */
    uint8  IsBattChange;     /* level changed */
    uint8  IsBatt_Charge;    /* charging flag */
} BATTERY_T;

extern BATTERY_T gBattery;

/* ---- Charge states ---- */
typedef enum {
    CHARGE_NO = 0,
    CHARGE_BATT,
    CHARGE_FULL,
    CHARGE_ENABLE
} eChargeState;

#define CHARGE_CHECKFULL_COUNT 10
#define CHARGE_FULL_DELAY      2000
#define CHARGE_SAMPLE_TIME     100

/* ---- Battery ADC / levels ---- */
#define ADC_CHANEL_BATTERY     0
#define BATT_TOTAL_STEPS       100
#define BATT_FULL_VALUE        3000
#define BATT_EMPTY_VALUE       2200
#define BATT_USB_FULL_VALUE    2800
#define BATT_POWEROFF_VALUE    2300
#define BATT_POWEROFF_CNT      10

/* ---- API (from battery.c) ---- */
API uint32 Battery_GetLevel(void);
API uint16 Battery_GetChargeState(void);
API void   Battery_Init(void);
API uint16 Battery_GetVoltage(void);
API uint8  Battery_IsCharging(void);
API void   Battery_Task(void);

/* ---- battery.c internal API ---- */
API uint32 RealBattValue(uint16 adc);

/* Globals used by battery.c */
extern uint8  ChargeFullFlag;
extern uint16 Batt_Level[];
/* ---- more battery globals ---- */
extern uint32 BatterySystickCounterBack;
extern uint32 BatteryCounter;
extern uint32 BatteryCounter1;
#define BATTERY_VALUE_SETP   100

#endif /* BATTERY_H */
