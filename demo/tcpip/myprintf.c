/******************************************************************************
 *  Include Files
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>  
#include <stdarg.h> 
#include "myprintf.h"
#include "eat_modem.h"
#include "eat_uart.h"



/********************************************************************
 * Macros
 ********************************************************************/


/********************************************************************
 * Types
 ********************************************************************/


/********************************************************************
 * Extern Variables (Extern /Global)
 ********************************************************************/
 
/********************************************************************
 * Local Variables:  STATIC
 ********************************************************************/


/********************************************************************
 * External Functions declaration
 ********************************************************************/


/********************************************************************
 * Local Function declaration
 ********************************************************************/

/********************************************************************
 * Local Function
 ********************************************************************/
my_putchar(const char cha)
{
    char temp[2];
    temp[0] = cha;
    eat_uart_write(EAT_UART_1, (char *)temp, 1);
}
void printch(const char ch)   //输出字符  
{    
    my_putchar(ch);    
}    
void printint(const int dec)     //输出整型数  
{    
    if(dec == 0)    
    {    
        return;    
    }    
    printint(dec / 10);    
    my_putchar((char)(dec % 10 + '0'));    
}    
void printstr(const char *ptr)        //输出字符串  
{    
    while(*ptr)    
    {    
        my_putchar(*ptr);    
        ptr++;    
    }    
}    
void printfloat(const float flt)     //输出浮点数，小数点第5位四舍五入  
{    
    int tmpint = (int)flt;    
    int tmpflt = (int)(100000 * (flt - tmpint));    
    if(tmpflt % 10 >= 5)    
    {    
        tmpflt = tmpflt / 10 + 1;    
    }    
    else    
    {    
        tmpflt = tmpflt / 10;    
    }    
    printint(tmpint);    
    my_putchar('.');    
    printint(tmpflt);    
  
}    
void my_printf(const char *format,...)    
{    
    va_list ap;    
    va_start(ap,format);     //将ap指向第一个实际参数的地址  
    while(*format)    
    {    
        if(*format != '%')    
        {    
            my_putchar(*format);    
            format++;    
        }    
        else    
        {    
            format++;    
            switch(*format)    
            {    
                case 'c':    
                {    
                    char valch = va_arg(ap,int);  //记录当前实践参数所在地址  
                    printch(valch);    
                    format++;    
                    break;    
                }    
                case 'd':    
                {    
                    int valint = va_arg(ap,int);    
                    printint(valint);    
                    format++;    
                    break;    
                }    
                case 's':    
                {    
                    char *valstr = va_arg(ap,char *);    
                    printstr(valstr);    
                    format++;    
                    break;    
                }    
                case 'f':    
                {    
                    float valflt = va_arg(ap,double);    
                    printfloat(valflt);    
                    format++;    
                    break;    
                }    
                default:    
                {    
                    printch(*format);    
                    format++;    
                }    
            }      
        }    
    }  
    va_end(ap);           
    eat_uart_write(EAT_UART_1, "\r\n", 2);
}

