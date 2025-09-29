/**
 ******************************************************************************
 * @author
 * @version
 * @date
 * @brief
 ******************************************************************************
 ******************************************************************************
 */

#include "comm.h"

 typedef struct {
    GPIO_enum     	alias;
    const char *Name;
    uint8_t             pinDir;
    uint8_t             activeMode; // 激活电平是高还是低
} GPIOConfig;

const static GPIOConfig g_gpioConfigComm[] = {
    {PinFANDirctionOut, "FANDirctionOut",   OUTPUT, 0}, // 0: 吸气，常闭； 1: 排气 常开
    {PinButton, "button",   INPUT_PULLUP, 0},
    {PinFANEnable, "FANEnable",   OUTPUT, 0},
};
static const GPIOConfig *GPIO_findGpio(GPIO_enum alias);
void GPIO_bspInit(void)
{
    for (uint8_t i = 0; i < ARRARY_SIZE(g_gpioConfigComm); i++)
    {
        if (g_gpioConfigComm[i].pinDir == INPUT || g_gpioConfigComm[i].pinDir == INPUT_PULLUP || 
            g_gpioConfigComm[i].pinDir == INPUT_PULLDOWN_16) {
            pinMode(g_gpioConfigComm[i].alias, g_gpioConfigComm[i].pinDir);
        }else{
            GPIO_setPinStatus(g_gpioConfigComm[i].alias, DISABLE);
        }
    }
}
static const GPIOConfig *GPIO_findGpio(GPIO_enum alias)
{
    uint8_t num;
    const GPIOConfig *p_gpioCfg;
    if (g_gpioConfigComm != NULL)
    {
        num = ARRARY_SIZE(g_gpioConfigComm);
        for (uint8_t i = 0; i < num; i++)
        {
            p_gpioCfg = &g_gpioConfigComm[i];
            if (p_gpioCfg->alias == alias)
            {
                return p_gpioCfg;
            }
        }
    }
    return NULL;
}
bool GPIO_getPinStatus(GPIO_enum alias)
{
    return digitalRead(alias);
}
bool GPIO_isPinActive(GPIO_enum alias)
{
    const GPIOConfig *p_gpioCfg = GPIO_findGpio(alias);

    if (p_gpioCfg == NULL)
    {
        return false;
    }
    int staus = digitalRead(alias);
    if (staus == p_gpioCfg->activeMode) {
        return true;
    }
    return false;
}
bool GPIO_getPinName(GPIO_enum alias, const char **name)
{
    const GPIOConfig *p_gpioCfg = GPIO_findGpio(alias);

    if (p_gpioCfg == NULL)
    {
        return false;
    }

    *name = p_gpioCfg->Name;
    return true;
}
uint32_t GPIO_getGPIOConfigCount(void)
{
    return ARRARY_SIZE(g_gpioConfigComm);
}
bool GPIO_setPinStatus(GPIO_enum alias, ControlStatus isActive)
{
    const GPIOConfig *p_gpioCfg = GPIO_findGpio(alias);

    if (p_gpioCfg == NULL)
    {
        return false;
    }
    
    if (p_gpioCfg->pinDir != OUTPUT) {
        return false;
    }

    pinMode(alias, OUTPUT);
    
    Serial.printf("init gpio ,set pin %s output, active mode is %d, isActive is %d\r\n", 
        p_gpioCfg->Name, p_gpioCfg->activeMode, isActive);
    if (p_gpioCfg->activeMode)
    {
        if (isActive == ENABLE)
        {
            digitalWrite(alias, HIGH);
        }
        else
        {
            digitalWrite(alias, LOW);
        }
    }
    else
    {
        if (isActive == ENABLE)
        {
            digitalWrite(alias, LOW);
        }
        else
        {
            digitalWrite(alias, HIGH);
        }
    }
    return true;
}


/*********************************************END OF FILE**********************/
