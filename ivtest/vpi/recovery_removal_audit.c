/* Copyright (c) 2026 Stephen Williams (steve@icarus.com)
 * SPDX-License-Identifier: GPL-2.0-or-later
 * Reuse the independent two-terminal object/notifier/callback audit. */
#define AUDIT_LABEL "RECOVERY/REMOVAL"
#define AUDIT_TAG "RR"
#define AUDIT_PREFIX "rr"
#define AUDIT_CHECK_TYPE(s) (!strcmp((s),"recovery")?vpiRecovery:!strcmp((s),"removal")?vpiRemoval:0)
#include "setup_hold_audit.c"
