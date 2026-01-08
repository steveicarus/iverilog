'''Functions for running Icarus Verilog

'''

import subprocess
import difflib
import os
import sys
import re

def assemble_iverilog_cmd(options: dict, cfg: dict, outfile: str) -> list:
    '''Build the iverilog command line'''
    res = []
    if cfg['with-valgrind']:
        res += ["valgrind", "--trace-children=yes"]
    res += ["iverilog"+cfg['suffix'], "-o", os.path.join("work", outfile)]
    res += options['iverilog_args']
    res += [options['source']]
    return res


def assemble_vvp_cmd(options: dict, cfg: dict) -> list:
    '''Build the vvp command line'''
    res = []
    if cfg['with-valgrind']:
        res += ["valgrind", "--leak-check=full", "--show-reachable=yes"]
    res += ["vvp"+cfg['suffix']]
    res += options['vvp_args']
    res.append(os.path.join("work", "a.out"))
    res += options['vvp_args_extended']
    return res


def get_ivl_version (suffix: str) -> int:
    '''Figure out the version of the installed iverilog compler.

    The return value is a list of 2 numbers, the major and minor version
    numbers, or None if the version string couldn't be found.'''

    # Get the output from the "iverilog -V" command for the version string.
    text = subprocess.check_output(["iverilog"+suffix, "-V"])
    match = re.search(b'Icarus Verilog version ([0-9]+)\\.([0-9]+)', text)
    if not match:
        return None

    items = match.groups()
    return int(items[0])

def build_runtime(it_key: str) -> None:
    '''Check and prepare the runtime environment for a test

    This is called in front of tests to make sure that the directory
    structure is correct, and common temp files that might linger from
    a previous run are removed. We need to make sure that the directories
    "work" and "log" are present, and the log files related to this key
    are removed.'''

    try:
        os.mkdir("log")
    except FileExistsError:
        pass

    try:
        os.remove(os.path.join("log", it_key + ".log"))
    except FileNotFoundError:
        pass

    try:
        os.mkdir("work")
    except FileExistsError:
        pass

    try:
        os.remove(os.path.join("work", "a.out"))
    except FileNotFoundError:
        pass

def get_log_file(key: str, title: str, stream: str) -> str:
    '''Get the name of the log file'''
    # pylint: disable-next=consider-using-f-string
    res = "{key}-{title}-{stream}.log".format(key=key, title=title, stream=stream)
    return res

def log_results(key, title, res) -> None:
    ''' Write results into log files.

    Generate a log file with the name of the key and title, and
    put the stdout and stderr into separate files.'''

    with open(os.path.join("log", get_log_file(key, title, "stdout")), 'wb') as fd:
        fd.write(res.stdout)

    with open(os.path.join("log", get_log_file(key, title, "stderr")), 'wb') as fd:
        fd.write(res.stderr)


def compare_files(log_path, gold_path):
    '''Compare the log file and the gold file

    The files are read it, line at a time, and the lines are compared.
    If they differ, then write to stdout a unified diff. In any case,
    return True or False to indicate the results of the test.'''

    with open(log_path, 'rt', encoding='ascii') as fd:
        a_raw = fd.readlines()
    # Remove the valgrind lines from the log. They start with ==PID==
    # and error messages are **PID**
    a = list(filter(lambda item: not re.match('(==\\d+==)|(\\*\\*\\d+\\*\\*)', item), a_raw))

    # Allow to omit empty gold files
    if os.path.exists(gold_path):
        with open(gold_path, 'rt', encoding='ascii') as fd:
            b = fd.readlines()
    else:
        b = []

    if a != b:
        # pylint: disable-next=consider-using-f-string
        print("{gold} and {log} differ:".format(gold=gold_path,log=log_path))
        sys.stdout.writelines(difflib.unified_diff(b, a, gold_path, log_path))
        return False

    return True


def run_cmd(cmd: list) -> subprocess.CompletedProcess:
    '''Run the given command'''
    res = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False)
    return res


# pylint: disable-next=invalid-name
def run_CE(options: dict, cfg: dict) -> list:
    ''' Run the compiler, and expect an error

    In this case, we assert that the command fails to run and reports
    an error. This is to check that invalid input generates errors.'''

    it_key = options['key']
    build_runtime(it_key)

    # Run as vlog95 if needed.
    if cfg['vlog95']:
        options['iverilog_args'].extend(["-tvlog95", "-pfileline=1", "-pspacing=4"])
        ivl_cmd = assemble_iverilog_cmd(options, cfg, 'vlog95.v')
    else:
        ivl_cmd = assemble_iverilog_cmd(options, cfg, 'a.out')

    res = run_cmd(ivl_cmd)
    log_results(it_key, "iverilog", res)

    log_list = ["iverilog-stdout", "iverilog-stderr"]

    if res.returncode == 0:
        return [1, "Failed - CE (no error reported)."]
    if res.returncode >= 256:
        return [1, "Failed - CE (execution error)."]
    if options['gold'] is not None and not check_gold(options, log_list):
        return [1, "Failed - CE (Gold file doesn't match output)."]
    return [0, "Passed - CE."]


def check_gold(options: dict, log_list: list) -> list:
    '''Check if the log and gold file match'''
    compared = True
    for log_name in log_list:
        # pylint: disable-next=consider-using-f-string
        log_path = os.path.join("log", "{key}-{log}.log".format(key=options['key'],
                                                                log=log_name))
        # pylint: disable-next=consider-using-f-string
        gold_path = os.path.join("gold", "{gold}-{log}.gold".format(gold=options['gold'],
                                                                    log=log_name))
        compared = compared and compare_files(log_path, gold_path)

    if compared:
        return [0, "Passed."]
    return [1, "Failed - Gold output doesn't match actual output."]


def check_diff(fname1: str, fname2: str, skip: int, expected_fail: bool) -> list:
    '''Check if the difference files match after skipping the number of
       specified lines'''
    with open(fname1, 'rt', encoding='ascii') as fd:
        # pylint: disable-next=unused-variable
        for idx in range(skip):
            fd.readline()
        data1 = fd.read()

    with open(fname2, 'rt', encoding='ascii') as fd:
        # pylint: disable-next=unused-variable
        for idx in range(skip):
            fd.readline()
        data2 = fd.read()

    if data1 == data2:
        return [0, "Passed."]
    if expected_fail:
        return [0, "Passed - EF."]
    # pylint: disable-next=consider-using-f-string
    return [1, "Failed - Files {name1} and {name2} differ.".format(name1=fname1,
                                                                  name2=fname2)]

def check_run_outputs(options: dict, it_stdout: str, log_list: list,
                      expected_fail: bool) -> list:
    '''Check the output files, and return success or failed.

    This function takes an options dictionary that describes the settings, and
    the output from the final command. This also takes a list of log files to check
    if there are gold files present.'''

    # Check the results against the gold file if it exists.
    if options['gold'] is not None:
        return check_gold(options, log_list)

    # If there is a diff description, then compare named files instead of
    # the log and a gold file.
    it_diff = options['diff']
    if it_diff is not None:
        return check_diff(it_diff[0], it_diff[1], it_diff[2], expected_fail)

    # Otherwise, look for the PASSED output string in stdout.
    for line in it_stdout.splitlines():
        if line == "PASSED":
            return [0, "Passed."]

    # If there is no PASSED output, and nothing else to check, then
    # assume a failure unless a fail is expected.
    if expected_fail:
        return [0, "Passed - EF."]
    return [1, "Failed - No PASSED output, and no gold file."]


def options_to_pass(options: dict) -> list:
    '''Options to pass to the translated compile stage.'''
    rtn = []
    if "-gspecify" in options['iverilog_args']:
        rtn.append("-gspecify")
    if "-ginterconnect" in options['iverilog_args']:
        rtn.append("-ginterconnect")
    if "-Tmin" in options['iverilog_args']:
        rtn.append("-Tmin")
    if "-Ttyp" in options['iverilog_args']:
        rtn.append("-Ttyp")
    if "-Tmax" in options['iverilog_args']:
        rtn.append("-Tmax")
    return rtn

def build_ivl_return(translation_fail: bool, res: subprocess.CompletedProcess) -> list:
    '''Generate the return for the iverilog run.'''
    if translation_fail:
        if res.returncode != 0:
            return [0, "Passed - TE."]
        return [1, "Failed - TE did not fail."]
    if res.returncode != 0:
        return [1, "Failed - Compile failed."]
    return[]

def build_vvp_return(expected_fail: bool, res: subprocess.CompletedProcess) -> list:
    '''Generate the return for the vvp run.'''
    if res.returncode != 0 and expected_fail:
        return [0, "Passed - EF."]
    if res.returncode >= 256:
        return [1, "Failed - vvp execution error"]
    if res.returncode > 0 and res.returncode < 256 and not expected_fail:
        return [1, "Failed - vvp error, but expected to succeed"]
    return []

def do_run_normal(options: dict, cfg: dict, expected_fail: bool,
                  translation_fail: bool) -> list:
    '''Run the iverilog and vvp commands.

    In this case, run the compiler to generate a vvp output file, and
    run the vvp command to actually execute the simulation. Collect
    the results and look for a "PASSED" string.'''

    it_key = options['key']
    build_runtime(it_key)

    # Run the vlog95 translation if needed.
    if cfg['vlog95']:
        options['iverilog_args'].extend(["-tvlog95", "-pfileline=1", "-pspacing=4"])
        ivl_tcmd = assemble_iverilog_cmd(options, cfg, 'vlog95.v')
        ivl_tres = run_cmd(ivl_tcmd)

        log_results(it_key, "iverilog", ivl_tres)
        if ivl_tres.returncode != 0:
            return [1, "Failed - vlog95 translation failed."]

        saved_options = options_to_pass(options)
        if "-pallowsigned=1" in options['iverilog_args']:
            options['iverilog_args'] = [ "-g2001-noconfig" ]
        else:
            options['iverilog_args'] = [ "-g1995" ]
        options['iverilog_args'].extend(saved_options)
        options['source'] = os.path.join("work", "vlog95.v")

    # Run the iverilog command
    ivl_cmd = assemble_iverilog_cmd(options, cfg, 'a.out')
    ivl_res = run_cmd(ivl_cmd)

    if cfg['vlog95']:
        log_results(it_key, "iverilog-vlog95", ivl_res)
    else:
        log_results(it_key, "iverilog", ivl_res)

    ivl_rtn = build_ivl_return(translation_fail, ivl_res)
    if ivl_rtn:
        return ivl_rtn

    # run the vvp command
    vvp_cmd = assemble_vvp_cmd(options, cfg)
    vvp_res = run_cmd(vvp_cmd)
    log_results(it_key, "vvp", vvp_res)

    vvp_rtn = build_vvp_return(expected_fail, vvp_res)
    if vvp_rtn:
        return vvp_rtn

    log_list = ["iverilog-stdout", "iverilog-stderr",
                "vvp-stdout", "vvp-stderr"]
    if cfg['vlog95']:
        log_list[2:2] = ["iverilog-vlog95-stdout", "iverilog-vlog95-stderr"]

    return check_run_outputs(options, vvp_res.stdout.decode('ascii'), log_list, expected_fail)

def run_normal(options: dict, cfg: dict) -> list:
    '''Run a normal test'''
    return do_run_normal(options, cfg, False, False)


# pylint: disable-next=invalid-name
def run_EF(options: dict, cfg: dict) -> list:
    '''Run an expected fail test'''
    return do_run_normal(options, cfg, True, False)


# pylint: disable-next=invalid-name
def run_TE(options: dict, cfg: dict) -> list:
    '''Run a translation fail test'''
    return do_run_normal(options, cfg, False, True)
