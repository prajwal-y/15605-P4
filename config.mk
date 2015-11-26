###########################################################################
#
#    #####          #######         #######         ######            ###
#   #     #            #            #     #         #     #           ###
#   #                  #            #     #         #     #           ###
#    #####             #            #     #         ######             #
#         #            #            #     #         #
#   #     #            #            #     #         #                 ###
#    #####             #            #######         #                 ###
#
#
# Please read the directions in README and in this config.mk carefully.
# Do -N-O-T- just dump things randomly in here until your kernel builds.
# If you do that, you run an excellent chance of turning in something
# which can't be graded.  If you think the build infrastructure is
# somehow restricting you from doing something you need to do, contact
# the course staff--don't just hit it with a hammer and move on.
#
# [Once you've read this message, please edit it out of your config.mk]
# [Once you've read this message, please edit it out of your config.mk]
# [Once you've read this message, please edit it out of your config.mk]
###########################################################################

###########################################################################
# This is the include file for the make file.
# You should have to edit only this file to get things to build.
###########################################################################

###########################################################################
# Tab stops
###########################################################################
# If you use tabstops set to something other than the international
# standard of eight characters, this is your opportunity to inform
# our print scripts.
TABSTOP = 4

###########################################################################
# The method for acquiring project updates.
###########################################################################
# This should be "afs" for any Andrew machine, "web" for non-andrew machines
# and "offline" for machines with no network access.
#
# "offline" is strongly not recommended as you may miss important project
# updates.
#
UPDATE_METHOD = afs

###########################################################################
# WARNING: When we test your code, the two TESTS variables below will be
# blanked.  Your kernel MUST BOOT AND RUN if 410TESTS and STUDENTTESTS
# are blank.  It would be wise for you to test that this works.
###########################################################################

###########################################################################
# Test programs provided by course staff you wish to run
###########################################################################
# A list of the test programs you want compiled in from the 410user/progs
# directory.
#
410TESTS = fork_exit_bomb fork_wait_bomb fork_bomb fork_wait actual_wait \
		   new_pages readline_basic remove_pages_test1 remove_pages_test2 \
		   getpid_test1 loader_test1 loader_test2 exec_basic exec_basic_helper \
		   exec_nonexist fork_test1 print_basic stack_test1 wait_getpid sleep_test1 \
		   make_crash make_crash_helper cho wild_test1 yield_desc_mkrun \
		   cho2 cho_variant swexn_basic_test swexn_cookie_monster swexn_regs \
		   swexn_dispatch swexn_uninstall_test swexn_stands_for_swextensible \
		   mem_permissions deschedule_hang halt_test mem_eat_test minclone_mem \
		   register_test chow fib ack slaughter coolness mandelbrot

###########################################################################
# Test programs you have written which you wish to run
###########################################################################
# A list of the test programs you want compiled in from the user/progs
# directory.
#
STUDENTTESTS =

###########################################################################
# Data files provided by course staff to build into the RAM disk
###########################################################################
# A list of the data files you want built in from the 410user/files
# directory.
#
410FILES =

###########################################################################
# Data files you have created which you wish to build into the RAM disk
###########################################################################
# A list of the data files you want built in from the user/files
# directory.
#
STUDENTFILES =

###########################################################################
# Object files for your thread library
###########################################################################
THREAD_OBJS = malloc.o panic.o mutex.o asm.o cond_var.o thread.o rwlock.o list.o

# Thread Group Library Support.
#
# Since libthrgrp.a depends on your thread library, the "buildable blank
# P3" we give you can't build libthrgrp.a.  Once you install your thread
# library and fix THREAD_OBJS above, uncomment this line to enable building
# libthrgrp.a:
410USER_LIBS_EARLY += libthrgrp.a

###########################################################################
# Object files for your syscall wrappers
###########################################################################
SYSCALL_OBJS = sys_print.o set_status.o vanish.o deschedule.o sys_exec.o fork.o \
               getchar.o get_cursor_pos.o get_ticks.o gettid.o halt.o \
			   make_runnable.o misbehave.o new_pages.o readfile.o sys_readline.o \
			   remove_pages.o set_cursor_pos.o set_term_color.o sleep.o \
			   swexn.o task_vanish.o wait.o yield.o udriv_register.o \
			   udriv_deregister.o udriv_send.o udriv_wait.o udriv_inb.o \
			   udriv_outb.o udriv_mmap.o

###########################################################################
# Object files for your automatic stack handling
###########################################################################
AUTOSTACK_OBJS = autostack.o

###########################################################################
# Parts of your kernel
###########################################################################
#
# Kernel object files you want included from 410kern/
#
410KERNEL_OBJS = load_helper.o drivers/console/console.o
#
# Kernel object files you provide in from kern/
#
KERNEL_OBJS = kernel.o loader/loader.o list/list.o drivers/console/console.o \
			  drivers/console/console_util.o drivers/timer/timer.o drivers/timer/timer_handler.o \
			  interrupts/interrupt_handlers.o interrupts/idt_entry.o interrupts/fault_handlers.o \
			  interrupts/fault_handlers_asm.o \
			  drivers/keyboard/keyboard.o drivers/keyboard/keyboard_handler.o allocator/frame_allocator.o \
			  sync/mutex.o sync/cond_var.o  sync/sem.o \
			  vm/vm.o core/task.o core/thread.o core/fork.o asm/asm.o syscalls/syscall_handlers.o \
			  syscalls/thread_syscalls.o syscalls/thread_syscalls_asm.o syscalls/console_syscalls.o \
			  syscalls/console_syscalls_asm.o syscalls/lifecycle_syscalls.o syscalls/lifecycle_syscalls_asm.o \
			  common/assert.o common/malloc_wrappers.o core/context.o core/scheduler.o core/exec.o syscalls/misc_syscalls.o \
			  syscalls/misc_syscalls_asm.o core/wait_vanish.o syscalls/memory_syscalls.o syscalls/memory_syscalls_asm.o \
			  drivers/keyboard/keyboard_circular_buffer.o syscalls/system_check_syscalls.o \
			  syscalls/system_check_syscalls_asm.o core/sleep.o	syscalls/syscall_util.o \
			  interrupts/device_handlers.o interrupts/device_handlers_asm.o \
			  udriv/udriv.o syscalls/udriv_syscalls_asm.o


###########################################################################
# WARNING: Do not put **test** programs into the REQPROGS variables.  Your
#          kernel will probably not build in the test harness and you will
#          lose points.
###########################################################################

###########################################################################
# Mandatory programs whose source is provided by course staff
###########################################################################
# A list of the programs in 410user/progs which are provided in source
# form and NECESSARY FOR THE KERNEL TO RUN.
#
# The shell is a really good thing to keep here.  Don't delete idle
# or init unless you are writing your own, and don't do that unless
# you have a really good reason to do so.
#
410REQPROGS = idle init shell

###########################################################################
# Mandatory programs whose source is provided by you
###########################################################################
# A list of the programs in user/progs which are provided in source
# form and NECESSARY FOR THE KERNEL TO RUN.
#
# Leave this blank unless you are writing custom init/idle/shell programs
# (not generally recommended).  If you use STUDENTREQPROGS so you can
# temporarily run a special debugging version of init/idle/shell, you
# need to be very sure you blank STUDENTREQPROGS before turning your
# kernel in, or else your tweaked version will run and the test harness
# won't.
#
STUDENTREQPROGS =
