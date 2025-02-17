/*
 * Copyright (c) 2015-2025 CERN
 * @author Maciej Suminski <maciej.suminski@cern.ch>
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/**
 * The following VPI module implements some of the functions available
 * in std.textio library.
 *
 * Type counterparts:
 * VHDL             SystemVerilog
 * ------------------------------
 * LINE             string       (line of text, in VHDL it is a pointer to string)
 * TEXT             int          (file handle)
 *
 * Some of functions offered by std.textio library are not implemented here,
 * as they can be directly replaced with SystemVerilog system functions.
 *
 * VHDL                     SystemVerilog
 * --------------------------------------
 * FILE_CLOSE(file F: TEXT) $fclose(fd)
 * ENDFILE(file F: TEXT)    $feof(fd)
 *
 * Procedures:
 * HREAD (L: inout LINE; VALUE: out BIT_VECTOR)
 * HWRITE (L: inout LINE; VALUE: out BIT_VECTOR)
 * are handled with $ivlh_read/write() using FORMAT_HEX parameter (see format_t enum).
 */

# include  "sys_priv.h"
# include  "vpi_config.h"
# include  "vpi_user.h"
# include  <assert.h>
# include  <string.h>
# include  <ctype.h>
# include  <errno.h>
# include  "ivl_alloc.h"

/* additional parameter values to distinguish between integer, boolean and
 * time types or to use hex format */
enum format_t { FORMAT_STD, FORMAT_BOOL, FORMAT_TIME, FORMAT_HEX, FORMAT_STRING };

enum file_mode_t { FILE_MODE_READ, FILE_MODE_WRITE, FILE_MODE_APPEND, FILE_MODE_LAST };
enum file_open_status_t { FS_OPEN_OK, FS_STATUS_ERROR, FS_NAME_ERROR, FS_MODE_ERROR };

/* bits per vector, in a single s_vpi_vecval struct */
static const size_t BPW = 8 * sizeof(PLI_INT32);

/* string buffer size */
static const size_t STRING_BUF_SIZE = 1024;

static int is_integer_var(vpiHandle obj)
{
    PLI_INT32 type = vpi_get(vpiType, obj);

    return (type == vpiIntegerVar || type == vpiShortIntVar ||
            type == vpiIntVar || type == vpiLongIntVar ||
              /* In vlog95 translation this is a signed 32-bit register. */
            (type == vpiReg && vpi_get(vpiSigned, obj) &&
             vpi_get(vpiSize, obj) == 32));
}

static int is_const(vpiHandle obj)
{
    return vpi_get(vpiType, obj) == vpiConstant;
}

static void show_error_line(vpiHandle callh) {
    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
            (int)vpi_get(vpiLineNo, callh));
}

static void show_warning_line(vpiHandle callh) {
    vpi_printf("WARNING: %s:%d: ", vpi_get_str(vpiFile, callh),
            (int)vpi_get(vpiLineNo, callh));
}

/* sets a single bit value in a bit/logic vector */
static int set_vec_val(s_vpi_vecval* vector, char value, int idx) {
    s_vpi_vecval*v = &vector[idx / BPW];
    PLI_INT32 bit = idx % BPW;

    switch(value) {
        case '0':
            v->bval &= ~(1 << bit);
            v->aval &= ~(1 << bit);
            break;

        case '1':
            v->bval &= ~(1 << bit);
            v->aval |= (1 << bit);
            break;

        case 'z':
        case 'Z':
            v->bval |= (1 << bit);
            v->aval &= ~(1 << bit);
            break;

        case 'x':
        case 'X':
            v->bval |= (1 << bit);
            v->aval |= (1 << bit);
            break;

        default:
            return 1;
    }

    return 0;
}

/* Converts a string of characters to a vector in s_vpi_value struct.
 * Returns number of processed characters, 0 in case of failure.
 * string is the data to be converted.
 * val is the target s_vpi_value struct.
 * var is the variable that is converted (to obtain size & type [2/4-state]).
 */
static int read_vector(const char *string, s_vpi_value *val, vpiHandle var)
{
#if 0
    /* It could be easier to simply use val.format = vpiBinStrVal
     * but there is no way to check if processing went fine */
    val.format = vpiBinStrVal;
    val.value.str = string;
    processed_chars = size;
#endif
    /* Vector size (==1 for scalars) */
    int size = vpi_get(vpiSize, var);

    /* Number of required s_vpi_vecval structs to store the result */
    int words = (size + BPW - 1) / BPW; /* == ceil(size / BPW) */
    int len = strlen(string);

    val->format = vpiVectorVal;  /* it also covers scalars */
    val->value.vector = calloc(words, sizeof(s_vpi_vecval));

    /* Skip spaces in the beginning */
    int skipped = 0;
    while(*string && *string == ' ') {
        --len;
        ++string;
        ++skipped;
    }

    /* Process bits */
    int p;
    for(p = 0; p < size && p < len; ++p) {
        if(set_vec_val(val->value.vector, string[p], size - p - 1)) {
            free(val->value.vector);        /* error */
            return 0;
        }
    }

    /* 2-logic variables cannot hold X or Z values, so change them to 0 */
    if(vpi_get(vpiType, var) == vpiBitVar) {
        for(int i = 0; i < words; ++i) {
            val->value.vector[i].aval &= ~val->value.vector[i].bval;
            val->value.vector[i].bval = 0;
        }
    }

    return p + skipped;
}

/* Converts a string of characters to a time value, stored in vector filed of
 * s_vpi_value struct.
 * Returns number of processed characters, 0 in case of failure.
 * string is the data to be converted.
 * val is the target s_vpi_value struct.
 * scope_unit is the time unit used in the scope (-3 for millisecond,
 *      -6 for microsecond, etc.)
 */
static int read_time(const char *string, s_vpi_value *val, PLI_INT32 scope_unit) {
    PLI_UINT64 period;
    char units[3];
    int time_unit, processed_chars;

    if(sscanf(string, "%" PLI_UINT64_FMT " %2s%n", &period, units, &processed_chars) != 2)
        return 0;

    if(!strncasecmp(units, "fs", 2))
        time_unit = -15;
    else if(!strncasecmp(units, "ps", 2))
        time_unit = -12;
    else if(!strncasecmp(units, "ns", 2))
        time_unit = -9;
    else if(!strncasecmp(units, "us", 2))
        time_unit = -6;
    else if(!strncasecmp(units, "ms", 2))
        time_unit = -3;
    else if(!strncasecmp(units, "s", 1))
        time_unit = 0;
    else
        return 0;

    /* Scale the time units to the one used in the scope */
    int scale_diff = time_unit - scope_unit;

    if(scale_diff > 0) {
        for(int i = 0; i < scale_diff; ++i)
            period *= 10;
    } else {
        for(int i = 0; i < -scale_diff; ++i)
            period /= 10;
    }

    /* vpiTimeVal format is not handled at the moment,
     * so return the read value as a vector*/
    val->format = vpiVectorVal;
    val->value.vector = calloc(2, sizeof(s_vpi_vecval));
    memset(val->value.vector, 0, 2 * sizeof(s_vpi_vecval));

    val->value.vector[1].aval = (PLI_UINT32) (period >> 32);
    val->value.vector[0].aval = (PLI_UINT32) period;

    return processed_chars;
}

static int read_string(const char *string, s_vpi_value *val, int count) {
    char buf[STRING_BUF_SIZE];
    int processed_chars;
    char format_str[32];

    /* No string length limit imposed */
    if(count <= 0 || count >= (int)STRING_BUF_SIZE)
        count = STRING_BUF_SIZE - 1;

    snprintf(format_str, 32, "%%%ds%%n", count);

    if(sscanf(string, format_str, buf, &processed_chars) != 1)
        return 0;

    val->format = vpiStringVal;
    val->value.str = strdup(buf);

    return processed_chars;
}

static int write_time(char *string, const s_vpi_value* val,
                       size_t width, PLI_INT32 scope_unit) {
    char prefix = 0;
    PLI_UINT64 period;

    switch(val->format) {
        case vpiIntVal:
            period = val->value.integer;
            break;

        case vpiVectorVal:
            period = val->value.vector[0].aval;

            if(width > BPW)
                period |= (PLI_UINT64)(val->value.vector[1].aval) << 32;
            break;

        default:
            return 1;
    }

    /* Handle the case when the time unit base is 10 or 100 */
    int remainder = scope_unit % -3;
    if(remainder) {
        remainder += 3;
        scope_unit -= remainder;

        while(remainder--)
            period *= 10;
    }

    switch(scope_unit) {
        case -15: prefix = 'f'; break;
        case -12: prefix = 'p'; break;
        case -9: prefix = 'n'; break;
        case -6: prefix = 'u'; break;
        case -3: prefix = 'm'; break;
    }

    if(prefix)
        sprintf(string, "%" PLI_UINT64_FMT " %cs", period, prefix);
    else
        sprintf(string, "%" PLI_UINT64_FMT " s", period);

    return 0;
}

/* slightly modified sys_fopen_compiletf */
static PLI_INT32 ivlh_file_open_compiletf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
    vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
    vpiHandle argv, arg;
    assert(callh != 0);
    int ok = 1;

    argv = vpi_iterate(vpiArgument, callh);

    /* Check that there is a file name argument and that it is a string. */
    if (argv == 0)
        ok = 0;

    arg = vpi_scan(argv);
    if (!arg || !is_integer_var(arg))
        ok = 0;

    arg = vpi_scan(argv);
    if (arg && is_integer_var(arg))
        arg = vpi_scan(argv);

    // no vpi_scan() here, if we had both 'status' and 'file' arguments,
    // then the next arg is read in the above if, otherwise we are going
    // to check the second argument once again
    if (!arg || !is_string_obj(arg))
        ok = 0;

    arg = vpi_scan(argv);
    if (arg && !is_const(arg))
        ok = 0;

    if (!ok) {
        show_error_line(callh);
        vpi_printf("%s() function is available in following variants:\n", name);
        vpi_printf("* (file f: text; filename: in string, file_open_kind: in mode)\n");
        vpi_printf("* (status: out file_open_status, file f: text; filename: in string, file_open_kind: in mode)\n");
	vpip_set_return_value(1);
        vpi_control(vpiFinish, 1);
    }

    /* Make sure there are no extra arguments. */
    check_for_extra_args(argv, callh, name, "four arguments", 1);

    return 0;
}

/* procedure FILE_MODE(file F: TEXT; External_Name; in STRING;
                       Open_Kind: in FILE_MODE_KIND := READ_MODE); */
/* slightly modified sys_fopen_calltf */
static PLI_INT32 ivlh_file_open_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
    vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
    vpiHandle argv = vpi_iterate(vpiArgument, callh);
    s_vpi_value val;
    int mode;
    char *fname;

    vpiHandle fstatush = vpi_scan(argv);
    vpiHandle fhandleh = vpi_scan(argv);
    vpiHandle fnameh = vpi_scan(argv);
    vpiHandle modeh = vpi_scan(argv);

    if(!modeh) {
        /* There are only three arguments, so rearrange handles */
        modeh = fnameh;
        fnameh = fhandleh;
        fhandleh = fstatush;
        fstatush = 0;
    } else {
        vpi_free_object(argv);
    }

    /* Get the mode handle */
    val.format = vpiIntVal;
    vpi_get_value(modeh, &val);
    mode = val.value.integer;

    if(mode < 0 || mode >= FILE_MODE_LAST) {
        show_error_line(callh);
        vpi_printf("%s's file open mode argument is invalid.\n", name);
        return 0;
    }

    fname = get_filename(callh, name, fnameh);

    if(fname == 0) {
        show_error_line(callh);
        vpi_printf("%s's could not obtain the file name.\n", name);
        return 0;
    }

    /* Open file and save the handle */
    PLI_INT32 result = -1;
    switch(mode) {
        case FILE_MODE_READ:
            result = vpi_fopen(fname, "r");
            break;

        case FILE_MODE_WRITE:
            result = vpi_fopen(fname, "w");
            break;

        case FILE_MODE_APPEND:
            result = vpi_fopen(fname, "a");
            break;
    }

    if(fstatush) {
        val.format = vpiIntVal;

        if(!result) {
            switch(errno) {
                case ENOENT:
                case ENAMETOOLONG:
                    val.value.integer = FS_NAME_ERROR;
                    break;

                case EINVAL:
                case EACCES:
                case EEXIST:
                case EISDIR:
                    val.value.integer = FS_MODE_ERROR;
                    break;

                default:
                    val.value.integer = FS_STATUS_ERROR;
                    break;
            }
        } else {
            val.value.integer = FS_OPEN_OK;
        }

        vpi_put_value(fstatush, &val, 0, vpiNoDelay);
    }

    val.format = vpiIntVal;
    val.value.integer = result;
    vpi_put_value(fhandleh, &val, 0, vpiNoDelay);
    free(fname);

    return 0;
}

static PLI_INT32 ivlh_readwriteline_compiletf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
    vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
    vpiHandle argv = vpi_iterate(vpiArgument, callh);
    vpiHandle arg;

    /* Check that there are two arguments and that the first is an
       integer (file handle) and that the second is string. */
    if(argv == 0) {
        show_error_line(callh);
        vpi_printf("%s requires two arguments.\n", name);
	vpip_set_return_value(1);
        vpi_control(vpiFinish, 1);
        return 0;
    }

    arg = vpi_scan(argv);
    if(!arg || !is_integer_var(arg)) {
        show_error_line(callh);
        vpi_printf("%s's first argument must be an integer variable (file handle).\n", name);
	vpip_set_return_value(1);
        vpi_control(vpiFinish, 1);
    }

    arg = vpi_scan(argv);
    if(!arg || !is_string_obj(arg)) {
        show_error_line(callh);
        vpi_printf("%s's second argument must be a string.\n", name);
	vpip_set_return_value(1);
        vpi_control(vpiFinish, 1);
    }

    /* Make sure there are no extra arguments. */
    check_for_extra_args(argv, callh, name, "two arguments", 0);

    return 0;
}

/* procedure READLINE (file F: TEXT; L: inout LINE); */
/* slightly modified sys_fgets_calltf */
static PLI_INT32 ivlh_readline_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
    vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
    vpiHandle argv = vpi_iterate(vpiArgument, callh);
    vpiHandle stringh, arg;
    s_vpi_value val;
    PLI_UINT32 fd;
    FILE *fp;
    char *text;
    char buf[STRING_BUF_SIZE];

    /* Get the file descriptor. */
    arg = vpi_scan(argv);
    val.format = vpiIntVal;
    vpi_get_value(arg, &val);
    fd = val.value.integer;

    /* Get the string handle. */
    stringh = vpi_scan(argv);
    vpi_free_object(argv);

    /* Return zero if this is not a valid fd. */
    fp = vpi_get_file(fd);
    if(!fp) {
        show_warning_line(callh);
        vpi_printf("invalid file descriptor (0x%x) given to %s.\n",
                 (unsigned int)fd, name);
        return 0;
    }

    /* Read in the bytes. Return 0 if there was an error. */
    if(fgets(buf, STRING_BUF_SIZE, fp) == 0) {
        show_error_line(callh);
        vpi_printf("%s reading past the end of file.\n", name);

        return 0;
    }

    int len = strlen(buf);

    if(len == 0) {
        show_error_line(callh);
        vpi_printf("%s read 0 bytes.\n", name);
        return 0;
    } else if(len == STRING_BUF_SIZE - 1) {
        show_warning_line(callh);
        vpi_printf("%s has reached the buffer limit, part of the "
                "processed string might have been skipped.\n", name);
    }

    /* Remove the newline character(s) */
    while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r')) {
        buf[len-1] = 0;
        len--;
    }
    /* Return the characters to the register. */
    text = strdup(buf);
    val.format = vpiStringVal;
    val.value.str = text;
    vpi_put_value(stringh, &val, 0, vpiNoDelay);
    free(text);

    /* Set end-of-file flag if we have just reached the end of the file.
     * Otherwise the flag would be set only after the next read operation. */
    int c = fgetc(fp);
    ungetc(c, fp);

    return 0;
}

/* procedure WRITELINE (file F: TEXT; L: inout LINE); */
/* slightly modified sys_fgets_calltf */
static PLI_INT32 ivlh_writeline_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
    vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
    vpiHandle argv = vpi_iterate(vpiArgument, callh);
    vpiHandle stringh, arg;
    s_vpi_value val;
    PLI_UINT32 fd;
    FILE *fp;
    char *empty;

    /* Get the file descriptor. */
    arg = vpi_scan(argv);
    val.format = vpiIntVal;
    vpi_get_value(arg, &val);
    fd = val.value.integer;

    /* Get the string contents. */
    stringh = vpi_scan(argv);
    val.format = vpiStringVal;
    vpi_get_value(stringh, &val);

    vpi_free_object(argv);

    /* Return zero if this is not a valid fd. */
    fp = vpi_get_file(fd);
    if(!fp) {
        show_warning_line(callh);
        vpi_printf("invalid file descriptor (0x%x) given to %s.\n",
                 (unsigned int)fd, name);
        return 0;
    }

    fprintf(fp, "%s\n", val.value.str);

    /* Clear the written string */
    empty = strdup("");
    val.format = vpiStringVal;
    val.value.str = empty;
    vpi_put_value(stringh, &val, 0, vpiNoDelay);
    free(empty);

    return 0;
}

static PLI_INT32 ivlh_read_compiletf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
    vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
    vpiHandle argv = vpi_iterate(vpiArgument, callh);
    vpiHandle arg;

    if(argv == 0) {
        show_error_line(callh);
        vpi_printf("%s requires three arguments.\n", name);
	vpip_set_return_value(1);
        vpi_control(vpiFinish, 1);
        return 0;
    }

    arg = vpi_scan(argv);
    if(!arg || !is_string_obj(arg)) {
        show_error_line(callh);
        vpi_printf("%s's first argument must be a string.\n", name);
	vpip_set_return_value(1);
        vpi_control(vpiFinish, 1);
    }

    arg = vpi_scan(argv);
    if(!arg || is_constant_obj(arg)) {
        show_error_line(callh);
        vpi_printf("%s's second argument must be a variable.\n", name);
	vpip_set_return_value(1);
        vpi_control(vpiFinish, 1);
        return 0;
    }

    arg = vpi_scan(argv);
    if(!arg) {
        show_error_line(callh);
        vpi_printf("%s's third argument must be an integer.\n", name);
	vpip_set_return_value(1);
        vpi_control(vpiFinish, 1);
        return 0;
    }
    /* Make sure there are no extra arguments. */
    check_for_extra_args(argv, callh, name, "three arguments", 0);

    return 0;
}

/* procedure READ (L: inout LINE;
       VALUE: out BIT/BIT_VECTOR/BOOLEAN/CHARACTER/INTEGER/REAL/STRING/TIME); */
static PLI_INT32 ivlh_read_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
    vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
    vpiHandle argv = vpi_iterate(vpiArgument, callh);
    vpiHandle stringh, varh, formath;
    s_vpi_value val;
    PLI_INT32 type, format, dest_size;
    char *string = 0;
    unsigned int processed_chars = 0, fail = 0;

    /* Get the string */
    stringh = vpi_scan(argv);
    val.format = vpiStringVal;
    vpi_get_value(stringh, &val);

    if(strlen(val.value.str) == 0) {
        show_error_line(callh);
        vpi_printf("%s cannot read from an empty string.\n", name);
        return 0;
    }

    string = strdup(val.value.str);

    /* Get the destination variable */
    varh = vpi_scan(argv);
    type = vpi_get(vpiType, varh);
    dest_size = vpi_get(vpiSize, varh);

    /* Get the format (see enum format_t) */
    formath = vpi_scan(argv);
    val.format = vpiIntVal;
    vpi_get_value(formath, &val);
    format = val.value.integer;
    vpi_free_object(argv);

    switch(format) {
        case FORMAT_STD:
            switch(type) {
                /* TODO longint is 64-bit, so it has to be handled by vector */
                /*case vpiLongIntVar:*/
                case vpiShortIntVar:
                case vpiIntVar:
                case vpiByteVar:
                case vpiIntegerVar:
                    val.format = vpiIntVal;
                    if(sscanf(string, "%d%n", &val.value.integer, &processed_chars) != 1)
                        fail = 1;
                    break;

                case vpiBitVar:   /* bit, bit vector */
                case vpiLogicVar: /* ==vpiReg time, logic, logic vector */
                    processed_chars = read_vector(string, &val, varh);
                    break;

                case vpiRealVar:
                    val.format = vpiRealVal;
                    if(sscanf(string, "%lf%n", &val.value.real, &processed_chars) != 1)
                        fail = 1;
                    break;

                case vpiStringVar:
                    processed_chars = read_string(string, &val, dest_size / 8);
                    break;

                default:
                    fail = 1;
                    show_warning_line(callh);
                    vpi_printf("%s does not handle such type (%d).\n", name, type);
                    break;
                }
            break;

        case FORMAT_BOOL:
        {
            char buf[6];

            val.format = vpiIntVal;
            if(sscanf(string, "%5s%n", buf, &processed_chars) == 1)
            {
                if(!strncasecmp(buf, "true", 4))
                    val.value.integer = 1;
                else if(!strncasecmp(buf, "false", 5))
                    val.value.integer = 0;
                else
                    fail = 1;
            }
        }
            break;

        case FORMAT_TIME:
            val.format = vpiIntVal;
            processed_chars = read_time(string, &val, vpi_get(vpiTimeUnit, callh));
            break;

        case FORMAT_HEX:
            val.format = vpiIntVal;
            if(sscanf(string, "%x%n", &val.value.integer, &processed_chars) != 1)
                fail = 1;
            break;

        case FORMAT_STRING:
            processed_chars = read_string(string, &val, dest_size / 8);
            break;
    }

    if(processed_chars == 0) {
        show_error_line(callh);
        vpi_printf("%s could not read a valid value.\n", name);
        fail = 1;
    } else if(val.format == vpiStringVar && processed_chars == STRING_BUF_SIZE) {
        show_warning_line(callh);
        vpi_printf("%s has reached the buffer limit, part of the "
                "processed string might have been skipped.\n", name);
    }

    if(!fail) {
        assert(processed_chars > 0);

        /* Store the read value */
        vpi_put_value(varh, &val, 0, vpiNoDelay);

        /* Clean up */
        if(val.format == vpiStringVal)
            free(val.value.str);
        else if(val.format == vpiVectorVal)
            free(val.value.vector);

        /* Strip the read token from the string */
        char* tmp = strdup(&string[processed_chars]);
        val.format = vpiStringVal;
        val.value.str = tmp;
        vpi_put_value(stringh, &val, 0, vpiNoDelay);
        free(tmp);
    } else {
        show_error_line(callh);
        vpi_printf("%s failed.\n", name);
	/*vpip_set_return_value(1);*/
        /*vpi_control(vpiFinish, 1);*/
    }

    free(string);

    return 0;
}

static PLI_INT32 ivlh_write_compiletf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
    vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
    vpiHandle argv = vpi_iterate(vpiArgument, callh);
    vpiHandle arg;

    if(argv == 0) {
        show_error_line(callh);
        vpi_printf("%s requires three arguments.\n", name);
	vpip_set_return_value(1);
        vpi_control(vpiFinish, 1);
        return 0;
    }

    arg = vpi_scan(argv);
    if(!arg || !is_string_obj(arg)) {
        show_error_line(callh);
        vpi_printf("%s's first argument must be a string.\n", name);
	vpip_set_return_value(1);
        vpi_control(vpiFinish, 1);
    }

    arg = vpi_scan(argv);
    if(!arg) {
        show_error_line(callh);
        vpi_printf("%s requires three arguments.\n", name);
	vpip_set_return_value(1);
        vpi_control(vpiFinish, 1);
        return 0;
    }

    arg = vpi_scan(argv);
    if(!arg) {
        show_error_line(callh);
        vpi_printf("%s's third argument must be an integer.\n", name);
	vpip_set_return_value(1);
        vpi_control(vpiFinish, 1);
        return 0;
    }
    /* Make sure there are no extra arguments. */
    check_for_extra_args(argv, callh, name, "three arguments", 0);

    return 0;
}

/*procedure WRITE (L: inout LINE;
        VALUE: in BIT/BIT_VECTOR/BOOLEAN/CHARACTER/INTEGER/REAL/STRING/TIME);
        JUSTIFIED: in SIDE:= RIGHT; FIELD: in WIDTH := 0); */
/* JUSTIFIED & FIELD are not handled at the moment */
static PLI_INT32 ivlh_write_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
    vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
    vpiHandle argv = vpi_iterate(vpiArgument, callh);
    vpiHandle stringh, varh, formath;
    s_vpi_value val;
    PLI_INT32 type, format;
    char *string = 0;
    unsigned int fail = 0, res = 0;
    char buf[STRING_BUF_SIZE];

    /* Get the string */
    stringh = vpi_scan(argv);
    val.format = vpiStringVal;
    vpi_get_value(stringh, &val);
    string = strdup(val.value.str);

    /* Get the destination variable */
    varh = vpi_scan(argv);
    type = vpi_get(vpiType, varh);

    /* Get the format (see enum format_t) */
    formath = vpi_scan(argv);
    val.format = vpiIntVal;
    vpi_get_value(formath, &val);
    format = val.value.integer;
    vpi_free_object(argv);

    /* Convert constant types to variable types */
    if(type == vpiConstant) {
        type = vpi_get(vpiConstType, varh);

        switch(type) {
            case vpiRealConst:
                type = vpiRealVar;
                break;

            case vpiStringConst:
                type = vpiStringVar;
                break;

            case vpiDecConst:
            case vpiOctConst:
            case vpiHexConst:
                type = vpiIntVar;
                break;

            case vpiBinaryConst:
                type = vpiBitVar;
                break;
        }
    }

    switch(format) {
        case FORMAT_STD:
            switch(type) {
                /* TODO longint is 64-bit, so it has to be handled by vector */
                /*case vpiLongIntVar:*/
                case vpiShortIntVar:
                case vpiIntVar:
                case vpiByteVar:
                case vpiIntegerVar:
                    val.format = vpiIntVal;
                    vpi_get_value(varh, &val);
                    res = snprintf(buf, STRING_BUF_SIZE, "%s%d", string, val.value.integer);
                    break;

                case vpiBitVar:   /* bit, bit vector */
                case vpiLogicVar: /* ==vpiReg time, logic, logic vector */
                    val.format = vpiBinStrVal;
                    vpi_get_value(varh, &val);

                    /* VHDL stores X/Z values uppercase, so follow the rule */
                    for(size_t i = 0; i< strlen(val.value.str); ++i)
                        val.value.str[i] = toupper((int)val.value.str[i]);

                    res = snprintf(buf, STRING_BUF_SIZE, "%s%s", string, val.value.str);
                    break;

                case vpiRealVar:
                    val.format = vpiRealVal;
                    vpi_get_value(varh, &val);
                    res = snprintf(buf, STRING_BUF_SIZE, "%s%lf", string, val.value.real);
                    break;

                case vpiStringVar:
                    val.format = vpiStringVal;
                    vpi_get_value(varh, &val);
                    res = snprintf(buf, STRING_BUF_SIZE, "%s%s", string, val.value.str);
                    break;

                default:
                    fail = 1;
                    show_warning_line(callh);
                    vpi_printf("%s does not handle such type (%d).\n", name, type);
                    break;
            }
            break;

        case FORMAT_BOOL:
            val.format = vpiIntVal;
            vpi_get_value(varh, &val);
            res = snprintf(buf, STRING_BUF_SIZE, "%s%s", string,
                           val.value.integer ? "TRUE" : "FALSE");
            break;

        case FORMAT_TIME:
            {
            char tmp[64];

            val.format = vpiIntVal;
            vpi_get_value(varh, &val);

            if(write_time(tmp, &val, vpi_get(vpiSize, varh), vpi_get(vpiTimeUnit, callh))) {
                fail = 1;
                break;
            }

            res = snprintf(buf, STRING_BUF_SIZE, "%s%s", string, tmp);
            }
            break;

        case FORMAT_HEX:
            val.format = vpiIntVal;
            vpi_get_value(varh, &val);
            res = snprintf(buf, STRING_BUF_SIZE, "%s%X", string, val.value.integer);
            break;

        case FORMAT_STRING:
            val.format = vpiStringVal;
            vpi_get_value(varh, &val);
            res = snprintf(buf, STRING_BUF_SIZE, "%s%s", string, val.value.str);
            break;
    }

    if(res >= STRING_BUF_SIZE)
        fail = 1;

    if(!fail) {
        /* Strip the read token from the string */
        char* tmp = strdup(buf);
        val.format = vpiStringVal;
        val.value.str = tmp;
        vpi_put_value(stringh, &val, 0, vpiNoDelay);
        free(tmp);
    } else {
        show_error_line(callh);
        vpi_printf("%s failed.\n", name);
	/*vpip_set_return_value(1);*/
        /*vpi_control(vpiFinish, 1);*/
    }

    free(string);

    return 0;
}

static void vhdl_register(void)
{
    vpiHandle res;

    s_vpi_systf_data tf_data[] = {
        { vpiSysTask, 0, "$ivlh_file_open",
          ivlh_file_open_calltf, ivlh_file_open_compiletf, 0,
          "$ivlh_file_open" },

        { vpiSysTask, 0, "$ivlh_readline",
          ivlh_readline_calltf, ivlh_readwriteline_compiletf, 0,
          "$ivlh_readline" },

        { vpiSysTask, 0, "$ivlh_writeline",
          ivlh_writeline_calltf, ivlh_readwriteline_compiletf, 0,
          "$ivlh_writeline" },

        { vpiSysTask, 0, "$ivlh_read",
          ivlh_read_calltf, ivlh_read_compiletf, 0,
          "$ivlh_read" },

        { vpiSysTask, 0, "$ivlh_write",
          ivlh_write_calltf, ivlh_write_compiletf, 0,
          "$ivlh_write" },
    };

    for(unsigned int i = 0; i < sizeof(tf_data) / sizeof(s_vpi_systf_data); ++i) {
        res = vpi_register_systf(&tf_data[i]);
        vpip_make_systf_system_defined(res);
    }
}

void (*vlog_startup_routines[])(void) = {
    vhdl_register,
    0
};
