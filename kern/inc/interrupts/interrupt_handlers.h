/** @file interrupt_handlers.h
 *  @brief prototypes for interrupt handling utility
 *         functions
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 */
#ifndef __INTERRUPT_HANDLERS_H
#define __INTERRUPT_HANDLERS_H

/* Serial device offsets */
#define DATA_REG_OFFSET 0
#define INT_ENABLE_REG_OFFSET 1
#define BAUD_DIVIDER_LSB_OFFSET 0 
#define BAUD_DIVIDER_MSB_OFFSET 1
#define FIFO_CNTL_REG_OFFSET 2
#define INT_ID_REG_OFFSET 2
#define LINE_CNTL_REG_OFFSET 3
#define MODEM_CNTL_REG_OFFSET 4
#define LINE_STATUS_REG_OFFSET 5
#define MODEM_STATUS_REG_OFFSET 6
#define SCRATCH_REG_OFFSET 7

void acknowledge_interrupt();

int install_handlers();

#endif  /* __INTERRUPT_HANDLERS_H */
