/*
 * Copyright (c) 2026 Lars-Peter Clausen <lars@metafoo.de>
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

# include  "CmdExec.h"

# include  <stdlib.h>
# include  <string.h>

#ifdef __MINGW32__
# if !defined(_WIN32_WINNT) || _WIN32_WINNT < 0x0600
#  undef _WIN32_WINNT
#  define _WIN32_WINNT 0x0600
# endif
# if !defined(WINVER) || WINVER < 0x0600
#  undef WINVER
#  define WINVER 0x0600
# endif
# include  <fcntl.h>
# include  <io.h>
# include  <stdint.h>
# include  <windows.h>

static char *get_cmd_exe(void)
{
	DWORD n = GetEnvironmentVariableA("COMSPEC", NULL, 0);
	char *buf;

	if (n > 0) {
		buf = (char *)malloc(n);
		if (buf == NULL)
			return NULL;

		DWORD len = GetEnvironmentVariableA("COMSPEC", buf, n);

		if (len > 0 && len < n)
			return buf;

		free(buf);
	}

	/* Fallback. This avoids searching PATH for cmd.exe. */
	{
		char sysdir[MAX_PATH];
		UINT len = GetSystemDirectoryA(sysdir, sizeof(sysdir));

		if (len > 0 && len < sizeof(sysdir)) {
			static const char suffix[] = "\\cmd.exe";
			size_t need = strlen(sysdir) + sizeof(suffix);

			buf = (char *)malloc(need);
			if (buf == NULL)
				return NULL;

			strcpy(buf, sysdir);
			strcat(buf, suffix);
			return buf;
		}
	}

	return _strdup("cmd.exe");
}

/*
 * On Windows `system()` runs the string as cmd.exe /C <command>.
 * `popen()` similarly runs the string through CMD.exe and attaches one side of
 * a pipe.
 *
 * cmd.exe has special quote handling for /C. Quote characters are preserved
 * only if all of the following are true:
 *	- there is no /S switch;
 *	- there are exactly two quote characters;
 *	- there are no special characters between them, where special is one
 *	  of &<>()@^|;
 *	- there is whitespace between them;
 *	- the string between them is the name of an executable file.
 *
 * For an input like "C:\Program Files\...\ivlpp" -F"...\defs" "input file.v"
 * the quotes at the beginning and the end get stripped, which results in
 * incorrect behavior.
 *
 * Below are versions of `system()` and `popen()` that call cmd.exe with /C /S,
 * which will preserve the quotes. Select cmd.exe from COMSPEC, with a fallback
 * to the system directory, and pass it as the application name so PATH is not
 * searched.
 *
 * Also pass /D to suppress AutoRun commands so a registry setting cannot
 * modify this parsing path.
 */
static int start_cmd_process(const char *cmd, HANDLE stdout_handle,
			     PROCESS_INFORMATION *pi,
			     DWORD *last_error)
{
	STARTUPINFOEX siex;
	STARTUPINFO si;
	STARTUPINFO *startup_info;
	HANDLE child_stdout = NULL;
	HANDLE inherit_handles[3];
	DWORD creation_flags = 0;
	BOOL inherit = FALSE;
	SIZE_T attr_size = 0;
	unsigned inherit_count = 0;
	char *cmd_exe = get_cmd_exe();
	size_t cmd2len;
	char *cmd2;
	LPPROC_THREAD_ATTRIBUTE_LIST attr_list = NULL;
	int attr_list_initialized = 0;
	int rc = -1;

	if (last_error != NULL)
		*last_error = 0;

	if (cmd_exe == NULL)
		return -1;

	cmd2len = strlen(cmd_exe) + strlen(cmd) + 16;
	cmd2 = (char *)malloc(cmd2len);
	if (cmd2 == NULL)
		goto out;

	snprintf(cmd2, cmd2len, "\"%s\" /D /S /C \"%s\"", cmd_exe, cmd);

	memset(&si, 0x00, sizeof(si));
	si.cb = sizeof(si);
	startup_info = &si;

	if (stdout_handle != NULL) {
		HANDLE std_input = GetStdHandle(STD_INPUT_HANDLE);
		HANDLE std_error = GetStdHandle(STD_ERROR_HANDLE);

		memset(&siex, 0x00, sizeof(siex));
		siex.StartupInfo.cb = sizeof(siex);
		siex.StartupInfo.dwFlags = STARTF_USESTDHANDLES;

		if (!DuplicateHandle(GetCurrentProcess(), stdout_handle, GetCurrentProcess(),
				     &child_stdout, 0, TRUE, DUPLICATE_SAME_ACCESS))
			goto out;

		/* The normal system()/popen() path already relies on standard
		 * handles being inheritable. Only the pipe writer is duplicated.
		 */
		if (std_input != NULL && std_input != INVALID_HANDLE_VALUE)
			inherit_handles[inherit_count++] = std_input;
		inherit_handles[inherit_count++] = child_stdout;
		if (std_error != NULL && std_error != INVALID_HANDLE_VALUE)
			inherit_handles[inherit_count++] = std_error;

		siex.StartupInfo.hStdInput = std_input;
		siex.StartupInfo.hStdOutput = child_stdout;
		siex.StartupInfo.hStdError = std_error;

		InitializeProcThreadAttributeList(NULL, 1, 0, &attr_size);
		attr_list = (LPPROC_THREAD_ATTRIBUTE_LIST)malloc(attr_size);
		if (attr_list == NULL)
			goto out;
		if (!InitializeProcThreadAttributeList(attr_list, 1, 0, &attr_size))
			goto out;
		attr_list_initialized = 1;
		if (!UpdateProcThreadAttribute(attr_list, 0,
					       PROC_THREAD_ATTRIBUTE_HANDLE_LIST,
					       inherit_handles,
					       inherit_count * sizeof(inherit_handles[0]),
					       NULL, NULL))
			goto out;

		siex.lpAttributeList = attr_list;
		startup_info = &siex.StartupInfo;
		creation_flags = EXTENDED_STARTUPINFO_PRESENT;
		inherit = TRUE;
	}
	memset(pi, 0x00, sizeof(*pi));

	if (!CreateProcess(cmd_exe, cmd2, NULL, NULL, inherit,
			   creation_flags, NULL, NULL, startup_info, pi)) {
		if (last_error != NULL)
			*last_error = GetLastError();
		goto out;
	}

	rc = 0;

out:
	if (attr_list_initialized)
		DeleteProcThreadAttributeList(attr_list);
	if (attr_list != NULL)
		free(attr_list);
	if (child_stdout != NULL)
		CloseHandle(child_stdout);
	free(cmd2);
	free(cmd_exe);
	return rc;
}
#endif

int ivl_run_cmd(const char *cmd, int verbose)
{
	if (verbose)
		fprintf(stderr, "Executing: %s", cmd);

#ifdef __MINGW32__
	DWORD exit_code = 1;
	DWORD last_error;
	PROCESS_INFORMATION pi;

	if (start_cmd_process(cmd, NULL, &pi, &last_error) != 0) {
		if (last_error != 0)
			fprintf(stderr, "CreateProcess failed (%lu).\n", last_error);
		return -1;
	}

	WaitForSingleObject(pi.hProcess, INFINITE);
	GetExitCodeProcess(pi.hProcess, &exit_code);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return exit_code;
#else
	return system(cmd);
#endif
}

FILE *ivl_run_cmd_pipe(const char *cmd)
{
#ifdef __MINGW32__
	SECURITY_ATTRIBUTES sa;
	HANDLE read_pipe = NULL;
	HANDLE write_pipe = NULL;
	PROCESS_INFORMATION pi;
	int fd;
	FILE *fp;

	memset(&sa, 0x00, sizeof(sa));
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = FALSE;

	if (!CreatePipe(&read_pipe, &write_pipe, &sa, 0))
		return NULL;

	if (start_cmd_process(cmd, write_pipe, &pi, NULL) != 0) {
		CloseHandle(read_pipe);
		CloseHandle(write_pipe);
		return NULL;
	}

	CloseHandle(write_pipe);
	CloseHandle(pi.hThread);

	/* Match popen("r") text-mode semantics. The lexer expects CRLF from
	 * Windows command output to be normalized before it sees `line directives.
	 */
	fd = _open_osfhandle((intptr_t)read_pipe, _O_RDONLY | _O_TEXT);
	if (fd < 0) {
		CloseHandle(read_pipe);
		TerminateProcess(pi.hProcess, 1);
		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
		return NULL;
	}

	fp = _fdopen(fd, "rt");
	if (fp == NULL) {
		_close(fd);
		TerminateProcess(pi.hProcess, 1);
		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
		return NULL;
	}

	/* The child owns the pipe writer. We do not need the process handle
	 * unless we want pclose() style exit status.
	 */
	CloseHandle(pi.hProcess);
	return fp;
#else
	return popen(cmd, "r");
#endif
}

int ivl_close_cmd_pipe(FILE *fp)
{
#ifdef __MINGW32__
	return fclose(fp);
#else
	return pclose(fp);
#endif
}
