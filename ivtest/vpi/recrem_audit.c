/* Copyright (c) 2026 Stephen Williams (steve@icarus.com)
 * SPDX-License-Identifier: GPL-2.0-or-later
 * Observe standard combined RECREM objects and their standalone peers. */
#define AUDIT_COMBINED 1
#define AUDIT_COMBINED_TYPE vpiRecrem
#define AUDIT_LABEL "RECREM"
#define AUDIT_TAG "RRC"
#define AUDIT_PREFIX "rrc"
#define AUDIT_CHECK_TYPE(s) (!strcmp(s,"recrem")?vpiRecrem:!strcmp(s,"recovery")?vpiRecovery:!strcmp(s,"removal")?vpiRemoval:0)
#define AUDIT_LIMIT_COUNT(t) ((t)==vpiRecrem?2:1)
#include "setup_hold_audit.c"
