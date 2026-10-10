/*
********************************************************************************
*                   Copyright (c) 2008, Rock-Chips
*                         All rights reserved.
*
* File Name£º   GPIO.c
* 
* Description:  C program template
*
* History:      <author>          <time>        <version>       
*             anzhiguo      2008-11-20         1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_GPIO_

#include "DriverInclude.h"

const uint32 GPIO_GROUP[3] = 
{
    GPIO0_BASE,
    GPIO1_BASE,
    GPIO2_BASE,
};

#define GPIOPORT(n)          ((GPIO_t*)(GPIO_GROUP[n]))

 
/*
--------------------------------------------------------------------------------
  Function name : GPIO_SetPinDirection(eGPIOPinNum_t GPIOPinNum, eGPIOPinDirection_t direction)
  Author        : anzhiguo
  Description   : 
                 
  Input         : GPIOPinNum --
                : direction --
  Return        : 

  History:     <author>         <time>         <version>       
             anzhiguo     2009-1-14         Ver1.0
  desc:        ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
void Gpio_SetPinDirection(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum, eGPIOPinDirection_t direction)
{
    UINT8  pin;
    GPIO_t* pgpio; 

    pgpio = GPIOPORT(gpioPort);    
    
    pin   = GPIOPinNum % 32;

    if (direction)
    {
        pgpio->PortDir |= (1 << pin);
    }
    else
    {   
        pgpio->PortDir &= ~(1 << pin);
    }   
}

void Gpio_SetPortDirec(eGPIO_CHANNEL gpioChn,uint32 dirData)
{
    UINT8  pin;
    GPIO_t* pgpio; 

    pgpio = GPIOPORT(gpioChn);    
     
    pgpio->PortDir = dirData;
}

/*
--------------------------------------------------------------------------------
  Function name : GPIO_GetPinDirection(eGPIOPinNum_t GPIOPinNum)
  Author        : anzhiguo
  Description   : 
                 
  Input         : GPIOPinNum -- GPIO pin number 
                
  Return        : not 0 -- out direction
                  0   -- in direction
  History:     <author>         <time>         <version>       
             anzhiguo     2009-1-14         Ver1.0
  desc:        ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
UINT8 Gpio_GetPinDirection(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum)
{
    UINT8  pin;
    GPIO_t* pgpio;

    pgpio = GPIOPORT(gpioPort);

    pin   = GPIOPinNum % 32;
    return (((pgpio->PortDir) & (0x1 << pin)) >> pin);    
            
}

UINT8 Gpio_GetPortDirec(eGPIO_CHANNEL gpioChn,eGPIO_PORT gpioPort)
{
    UINT8  pin;
    GPIO_t* pgpio;
    uint32 portDir;
    uint32 pinDir;
    int i;

    pgpio = GPIOPORT(gpioChn);
    portDir = pgpio->PortDir;

//    printf("    0 -- IN , 1 -- OUT   \n");
    if( gpioPort == GPIO_PORTA)
    {
        for(i = 0; i < 8 ;i++)
        {
            pinDir = ((portDir & (0x1 << i)) >> i);
            //printf("gpio channel %d:PORT A pin %d direction is [%s] \n",gpioChn,i,(pinDir==0) ? "IN":"OUT");
        } 
    }
    else if( gpioPort == GPIO_PORTB)
    {
        for(i = 8; i < 16 ;i++)
        {
            pinDir = ((portDir & (0x1 << i)) >> i);
            //printf("gpio channel %d:PORT B pin %d direction is [%s] \n",gpioChn,i,(pinDir==0) ? "IN":"OUT");
        } 
    }
    else if( gpioPort == GPIO_PORTC)
    {
        for(i = 16; i < 24 ;i++)
        {
            pinDir = ((portDir & (0x1 << i)) >> i);
            //printf("gpio channel %d:PORT C pin %d direction is [%s]\n",gpioChn,i,(pinDir==0) ? "IN":"OUT");
        } 
    } 
    else if( gpioPort == GPIO_PORTD)
    {
        for(i = 24; i < 32 ;i++)
        {
            pinDir = ((portDir & (0x1 << i)) >> i);
            //printf("gpio channel %d:PORT D pin %d direction is %s \n",gpioChn,i,(pinDir==0) ? "IN":"OUT");
        } 
    }        

    return 0;
}

/*
--------------------------------------------------------------------------------
  Function name : GPIO_SetPinLevel(eGPIOPinNum_t GPIOPinNum, eGPIOPinLevel_t level)
  Author        : anzhiguo
  Description   : 
                  
  Input         : GPIOPinNum -- GPIO pin number
                  level -- 1 or 0(high or low)
  Return        : 

  History:     <author>         <time>         <version>       
             anzhiguo     2009-1-14         Ver1.0
  desc:      GPIOPinNum define the enum by need in future.
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
void Gpio_SetPinLevel(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum, eGPIOPinLevel_t level)
{
    UINT8  pin;
    UINT32 GPIOPortData;
    GPIO_t* pgpio; 

    pgpio = GPIOPORT(gpioPort);     
    
    pin   = GPIOPinNum % 32;
        
    if (level)
    {
        pgpio->PortData |= (1 << pin);
    }
    else
    {
        pgpio->PortData &= ~(1 << pin);
    }
}

void Gpio_SetChnLevel(eGPIO_CHANNEL gpioChn,eGPIOPinLevel_t level)
{
    GPIO_t* pgpio; 

    pgpio = GPIOPORT(gpioChn); 
    
    if (level)
    { 
        pgpio->PortData = 0xffffffff;          
    }
    else
    {
        pgpio->PortData = 0x00000000; 
    }
    
}

void Gpio_SetPortLevel(eGPIO_CHANNEL gpioChn,eGPIO_PORT gpioPort,eGPIOPinLevel_t level)
{
    GPIO_t* pgpio; 

    pgpio = GPIOPORT(gpioChn); 
    
    if (level)
    {
        if( gpioPort == GPIO_PORTA)
            pgpio->PortData = 0xff;
        else if( gpioPort == GPIO_PORTB)
            pgpio->PortData |= 0xff << 8;
        else if( gpioPort == GPIO_PORTC)
            pgpio->PortData |= 0xff << 16;
        else if( gpioPort == GPIO_PORTD)
            pgpio->PortData |= 0xff << 24;          
    }
    else
    {
        if( gpioPort == GPIO_PORTA)
            pgpio->PortData &= ~(0xff);
        else if( gpioPort == GPIO_PORTB)
            pgpio->PortData &= ~(0xff << 8);
        else if( gpioPort == GPIO_PORTC)
            pgpio->PortData &= ~(0xff << 16);
        else if( gpioPort == GPIO_PORTD)
            pgpio->PortData &= ~(0xff << 24);   
    }
}

/*
--------------------------------------------------------------------------------
  Function name : GPIO_GetPinLevel(eGPIOPinNum_t GPIOPinNum)
  Author        : anzhiguo
  Description   : 
                  
  Input         : GPIOPinNum -- GPIO pin number
                  
  Return        : no 0 is high, 0 is low

  History:     <author>         <time>         <version>       
             anzhiguo     2009-1-14         Ver1.0
  desc:        ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
UINT8 Gpio_GetPinLevel(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum)
{
    UINT8  pin;
    GPIO_t* pgpio; 

    pgpio = GPIOPORT(gpioPort);     

    pin   = GPIOPinNum % 32;

    return (((pgpio->ExtPort) & (0x1 << pin)) >> pin);
}


UINT8 Gpio_GetPortLevel(eGPIO_CHANNEL gpioChn,eGPIO_PORT gpioPort)
{
    uint32 i;
    UINT8  pin;
    uint32 portLev;
    uint32 pinLev;
    GPIO_t* pgpio; 

    pgpio = GPIOPORT(gpioChn); 
    portLev = pgpio->ExtPort;

    //printf(" \n  gpio channel %d = 0x%08x \n", gpioChn, portLev);

    return 0;
}

/*
--------------------------------------------------------------------------------
  Function name : GPIO_SetIntMode(eGPIOPinNum_t GPIOPinNum, GPIOIntrType_c type)
  Author        : anzhiguo
  Description   : set GPIO pin interrupt type
                 
  Input         : GPIOPinNum -- GPIO pin
                : type -- 
  Return        : null
  
  note          :recommend use edge type.             
  History:     <author>         <time>         <version>       
             anzhiguo     2009-1-14         Ver1.0
  desc:        ORG  
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
void Gpio_SetIntMode(eGPIO_CHANNEL gpioChn,eGPIOPinNum_t GPIOPinNum, GPIOIntrType_c type)
{
    GPIO_t* pgpio;

    pgpio = GPIOPORT(gpioChn);
    
    switch (type)
    {
        case IntrTypeRisingEdge://rise edge
            pgpio->PortIntType     |= (1 << GPIOPinNum);//edge trigger
            pgpio->PortIntPolarity |= (1 << GPIOPinNum);//level trigger
            break;
            
        case IntrTypeFallingEdge://down edge
            pgpio->PortIntType     |=  (1 << GPIOPinNum);//edge trigger
            pgpio->PortIntPolarity &= ~(1 << GPIOPinNum);//low level valid
            break;
            
        case IntrTypeHighLevel://high voltage
            pgpio->PortIntType     &= ~(1 << GPIOPinNum);//level trigger
            pgpio->PortIntPolarity |=  (1 << GPIOPinNum);//high leve valid
            break;
            
        case IntrTypeLowLevel://low level
            pgpio->PortIntType     &= ~(1 << GPIOPinNum);//level trigger
            pgpio->PortIntPolarity &= ~(1 << GPIOPinNum);//low level valid
            break;
            
        default:
            
            break;
    }
}

void Gpio_SetPortIntMode(eGPIO_CHANNEL gpioChn,eGPIO_PORT gpioPort, GPIOIntrType_c type)
{
    GPIO_t* pgpio;

    pgpio = GPIOPORT(gpioChn);

    pgpio->PortIntType      = 0;
    pgpio->PortIntPolarity  = 0;
    
    switch (type)
    {
        case IntrTypeRisingEdge://rise edge
        {
            if(gpioPort == GPIO_PORTA)
            {
                pgpio->PortIntType     |= 0x000000ff;//edge trigger
                pgpio->PortIntPolarity |= 0x000000ff;//level trigger
            }
            else if(gpioPort == GPIO_PORTB)
            {
                pgpio->PortIntType     |= 0x0000ff00;//edge trigger
                pgpio->PortIntPolarity |= 0x0000ff00;//level trigger                
            }
            else if(gpioPort == GPIO_PORTC)
            {
                pgpio->PortIntType     |= 0x00ff0000;//edge trigger
                pgpio->PortIntPolarity |= 0x00ff0000;//level trigger                
            }
            else if(gpioPort == GPIO_PORTD)
            {
                pgpio->PortIntType     |= 0xff000000;//edge trigger
                pgpio->PortIntPolarity |= 0xff000000;//level trigger                
            }
        }
            break;
            
        case IntrTypeFallingEdge://down edge
        {
            if(gpioPort == GPIO_PORTA)
            {
                pgpio->PortIntType     |= 0x000000ff;//edge trigger
                pgpio->PortIntPolarity &= 0xffffff00;//level trigger
            }
            else if(gpioPort == GPIO_PORTB)
            {
                pgpio->PortIntType     |= 0x0000ff00;//edge trigger
                pgpio->PortIntPolarity &= 0xffff00ff;//level trigger                
            }
            else if(gpioPort == GPIO_PORTC)
            {
                pgpio->PortIntType     |= 0x00ff0000;//edge trigger
                pgpio->PortIntPolarity &= 0xff00ffff;//level trigger                
            }
            else if(gpioPort == GPIO_PORTD)
            {
                pgpio->PortIntType     |= 0xff000000;//edge trigger
                pgpio->PortIntPolarity &= 0x00ffffff;//level trigger                
            }
        } 
            break;
            
        case IntrTypeHighLevel://high voltage
        {
            if(gpioPort == GPIO_PORTA)
            {
                pgpio->PortIntType     &= 0xffffff00;//edge trigger
                pgpio->PortIntPolarity |= 0x000000ff;//level trigger
            }
            else if(gpioPort == GPIO_PORTB)
            {
                pgpio->PortIntType     &= 0xffff00ff;//edge trigger
                pgpio->PortIntPolarity |= 0x0000ff00;//level trigger                
            }
            else if(gpioPort == GPIO_PORTC)
            {
                pgpio->PortIntType     &= 0xff00ffff;//edge trigger
                pgpio->PortIntPolarity |= 0x00ff0000;//level trigger                
            }
            else if(gpioPort == GPIO_PORTD)
            {
                pgpio->PortIntType     &= 0x00ffffff;//edge trigger
                pgpio->PortIntPolarity |= 0xff000000;//level trigger                
            }
        }  
            break;
            
        case IntrTypeLowLevel://low level
        {
            if(gpioPort == GPIO_PORTA)
            {
                pgpio->PortIntType     &= 0xffffff00;//edge trigger
                pgpio->PortIntPolarity &= 0xffffff00 ;//level trigger
            }
            else if(gpioPort == GPIO_PORTB)
            {
                pgpio->PortIntType     &= 0xffff00ff;//edge trigger
                pgpio->PortIntPolarity &= 0xffff00ff;//level trigger                
            }
            else if(gpioPort == GPIO_PORTC)
            {
                pgpio->PortIntType     &= 0xff00ffff;//edge trigger
                pgpio->PortIntPolarity &= 0xff00ffff;//level trigger                
            }
            else if(gpioPort == GPIO_PORTD)
            {
                pgpio->PortIntType     &= 0x00ffffff;//edge trigger
                pgpio->PortIntPolarity &= 0x00ffffff;//level trigger                
            }
        }
            break;
            
        default:
            break;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : GPIO_GetIntMode(eGPIOPinNum_t GPIOPinNum)
  Author        : anzhiguo
  Description   : get interrupt type:edge or level
                 
  Input         : GPIOPinNum -- GPIO pin
                : 
  Return        : 0:level
             
  History:     <author>         <time>         <version>       
             anzhiguo     2009-1-14         Ver1.0
  desc:        ORG 
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
UINT32 Gpio_GetIntMode(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum)
{
    GPIO_t* pgpio; 

    pgpio = GPIOPORT(gpioPort);    
    return ((pgpio->PortIntType) & (0x1 << GPIOPinNum));
}

/*
--------------------------------------------------------------------------------
  Function name : GPIO_ClearInt_Level(eGPIOPinNum_t GPIOPinNum)
  Author        : anzhiguo
  Description   : mask interrupt.
                 
  Input         : GPIOPinNum -- GPIO pin
                : 
  Return        : null
             
  History:     <author>         <time>         <version>       
             anzhiguo     2009-1-14         Ver1.0
  desc:        ORG 
  note          : after clear interrupt trigger type,must clear interrupt mask bit,if not,it will affect
                  next interrupt.
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
void Gpio_ClearInt_Level(eGPIO_CHANNEL gpioChn,eGPIOPinNum_t GPIOPinNum)
{
    GPIO_t* pgpio; 

    pgpio = GPIOPORT(gpioChn);    
    pgpio->PortIntMask |= (0x1 << GPIOPinNum); 
}

void Gpio_ClearPortInt_Level(eGPIO_CHANNEL gpioChn,eGPIO_PORT gpioPort)
{
    GPIO_t* pgpio; 

    pgpio = GPIOPORT(gpioChn);
    
    if( gpioPort == GPIO_PORTA)
    {
        pgpio->PortIntMask |= 0xff;
    }
    else if( gpioPort == GPIO_PORTB)
    {
        pgpio->PortIntMask |= 0xff00;
    }
    else if( gpioPort == GPIO_PORTC)
    {
        pgpio->PortIntMask |= 0xff0000;
    }
    else if( gpioPort == GPIO_PORTD)
    {
        pgpio->PortIntMask |= 0xff000000;
    }    
}


/*
--------------------------------------------------------------------------------
  Function name : GPIO_EnableInt(eGPIOPinNum_t GPIOPinNum)
  Author        : anzhiguo
  Description   :
                 
  Input         : GPIOPinNum -- GPIO pin
                
  Return        : 
                  
  History:     <author>         <time>         <version>       
             anzhiguo     2009-1-14         Ver1.0
  desc:        ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
void Gpio_EnableInt(eGPIO_CHANNEL gpioChn,eGPIOPinNum_t GPIOPinNum)
{	
    GPIO_t* pgpio; 

    pgpio = GPIOPORT(gpioChn);    
    pgpio->PortIntEnable |= (1 << GPIOPinNum);
}

void Gpio_EnablePortInt(eGPIO_CHANNEL gpioChn,eGPIO_PORT gpioPort)
{	
    GPIO_t* pgpio; 

    pgpio = GPIOPORT(gpioChn);

    if( gpioPort == GPIO_PORTA)
    {
        pgpio->PortIntEnable |= 0xff;
    }
    else if( gpioPort == GPIO_PORTB)
    {
        pgpio->PortIntEnable |= 0xff00;
    }
    else if( gpioPort == GPIO_PORTC)
    {
        pgpio->PortIntEnable |= 0xff0000;
    }
    else if( gpioPort == GPIO_PORTD)
    {
        pgpio->PortIntEnable |= 0x0f000000;
    }
}
/*
--------------------------------------------------------------------------------
  Function name : GPIO_DisableInt(eGPIOPinNum_t GPIOPinNum)
  Author        : anzhiguo
  Description   : 
                 
  Input         : GPIOPinNum -- GPIO pin
                
  Return        : null
                  
  History:     <author>         <time>         <version>       
             anzhiguo     2009-1-14         Ver1.0
  desc:        ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
void Gpio_DisableInt(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum)
{    
    GPIO_t* pgpio; 

    pgpio = GPIOPORT(gpioPort);      
    pgpio->PortIntEnable &= ~(1 << GPIOPinNum);  
}

_ATTR_DRIVERLIB_CODE_
void Gpio_MaskInt(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum)
{    
    GPIO_t* pgpio; 

    pgpio = GPIOPORT(gpioPort);      
    pgpio->PortIntMask |= (1 << GPIOPinNum);  
}

void Gpio_MaskPortInt(eGPIO_CHANNEL gpioPort,uint32 data)
{    
    GPIO_t* pgpio; 

    pgpio = GPIOPORT(gpioPort);      
    pgpio->PortIntMask = data;  
}

_ATTR_DRIVERLIB_CODE_
void Gpio_UnMaskInt(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum)
{    
    GPIO_t* pgpio; 

    pgpio = GPIOPORT(gpioPort);      
    pgpio->PortIntMask &= ~(1 << GPIOPinNum);  
}

void Gpio_UnMaskPortInt(eGPIO_CHANNEL gpioPort,uint32 data)
{    
    GPIO_t* pgpio; 

    pgpio = GPIOPORT(gpioPort);      
    pgpio->PortIntMask = data;  
 }


_ATTR_DRIVERLIB_CODE_
void Gpio_ClearEdgeInt(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum)
{    
    GPIO_t* pgpio; 

    pgpio = GPIOPORT(gpioPort);      
    pgpio->PortIntClear |= (1 << GPIOPinNum); 
}

_ATTR_DRIVERLIB_CODE_
/*only when io port configured as input*/
UINT8 Gpio_GetExtPortLevel(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum)
{
    UINT8  pin;
    GPIO_t* pgpio; 

    pgpio = GPIOPORT(gpioPort);     

    pin   = GPIOPinNum % 32;

    return (((pgpio->ExtPort) & (0x1 << pin)) >> pin);
}


_ATTR_DRIVERLIB_CODE_
void Gpio_SetLevelSens_Sync(eGPIO_CHANNEL gpioPort,bool bSync )
{ 
    GPIO_t* pgpio; 

    pgpio = GPIOPORT(gpioPort);     

    if( bSync)
        pgpio->LevelSensSync = 1;//all level-sensitive interrupts being synchronized to pclk_intr
    else
        pgpio->LevelSensSync = 0;
}

_ATTR_DRIVERLIB_CODE_
void Gpio_EnableDebounce(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum)
{	
    GPIO_t* pgpio; 

    pgpio = GPIOPORT(gpioPort);    
    pgpio->DebounceEnable |= (1 << GPIOPinNum);
}

/*
*/


pFunc GPIOIsrCallBack[GPIO_CH_MAX][GPIOPortD_Pin4]=
{
    /*channel 0*/
    //GPIO_A
    0,0,0,0,0,0,0,0,
    //GPIO_B
    0,0,0,0,0,0,0,0,
    //GPIO_C
    0,0,0,0,0,0,0,0,
    //GPIO_D
    0,0,0,0,

    /*channel 1*/
    //GPIO_A
    0,0,0,0,0,0,0,0,
    //GPIO_B
    0,0,0,0,0,0,0,0,
    //GPIO_C
    0,0,0,0,0,0,0,0,
    //GPIO_D
    0,0,0,0,

    /*channel 2*/
    //GPIO_A
    0,0,0,0,0,0,0,0,
    //GPIO_B
    0,0,0,0,0,0,0,0,
    //GPIO_C
    0,0,0,0,0,0,0,0,
    //GPIO_D
    0,0,0,0
    
};

/*
*/


int32 GpioIsrRegister(eGPIO_CHANNEL gpioChn,uint32 pin, pFunc CallBack)
{
    if (pin >= GPIOPortD_Pin4)
    {
        //printf("GPIO beyond the range: pin = %d !\n" , pin);
        return ERROR;
    }
    
    GPIOIsrCallBack[gpioChn][pin] = (pFunc)CallBack;

    return OK;
}

/*
*/


int32 GPIOIsrUnRegister(eGPIO_CHANNEL gpioChn,uint32 pin)
{
    if (pin >= GPIOPortD_Pin4)
    {
        //printf("GPIO beyond the range: pin = %d!\n", pin);
        return ERROR;
    }
    
    GPIOIsrCallBack[gpioChn][pin] = NULL;

    return OK;
}

/*
*/


void GpioInt(eGPIO_CHANNEL gpioPort)
{
    int ch = 0;
    uint32 IsEnable;
    uint32 GPIOPinBitTmp;
    pFunc GPIOCallBack;

    uint32 pin;
    uint32 rawStatus;
    pFunc  CallBack;
    GPIO_t* pgpio;     

    pgpio = GPIOPORT(gpioPort);
    
    do 
    {
        rawStatus = (uint32)(pgpio->PortIntStatus) & 0x0fffffff;
        pgpio->PortIntClear = rawStatus;
        
        pin = 0;
        do
        {
             if (rawStatus & (0x01ul << pin))
             {
                rawStatus &= ~(0x01ul << pin);
                CallBack = GPIOIsrCallBack[gpioPort][pin];
                if (CallBack)
                {
                    CallBack();
                }
             }
             pin++;
        
        }while(rawStatus & 0x0fffffff);
        
    }while((uint32)(pgpio->PortIntStatus) & 0x0fffffff);

}

void GpioInt2(void)
{
    GpioInt(GPIO_CH2);
}

void GpioInt1(void)
{
    GpioInt(GPIO_CH1);
}

void GpioInt0(void)
{
    GpioInt(GPIO_CH0);
}

/*
********************************************************************************
*
*                         End of Gpio.c
*
********************************************************************************
*/

