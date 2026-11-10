/* Copyright (c) 2026 Jerry Zhong
 * SPDX-License-Identifier: GPL-2.0-or-later */
void pulse_history_register(void);
void (*vlog_startup_routines[])(void)={pulse_history_register,0};
