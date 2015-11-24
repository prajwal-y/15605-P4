/** @file console_util.c
 *  @brief This file contains various utility functions that 
 *         are used by the console driver.
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
*/

#include <string/string.h>
#include <asm.h>
#include <video_defines.h>
#include <drivers/console/console_util.h>
#include <common/malloc_wrappers.h>
#include <common/errors.h>

#define CONSOLE_MEM_END CONSOLE_MEM_BASE + (CONSOLE_HEIGHT * CONSOLE_WIDTH * 2)

int cursor_row;
int cursor_col;
char console_color;
char empty_char[2];
static char buf[CONSOLE_WIDTH * CONSOLE_HEIGHT * 2];

/** @brief a function to set the logical cursor
 *
 *  This function keeps track of the current logical cursor
 *  position and should be called by any function that manipulates
 *  the cursor
 *
 *  @param row the row of the cursor
 *         col the column of the cursor
 *  @return none
 */
void set_logical_cursor(int row, int col) {
    cursor_row = (row < 0) ? 0 : row; 
    cursor_col = (col < 0) ? 0 : col; 
}

/** @brief function to clear a position on the screen
 *
 *  This function clears the position(s) indicated by addr and num
 *  by setting the character to be a space and setting the color to be 
 *  the current terminal color
 *
 *  @param addr the address in video memory from which to clear the screen
 *         num the number of characters on the screen to be cleared
 *  @return void
 */
void empty_fill(void *addr, int num) {
    int i;
    int size_char = sizeof(empty_char);
    for (i = 0; i < num; i++) {
        memcpy((char *)addr + (i * size_char), empty_char, size_char);
    }
    return;
}

/** @brief function to set the hardware cursor
 *
 *  This function communicates to the VGA device to set the position of 
 *  the hardware cursor. this can be used to hide the cursor by setting
 *  the position beyond the bounds of the display device
 *
 *  @param row the row where the cursor has to be present
 *         col the column where the cursor has to be present
 *  @return void
 */
void set_hardware_cursor(int row, int col) {
    unsigned short position = (row * 80) + col;

    outb(CRTC_IDX_REG, CRTC_CURSOR_LSB_IDX);
    outb(CRTC_DATA_REG,(unsigned char) (position & 0xFF));

    outb(CRTC_IDX_REG, CRTC_CURSOR_MSB_IDX);
    outb(CRTC_DATA_REG,(unsigned char) ((position >> 8) & 0xFF));
    
    return;
} 

/** @brief Checks if a color is a valid VGA color
 *
 *  The validity is determined based on the colors defined in
 *  video_defines.h. The foreground color and the background color
 *  are independently checked against the ones defined in video_defines.h
 *
 *  @param color an integer representing the console color with background
 *                 and foreground
 *  @return integer 0 for a valid color and -ve integer for an invalid color
 */
int is_valid_color(int color) {
    int bg_color = color & 0xF;
    int fg_color = (color >> 4) & 0xF;

    if (fg_color < 0 || fg_color > 0xF
        || bg_color < 0 || bg_color > 0x8) {
        return ERR_INVAL;
    }
    return 0;
}

/** @brief a function to print a character at given row and column
 *
 *  If the character is a \n or a \r we just return since they just manipulate
 *  the position of the cursor.
 *
 *  @param ch The character that is to be printed
 *         row The row at which the character is to be printed
 *         col The column at which the character is to be printed
 *         color the color with which the character will be printed
 *  @return void
 */
void print_char(char ch, int row, int col, int color) {
    if (ch == '\n' || ch == '\r') {
        return;
    }
    *(char *)(CONSOLE_MEM_BASE + 2*(row * CONSOLE_WIDTH + col)) = ch;
    *(char *)(CONSOLE_MEM_BASE + 2*(row * CONSOLE_WIDTH + col) + 1) = color;
}

/** @brief scroll the screen by the specified number of rows
 *
 *  The function achieves the scroll by doing a memcpy of all the 
 *  rows to one row above it (basically a shift). We do not keep
 *  track of rows that are pushed off the screen.
 *
 *  @param num_rows number of rows to scroll by
 *  @return void
 */
void scroll_screen(int num_rows) {
    if (num_rows <= 0) {
        return;
    }

    if (num_rows >= CONSOLE_HEIGHT) {
        return;
    }

    int offset = num_rows * CONSOLE_WIDTH * 2;
    memcpy(buf, (void *)(CONSOLE_MEM_BASE + offset), 
           (CONSOLE_HEIGHT * CONSOLE_WIDTH * 2) - offset);
    memcpy((void *)CONSOLE_MEM_BASE, buf, 
           (CONSOLE_HEIGHT * CONSOLE_WIDTH * 2) - offset);
    empty_fill((void *)(CONSOLE_MEM_END - offset), offset);
    return;
}

/** @brief a function to find if cursor is currently hidden
 *  
 *  This function uses the current cursor position to determine
 *  if the cursor is hidden or not. The get_cursor function is 
 *  used to determine the current tow and column of the cursor.
 *  Values exceeding CONSOLE_WIDTH and CONSOLE_HEIGHT are assumed 
 *  to mean the cursor is hidden.
 *
 *  @return 0 if cursor is hidden
 *          -ve integer if cursor is visible
 */
int is_cursor_hidden() {
    if (cursor_row >= CONSOLE_HEIGHT || cursor_col >= CONSOLE_WIDTH) {
        return 0;
    }
    return ERR_INVAL;
}
