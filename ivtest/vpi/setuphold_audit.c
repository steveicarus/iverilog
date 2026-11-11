/* Copyright (c) 2026 Stephen Williams (steve@icarus.com)
 * SPDX-License-Identifier: GPL-2.0-or-later */
#define AUDIT_SETUPHOLD 1
#define AUDIT_LABEL "SETUPHOLD"
#define AUDIT_TAG "SHC"
#define AUDIT_PREFIX "shc"
#define AUDIT_CHECK_TYPE(s) (!strcmp(s,"setuphold")?vpiSetupHold:!strcmp(s,"setup")?vpiSetup:!strcmp(s,"hold")?vpiHold:0)
#define AUDIT_LIMIT_COUNT(t) ((t)==vpiSetupHold?2:1)
#include "setup_hold_audit.c"
