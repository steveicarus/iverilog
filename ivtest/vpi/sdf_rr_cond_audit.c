/* Copyright (c) 2026 Stephen Williams (steve@icarus.com)
 * SPDX-License-Identifier: GPL-2.0-or-later */
#define AUDIT_LABEL "RECOVERY/REMOVAL"
#define AUDIT_TAG "RR"
#define AUDIT_PREFIX "rr"
#define AUDIT_CHECK_TYPE(s) (!strcmp((s),"recovery")?vpiRecovery:!strcmp((s),"removal")?vpiRemoval:0)
#include "sdf_tchk_cond_audit.c"
