#if !defined(__LCD_H__)
#define __LCD_H__


#include "eat_interface.h"

/* Display characteristics */
#define X_PIXELS                 128     /** width in pixels  */
#define Y_PIXELS                 64/*96*/ /** height in pixels */
#define Y_PAGE_SIZE              8       /* bits */
#define Y_PAGES                  ((Y_PIXELS + Y_PAGE_SIZE - 1) / Y_PAGE_SIZE) /* last page is only 4bits */

extern void LcdWriteCommand(u8 cmd);
extern void LcdWriteData(u8 data);
extern void LcdSetDisplayAddress (u8 x, u8 y);
extern void lcd_init(void);

#endif




