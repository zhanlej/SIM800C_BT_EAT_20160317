#if !defined(__LCD_DISPLAY_H__)
#define __LCD_DISPLAY_H__

#include "eat_interface.h"
#include "lcd.h"
#include "LcdDataFont.h"

#define LCD_ALIGN_V       0x03
#define LCD_ALIGN_V_UP    0x01
#define LCD_ALIGN_V_DOWN  0x02
#define LCD_ALIGN_H       0x30
#define LCD_ALIGN_H_LEFT  0x10
#define LCD_ALIGN_H_RIGHT 0x20

extern void LcdDisplayStr(u8 xStart, u8 yStart, u8 xEnd, u8 yEnd, FontType_Enum font, u8 align, const char *str, eat_bool update);
/************************
   功能: 显示一行ASCII字符
   xStart: 左上角x坐标
   yStart: 左上角y坐标
   xEnd: 右下角x坐标
   yEnd: 右下角y坐标
   font: 使用的字体
   align: 对齐选项
   str: 显示字符串
   update: 是否将显示内容输出到LCD; EAT_TRUE-输出, EAT_FALSE-不输出
 */
extern void LcdDisplayClear(u8 xStart, u8 yStart, u8 xEnd, u8 yEnd, eat_bool update); 
/************************
   功能: 清除一块显示区域
   xStart: 左上角x坐标
   yStart: 左上角x坐标
   xEnd: 右下角x坐标
   yEnd: 右下角x坐标
   update: 是否将显示内容输出到LCD; EAT_TRUE-输出, EAT_FALSE-不输出
 */
extern void LcdDisplaySet(u8 xStart, u8 yStart, u8 xEnd, u8 yEnd, eat_bool update); 
/************************
   功能: 设置一块显示区域
   xStart: 左上角x坐标
   yStart: 左上角x坐标
   xEnd: 右下角x坐标
   yEnd: 右下角x坐标
   update: 是否将显示内容输出到LCD; EAT_TRUE-输出, EAT_FALSE-不输出
 */
extern void LcdUpdate(u8 xStart, u8 yStart, u8 xEnd, u8 yEnd);
/************************
   功能: 将一块显示区域的内容输出到LCD
   xStart: 左上角x坐标
   yStart: 左上角x坐标
   xEnd: 右下角x坐标
   yEnd: 右下角x坐标
 */
extern void LcdUpdateAll(void);
/************************
   功能: 将整屏的内容输出到LCD
 */

extern void LcdDisplayStrEn(u8 xStart, u8 yStart, u8 xEnd, u8 yEnd, u8 align, const unsigned short *str, eat_bool update);
extern void LcdDisplayStrCn(u8 xStart, u8 yStart, u8 xEnd, u8 yEnd, u8 align, const unsigned short *str, eat_bool update);

#endif

