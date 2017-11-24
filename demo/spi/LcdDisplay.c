
#include "lcd.h"
#include "LcdDisplay.h"
//#include "eat_font.h"

#define CHAR_SPACE_PIXEL 1  /* 定义两个字符之间的间隔 */

static u8 LcdDisplayData[X_PIXELS*Y_PIXELS]; /* LCD的显示内容 */
#define M_SetLcdDisplayData(x, y)    LcdDisplayData[(y)*X_PIXELS + (x)] = EAT_TRUE
#define M_ClearLcdDisplayData(x, y)  LcdDisplayData[(y)*X_PIXELS + (x)] = EAT_FALSE
#define M_LcdDisplayData(x, y)  LcdDisplayData[(y)*X_PIXELS + (x)]

void LcdDisplayStr(u8 xStart, u8 yStart, u8 xEnd, u8 yEnd, FontType_Enum font, u8 align, const char *str, eat_bool update)
{
  u8 x, y, ix, iy, cur_x, cur_x2, cur_y;
  u8 width, height;
  FontInfo_Struct fontInfo;
  const u8 *fontData;
  const char *p;
  
  if(font >= FONT_TYPE_NUM)
    return;
  if(str == NULL)
    return;
  if(xEnd < xStart)
    return;
  if(yEnd < yStart)
    return;
  
  /* 获取高度&宽度 */
  p = str;
  width = 0;
  height = 0;
  while(*p != 0)
  {
    /* 取字库信息 */
    fontData = FontFind(*p, font, &fontInfo);
    if(fontData != NULL)
    {
      width += fontInfo.width;
      #if defined(CHAR_SPACE_PIXEL)
      width += CHAR_SPACE_PIXEL;
      #endif
      if(fontInfo.height > height)
        height = fontInfo.height;
    }
    p++;
  }
  
  switch(align&LCD_ALIGN_V)
  {
    case LCD_ALIGN_V_UP:
      y = yStart;
      break;
    case LCD_ALIGN_V_DOWN:
      if((yStart+height) > yEnd)
        y = yStart;
      else
        y = yEnd-height-1;
      break;
    default:
      if((yStart+height) > yEnd)
        y = yStart;
      else
        y = yStart + ((yEnd - yStart - height) / 2);
      break;
  }

  switch(align&LCD_ALIGN_H)
  {
    case LCD_ALIGN_H_LEFT:
      x = xStart;
      break;
    case LCD_ALIGN_H_RIGHT:
      if((xStart+width) > xEnd)
        x = xStart;
      else
        x = xEnd-width-1;
      break;
    default:
      if((xStart+width) > xEnd)
        x = xStart;
      else
        x = xStart + ((xEnd - xStart - width) / 2);
      break;
  }
  
  LcdDisplayClear(xStart, yStart, xEnd, yEnd, EAT_FALSE);

  p = str;
  cur_x = x;
  while(*p != 0)
  {
    /* 取字库信息 */
    fontData = FontFind(*p, font, &fontInfo);
    if(fontData != NULL)
    {
      /* 字库转换为显示数据 */
      cur_y = y;
      for(iy=0; iy<fontInfo.height; iy++)
      {
        cur_x2 = cur_x;
        for(ix=0; ix<fontInfo.width; ix++)
        {
          if((fontData[iy*((fontInfo.width+7)/8) + ix/8] & (1<<(ix%8))))
          {
            M_SetLcdDisplayData(cur_x2, cur_y);
          }
          else
          {
            M_ClearLcdDisplayData(cur_x2, cur_y);
          }
          cur_x2++;
          if(cur_x2 > xEnd)
            break;
        }
        cur_y++;
        if(cur_y > yEnd)
          break;
      }
      cur_x += fontInfo.width;
      #if defined(CHAR_SPACE_PIXEL)
      cur_x += CHAR_SPACE_PIXEL;
      #endif
      if(cur_x > xEnd)
        break;
    }
    p++;
  }
  
  if(update == EAT_TRUE)
  {
    LcdUpdate(xStart, yStart, xEnd, yEnd);
  }
}

void LcdDisplayClear(u8 xStart, u8 yStart, u8 xEnd, u8 yEnd, eat_bool update)
{
  u8 x, y;
  for(y=yStart; y<=yEnd; y++)
  {
    for(x=xStart; x<=xEnd; x++)
    {
      M_ClearLcdDisplayData(x, y);
    }
  }
  if(update == EAT_TRUE)
  {
    LcdUpdate(xStart, yStart, xEnd, yEnd);
  }
}
void LcdDisplaySet(u8 xStart, u8 yStart, u8 xEnd, u8 yEnd, eat_bool update)
{
  u8 x, y;
  for(y=yStart; y<=yEnd; y++)
  {
    for(x=xStart; x<=xEnd; x++)
    {
      M_SetLcdDisplayData(x, y);
    }
  }
  if(update == EAT_TRUE)
  {
    LcdUpdate(xStart, yStart, xEnd, yEnd);
  }
}

void LcdUpdate(u8 xStart, u8 yStart, u8 xEnd, u8 yEnd)
{
  u8 x, y, page, page_end, data;

  page_end = yEnd / Y_PAGE_SIZE;
  for(page=yStart/Y_PAGE_SIZE; page<=page_end; page++)
  {
    #if defined(TP_SIM800W_OLD)
    LcdSetDisplayAddress(xStart, page*Y_PAGE_SIZE);
    #else
    LcdSetDisplayAddress(xStart + 4, page*Y_PAGE_SIZE);
    #endif
    for(x=xStart; x<=xEnd; x++)
    {
      data = 0;
      for(y=0; y<8; y++)
      {
        if(M_LcdDisplayData(x, ((page*Y_PAGE_SIZE)+y)) == EAT_TRUE)
        {
          data |= 1<<y;
        }
      }
      LcdWriteData(data);
    }
  }
}
void LcdUpdateAll(void)
{
  LcdUpdate(0, 0, X_PIXELS-1, Y_PIXELS-1);
}
#if 0
void LcdDisplayStrEn(u8 xStart, u8 yStart, u8 xEnd, u8 yEnd, u8 align, const unsigned short *str, eat_bool update)
{
  u8 x, y, ix, iy, cur_x, cur_x2, cur_y;
  EatCharFontInfo_st fontInfo;
  unsigned char height, width;
  unsigned char c_h, c_w;
  const unsigned short *p;
  unsigned int pos;

  /* 获取高度&宽度 */
  p = str;
  width = 0;
  height = 0;
  while(*p != 0)
  {
    /* 取字库信息 */
    if(EatGetCharFontWidthAndHeight(EAT_FONT_TYPE_ENGLISH, *p, &c_h, &c_w))
    {
      width += c_w;
      #if defined(CHAR_SPACE_PIXEL)
      width += CHAR_SPACE_PIXEL;
      #endif
      if(c_h > height)
        height = c_h;
    }
    p++;
  }
  
  switch(align&LCD_ALIGN_V)
  {
    case LCD_ALIGN_V_UP:
      y = yStart;
      break;
    case LCD_ALIGN_V_DOWN:
      if((yStart+height) > yEnd)
        y = yStart;
      else
        y = yEnd-height-1;
      break;
    default:
      if((yStart+height) > yEnd)
        y = yStart;
      else
        y = yStart + ((yEnd - yStart - height) / 2);
      break;
  }

  switch(align&LCD_ALIGN_H)
  {
    case LCD_ALIGN_H_LEFT:
      x = xStart;
      break;
    case LCD_ALIGN_H_RIGHT:
      if((xStart+width) > xEnd)
        x = xStart;
      else
        x = xEnd-width-1;
      break;
    default:
      if((xStart+width) > xEnd)
        x = xStart;
      else
        x = xStart + ((xEnd - xStart - width) / 2);
      break;
  }
  
  LcdDisplayClear(xStart, yStart, xEnd, yEnd, EAT_FALSE);

  p = str;
  cur_x = x;
  while(*p != 0)
  {
    /* 取字库信息 */
    if(EatGetCharFontInfo(EAT_FONT_TYPE_ENGLISH, *p, &fontInfo))
    {
      /* 字库转换为显示数据 */
      cur_y = y;
      pos = 0;
      for(iy=0; iy<fontInfo.height; iy++)
      {
        cur_x2 = cur_x;
        for(ix=0; ix<fontInfo.width; ix++)
        {
          if(fontInfo.data[pos])
          {
            M_SetLcdDisplayData(cur_x2, cur_y);
          }
          else
          {
            M_ClearLcdDisplayData(cur_x2, cur_y);
          }
          pos++;
          cur_x2++;
          if(cur_x2 > xEnd)
            break;
        }
        cur_y++;
        if(cur_y > yEnd)
          break;
      }
      cur_x += fontInfo.width;
      #if defined(CHAR_SPACE_PIXEL)
      cur_x += CHAR_SPACE_PIXEL;
      #endif
      if(cur_x > xEnd)
        break;
    }
    p++;
  }
  
  if(update == EAT_TRUE)
  {
    LcdUpdate(xStart, yStart, xEnd, yEnd);
  }
}

void LcdDisplayStrCn(u8 xStart, u8 yStart, u8 xEnd, u8 yEnd, u8 align, const unsigned short *str, eat_bool update)
{
  u8 x, y, ix, iy, cur_x, cur_x2, cur_y;
  EatCharFontInfo_st fontInfo;
  unsigned char height, width;
  unsigned char c_h, c_w;
  const unsigned short *p;
  unsigned int pos;

  /* 获取高度&宽度 */
  p = str;
  width = 0;
  height = 0;
  while(*p != 0)
  {
    /* 取字库信息 */
    if(EatGetCharFontWidthAndHeight(EAT_FONT_TYPE_CHINESE, *p, &c_h, &c_w))
    {
      width += c_w;
      #if defined(CHAR_SPACE_PIXEL)
      width += CHAR_SPACE_PIXEL;
      #endif
      if(c_h > height)
        height = c_h;
    }
    p++;
  }
  
  switch(align&LCD_ALIGN_V)
  {
    case LCD_ALIGN_V_UP:
      y = yStart;
      break;
    case LCD_ALIGN_V_DOWN:
      if((yStart+height) > yEnd)
        y = yStart;
      else
        y = yEnd-height-1;
      break;
    default:
      if((yStart+height) > yEnd)
        y = yStart;
      else
        y = yStart + ((yEnd - yStart - height) / 2);
      break;
  }

  switch(align&LCD_ALIGN_H)
  {
    case LCD_ALIGN_H_LEFT:
      x = xStart;
      break;
    case LCD_ALIGN_H_RIGHT:
      if((xStart+width) > xEnd)
        x = xStart;
      else
        x = xEnd-width-1;
      break;
    default:
      if((xStart+width) > xEnd)
        x = xStart;
      else
        x = xStart + ((xEnd - xStart - width) / 2);
      break;
  }
  
  LcdDisplayClear(xStart, yStart, xEnd, yEnd, EAT_FALSE);

  p = str;
  cur_x = x;
  while(*p != 0)
  {
    /* 取字库信息 */
    if(EatGetCharFontInfo(EAT_FONT_TYPE_CHINESE, *p, &fontInfo))
    {
      /* 字库转换为显示数据 */
      cur_y = y;
      pos = 0;
      for(iy=0; iy<fontInfo.height; iy++)
      {
        cur_x2 = cur_x;
        for(ix=0; ix<fontInfo.width; ix++)
        {
          if(fontInfo.data[pos])
          {
            M_SetLcdDisplayData(cur_x2, cur_y);
          }
          else
          {
            M_ClearLcdDisplayData(cur_x2, cur_y);
          }
          pos++;
          cur_x2++;
          if(cur_x2 > xEnd)
            break;
        }
        cur_y++;
        if(cur_y > yEnd)
          break;
      }
      cur_x += fontInfo.width;
      #if defined(CHAR_SPACE_PIXEL)
      cur_x += CHAR_SPACE_PIXEL;
      #endif
      if(cur_x > xEnd)
        break;
    }
    p++;
  }
  
  if(update == EAT_TRUE)
  {
    LcdUpdate(xStart, yStart, xEnd, yEnd);
  }
}

#endif
