#ifndef __LCD_DATA_FONT_H__
#define __LCD_DATA_FONT_H__

#include "eat_interface.h"

typedef struct 
{
  char charCode;              /* 字符 */
  u8 width;                   /* 字符的宽度 */
  u8 height;                  /* 字符的高度 */
  u16 len;                    /* 字符显示数据的长度 */
  u32 dataOffset;             /* 字符显示数据在字库中的偏移量 */
} FontInfo_Struct;            /* 字库信息 */

typedef struct 
{
  u16 charNum;                 /* 字库包含的字符个数 */
  const FontInfo_Struct *info; /* 字库信息 */
  const u8 *data;              /* 字库的显示数据, 取模选项: 阴码, 逆向(低位在前), 逐行 */
} FontInfoTable_Struct;        

typedef enum 
{
  FONT_TYPE_MAIN,
  FONT_TYPE_BIG,
  FONT_TYPE_NUM
} FontType_Enum; /* 字体 */

extern const u8 * FontFind(char c, FontType_Enum font, FontInfo_Struct *FontInfo);

#endif

