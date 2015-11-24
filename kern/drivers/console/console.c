/** @file console.c
 *
 *  @brief This file contains functions that implement 
 *         the console driver API. The prototypes of these functions
 *         are defined in p1kern.h
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */

#include <console.h>
#include <x86/video_defines.h>
#include <drivers/console/console_util.h>
#include <simics.h>
#include <common/errors.h>
#include <stddef.h>

/** @brief Sets the position of the cursor to the
 *         position (row, col).
 *
 *  Subsequent calls to putbytes should cause the console
 *  output to begin at the new position. If the cursor is
 *  currently hidden, a call to set_cursor() does not show
 *  the cursor.
 *
 *  @param row The new row for the cursor.
 *  @param col The new column for the cursor.
 *  @return 0 on success, ERR_INVAL if location is invalid
 */
int set_cursor(int row, int col) {
    if (row < 0 || row >= CONSOLE_HEIGHT
        || col < 0 || col >= CONSOLE_WIDTH) {  
            return ERR_INVAL;
    }

    set_logical_cursor(row, col);
    if (is_cursor_hidden() == 0) {
        return 0;
    }
    
    set_hardware_cursor(row, col);

    return 0;
}

/** @brief Prints character ch at the current location
 *         of the cursor.
 *
 *  If the character is a newline ('\n'), the cursor is moved
 *  to the beginning of the next line (scrolling if necessary).
 *  If the character is a carriage return ('\r'), the cursor is
 *  immediately reset to the beginning of the current line,
 *  causing any future output to overwrite any existing output
 *  on the line.  If backspace ('\b') is encountered, the previous
 *  character is erased.
 *
 *  @param ch the character to print
 *  @return The input character
 */
int putbyte(char ch) {
    
    switch (ch) {
        case '\n': set_logical_cursor(cursor_row + 1, 0);
                   break;
        case '\r': set_logical_cursor(cursor_row, 0);
                   break;
        case '\b': if (cursor_col == 0) {
                       set_logical_cursor(cursor_row - 1, CONSOLE_WIDTH - 1);
                   }
                   else {
                       set_logical_cursor(cursor_row, cursor_col - 1);
                   }
                   print_char(' ', cursor_row, cursor_col, console_color);
                   return ch;
                   break;
        default:   if (cursor_col == CONSOLE_WIDTH) {
                       set_logical_cursor(cursor_row + 1, 0);
                   }
                   break;
    }
    if (cursor_row == CONSOLE_HEIGHT) {
        scroll_screen(1);
        set_logical_cursor(cursor_row - 1, cursor_col);
    }
    print_char(ch, cursor_row, cursor_col, console_color);
    if (ch != '\n' && ch != '\r' && ch != '\b') { 
        set_logical_cursor(cursor_row, cursor_col + 1);
    }
    return ch;
}

/** @brief Prints the string s, starting at the current
 *         location of the cursor.
 *
 *  If the string is longer than the current line, the
 *  string fills up the current line and then
 *  continues on the next line. If the string exceeds
 *  available space on the entire console, the screen
 *  scrolls up one line, and then the string
 *  continues on the new line.  If '\n', '\r', and '\b' are
 *  encountered within the string, they are handled
 *  as per putbyte. If len is not a positive integer or s
 *  is null, the function has no effect.
 *
 *  @param s The string to be printed.
 *  @param len The length of the string s.
 *  @return Void.
 */
void putbytes(const char* s, int len) {
    if (!s || len <= 0) {
        return;
    }

    int i;
    for (i = 0; i < len; i++) {
        putbyte(s[i]);
    }
    return;
}

/** @brief Changes the foreground and background color
 *         of future characters printed on the console.
 *
 *  If the color code is invalid, the function has no effect.
 *
 *  @param color The new color code.
 *  @return 0 on success or -ve integer if color code is invalid.
 */
int set_term_color(int color) {
    if (is_valid_color(color) == -1) {
        return ERR_INVAL;
    }
    console_color = color;
    return 0;
}

/** @brief Writes the current foreground and background
 *         color of characters printed on the console
 *         into the argument color.
 *  @param color The address to which the current color
 *         information will be written.
 *  @return Void.
 */
void get_term_color(int *color) {
    if (color != NULL) {
        *color = console_color;
    }
    return;
}

/** @brief Hides the cursor.
 *
 *  Subsequent calls to putbytes do not cause the
 *  cursor to show again.
 *
 *  @return Void.
 */
void hide_cursor() {
    set_hardware_cursor(CONSOLE_HEIGHT + 1, CONSOLE_WIDTH + 1);
    return;
}

/** @brief Shows the cursor.
 *  
 *  If the cursor is already shown, the function has no effect.
 *  The cursor is made visible by setting the hardware cursor to
 *  the current position of the logical cursor.
 *
 *  @return Void.
 */
void show_cursor() {
    set_hardware_cursor(cursor_row, cursor_col);
    return;
}

/** @brief Clears the entire console.
 *
 *  The cursor is reset to the first row and column.
 *  If the cursor is currently hidden it remains so.
 *
 *  @return Void.
 */
void clear_console() {
    empty_fill((void *)CONSOLE_MEM_BASE, CONSOLE_HEIGHT * CONSOLE_WIDTH);
    set_cursor(0, 0);
    return;
}

/** @brief Prints character ch with the specified color
 *         at position (row, col).
 *
 *  If any argument is invalid, the function has no effect.
 *
 *  @param row The row in which to display the character.
 *  @param col The column in which to display the character.
 *  @param ch The character to display.
 *  @param color The color to use to display the character.
 *  @return Void.
 */
void draw_char(int row, int col, int ch, int color) {
    if (row < 0 || row >= CONSOLE_HEIGHT
        || col < 0 || col >= CONSOLE_WIDTH
        || (is_valid_color(color) == -1)) {
        return;
    }
    print_char(ch, row, col, color);
    return;
}

/** @brief Returns the character displayed at position (row, col).
 *
 *  @param row Row of the character.
 *  @param col Column of the character.
 *  @return The character at (row, col). Returns a \0 if the row/col
 *           are invalid
 */
char get_char(int row, int col) {
    if (row < 0 || row >= CONSOLE_HEIGHT
        || col < 0 || col >= CONSOLE_WIDTH) {
        return '\0';
    }
    return *(char *)(CONSOLE_MEM_BASE + 2*(row * CONSOLE_WIDTH + col));
}

/** @brief Returns the background color displayed at position (row, col).
 *
 *  @param row Row of the character.
 *  @param col Column of the character.
 *  @return The color at( row, col). Returns a \0 if the row/col
 *           are invalid
 */
char get_color(int row, int col) {
    if (row < 0 || row >= CONSOLE_HEIGHT
        || col < 0 || col >= CONSOLE_WIDTH) {
        return '\0';
    }
    return *(char *)(CONSOLE_MEM_BASE + 2*(row * CONSOLE_WIDTH + col) + 1);
}

/** @brief returns the current logical cursor position
 *
 *  @param row the location where the row data will be filled.
 *         col the location where column data will be filled.
 *  @return void
 */
void get_cursor(int *row, int *col) {
    if (row != NULL && col != NULL) {
        *row = cursor_row;
        *col = cursor_col;
    }
    return;
}
