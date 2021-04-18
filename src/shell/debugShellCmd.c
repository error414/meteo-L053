/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    shell_cmd.c
 * @brief   Simple CLI shell common commands code.
 *
 * @addtogroup SHELL
 * @{
 */

#include <string.h>

#include "ch.h"
#include "hal.h"
#include "shell.h"
#include "debugShellCmd.h"
#include "chprintf.h"

#if ((SHELL_CMD_EXIT_ENABLED == TRUE) && !defined(_CHIBIOS_NIL_)) ||        \
    defined(__DOXYGEN__)
static void cmd_exit(BaseSequentialStream *chp, int argc, char *argv[]) {

  (void)argv;
  if (argc > 0) {
    shellUsage(chp, "exit");
    return;
  }

  shellExit(MSG_OK);
}
#endif

#if (SHELL_CMD_INFO_ENABLED == TRUE) || defined(__DOXYGEN__)
static void cmd_info(BaseSequentialStream *chp, int argc, char *argv[]) {

  (void)argv;
  if (argc > 0) {
    shellUsage(chp, "info");
    return;
  }

  chprintf(chp, "Kernel:       %s" SHELL_NEWLINE_STR, CH_KERNEL_VERSION);
#ifdef PORT_COMPILER_NAME
  chprintf(chp, "Compiler:     %s" SHELL_NEWLINE_STR, PORT_COMPILER_NAME);
#endif
  chprintf(chp, "Architecture: %s" SHELL_NEWLINE_STR, PORT_ARCHITECTURE_NAME);
#ifdef PORT_CORE_VARIANT_NAME
  chprintf(chp, "Core Variant: %s" SHELL_NEWLINE_STR, PORT_CORE_VARIANT_NAME);
#endif
#ifdef PORT_INFO
  chprintf(chp, "Port Info:    %s" SHELL_NEWLINE_STR, PORT_INFO);
#endif
#ifdef PLATFORM_NAME
  chprintf(chp, "Platform:     %s" SHELL_NEWLINE_STR, PLATFORM_NAME);
#endif
#ifdef BOARD_NAME
  chprintf(chp, "Board:        %s" SHELL_NEWLINE_STR, BOARD_NAME);
#endif
#ifdef __DATE__
#ifdef __TIME__
  chprintf(chp, "Build time:   %s%s%s" SHELL_NEWLINE_STR, __DATE__, " - ", __TIME__);
#endif
#endif
}
#endif

#if (SHELL_CMD_ECHO_ENABLED == TRUE) || defined(__DOXYGEN__)
static void cmd_echo(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)argv;
  if (argc != 1) {
    shellUsage(chp, "echo \"message\"");
    return;
  }
  chprintf(chp, "%s" SHELL_NEWLINE_STR, argv[0]);
}
#endif

#if (SHELL_CMD_SYSTIME_ENABLED == TRUE) || defined(__DOXYGEN__)
static void cmd_systime(BaseSequentialStream *chp, int argc, char *argv[]) {

  (void)argv;
  if (argc > 0) {
    shellUsage(chp, "systime");
    return;
  }
  chprintf(chp, "%lu" SHELL_NEWLINE_STR, (unsigned long)chVTGetSystemTime());
}
#endif

#if (SHELL_CMD_MEM_ENABLED == TRUE) || defined(__DOXYGEN__)
static void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[]) {
  size_t n, total, largest;

  (void)argv;
  if (argc > 0) {
    shellUsage(chp, "mem");
    return;
  }
  n = chHeapStatus(NULL, &total, &largest);
  chprintf(chp, "core free memory : %u bytes" SHELL_NEWLINE_STR, chCoreGetStatusX());
  chprintf(chp, "heap fragments   : %u" SHELL_NEWLINE_STR, n);
  chprintf(chp, "heap free total  : %u bytes" SHELL_NEWLINE_STR, total);
  chprintf(chp, "heap free largest: %u bytes" SHELL_NEWLINE_STR, largest);
}
#endif

#if (SHELL_CMD_THREADS_ENABLED == TRUE) || defined(__DOXYGEN__)
static void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]) {
  static const char *states[] = {CH_STATE_NAMES};
  thread_t *tp;
  size_t sz;
	uint32_t used_pct;
	size_t n = 0;
  (void)argv;
  if (argc > 0) {
    shellUsage(chp, "threads");
    return;
  }
  chprintf(chp, "  size    used   used %%     prio     state         name\r\n");
  chprintf(chp, "--------------------------------------------------------------------\r\n");
  tp = chRegFirstThread();
  do {
	  n = 0;
#if (CH_DBG_ENABLE_STACK_CHECK == TRUE) || (CH_CFG_USE_DYNAMIC == TRUE)
    uint32_t stklimit = (uint32_t)tp->wabase;
#else
    uint32_t stklimit = 0U;
#endif

	uint8_t *begin = (uint8_t *)stklimit;
	uint8_t *end = (uint8_t *)tp;
	sz = end - begin;

	  while(begin < end)
		  if(*begin++ == CH_DBG_STACK_FILL_VALUE) ++n;


	  used_pct = 100 - (n * 100) / sz;
    chprintf(chp, "%6u   %6u  %6u%%   %3u      %9s %12s" SHELL_NEWLINE_STR,
             sz,
             sz - n,
             used_pct,
             (uint32_t)tp->realprio,
             states[tp->state],
             tp->name == NULL ? "" : tp->name);
    tp = chRegNextThread(tp);
  } while (tp != NULL);
}
#endif

/*===========================================================================*/
/* Module exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Array of the default commands.
 */

const ShellCommand shellCommands[] = {
#if (SHELL_CMD_EXIT_ENABLED == TRUE) && !defined(_CHIBIOS_NIL_)
  {"exit", cmd_exit},
#endif
#if SHELL_CMD_INFO_ENABLED == TRUE
  {"info", cmd_info},
#endif
#if SHELL_CMD_ECHO_ENABLED == TRUE
  {"echo", cmd_echo},
#endif
#if SHELL_CMD_SYSTIME_ENABLED == TRUE
  {"systime", cmd_systime},
#endif
#if SHELL_CMD_MEM_ENABLED == TRUE
  {"mem", cmd_mem},
#endif
#if SHELL_CMD_THREADS_ENABLED == TRUE
  {"threads", cmd_threads},
#endif
  {NULL, NULL}
};

/** @} */
