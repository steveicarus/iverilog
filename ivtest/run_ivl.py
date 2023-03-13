'''Functions for running Icarus Verilog

'''

import subprocess
import difflib
import os
import sys
import re

def assemble_iverilog_cmd(source: str, it_dir: str, args: list, outfile = "a.out") -> list:
    res = ["iverilog", "-o", os.path.join("work", outfile)]
    res += ["-D__ICARUS_UNSIZED__"]
    res += args
    src = os.path.join(it_dir, source)
    res += [src]
    return res


def assemble_vvp_cmd(args: list = [], plusargs: list = []) -> list:
    res = ["vvp"]
    res = res + args
    res.append(os.path.join("work", "a.out"))
    res = res + plusargs
    return res


def get_ivl_version () -> list:
    '''Figure out the version of the installed iverilog compler.

    The return value is a list of 2 numbers, the major and minor version
    numbers, or None if the version string couldn't be found.'''

    # Get the output from the "iverilog -V" command for the version string.
    text = subprocess.check_output(["iverilog", "-V"])
    match = re.search(b'Icarus Verilog version ([0-9]+)\.([0-9]+)', text)
    if not match:
        return None

    return [int(match[1]), int(match[2])]

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

def log_results(key, title, res) -> None:
    ''' Write results into log files.

    Generate a log file with the name of the key and title, and
    put the stdout and stderr into separate files.'''

    with open(os.path.join("log", f"{key}-{title}-stdout.log"), 'wb') as fd:
        fd.write(res.stdout)

    with open(os.path.join("log", f"{key}-{title}-stderr.log"), 'wb') as fd:
        fd.write(res.stderr)


def compare_files(log_path, gold_path):
    '''Compare the log file and the gold file

    The files are read it, line at a time, and the lines are compared.
    If they differ, then write tou stdout a unified diff. In any case,
    return True or False to indicate the results of the test.'''

    with open(log_path, 'rt') as fd:
        a = fd.readlines()

    # Allow to omit empty gold files
    if os.path.exists(gold_path):
        with open(gold_path, 'rt') as fd:
            b = fd.readlines()
    else:
        b = []

    flag = a == b
    if not flag:
        print(f"{log_path} and {gold_path} differ:")
        sys.stdout.writelines(difflib.unified_diff(a, b, log_path, gold_path))

    return flag


def run_CE(options : dict) -> list:
    ''' Run the compiler, and expect an error

    In this case, we assert that the command fails to run and reports
    an error. This is to check that invalid input generates errors.'''

    it_key = options['key']
    it_dir = options['directory']
    it_args = options['iverilog_args']

    build_runtime(it_key)

    cmd = assemble_iverilog_cmd(options['source'], it_dir, it_args)
    res = subprocess.run(cmd, capture_output=True)
    log_results(it_key, "iverilog", res)

    if res.returncode == 0:
        return [1, "Failed - CE (no error reported)"]
    elif res.returncode >= 256:
        return [1, "Failed - CE (execution error)"]
    else:
        return [0, "Passed - CE"]

def check_run_outputs(options : dict, expected_fail : bool, it_stdout : str, log_list : list) -> list:
    '''Check the output files, and return success for failed.
    
    This function takes an options dictionary that describes the settings, and
    the output from the final command. This also takes a list of log files to check
    there there are gold files present.'''

    # Get the options this step needs...
    it_key = options['key']
    it_gold = options['gold']
    it_diff = options['diff']

    if it_gold is not None:
        compared = True
        for log_name in log_list:
            log_path = os.path.join("log", f"{it_key}-{log_name}.log")
            gold_path = os.path.join("gold", f"{it_gold}-{log_name}.gold")
            compared = compared and compare_files(log_path, gold_path)

        if expected_fail:
            if compared:
                return [1, "Failed = Passed, but expected failure"]
            else:
                return [0, "Passed - Expected fail"]
        else:
            if compared:
                return [0, "Passed"]
            else:
                return [1, "Failed - Gold output doesn't match actual output."]

    # If there is a diff description, then compare named files instead of
    # the log and a gold file.
    if it_diff is not None:
        diff_name1 = it_diff[0]
        diff_name2 = it_diff[1]
        diff_skip = int(it_diff[2])

        with open(diff_name1) as fd:
            for idx in range(diff_skip):
                fd.readline()
            diff_data1 = fd.read()

        with open(diff_name2) as fd:
            for idx in range(diff_skip):
                fd.readline()
            diff_data2 = fd.read()

        if expected_fail:
            if diff_data1 == diff_data2:
                return [1, "Failed - Passed, but expected failure"]
            else:
                return [0, "Passed"]
        else:
            if diff_data1 == diff_data2:
                return [0, "Passed"]
            else:
                return [1, f"Failed - Files {diff_name1} and {diff_name2} differ."]


    # Otherwise, look for the PASSED output string in stdout.
    for line in it_stdout.splitlines():
        if line == "PASSED":
            if expected_fail:
                return [1, "Failed - Passed, but expected failure"]
            else:
                return [0, "Passed"]

    # If there is no PASSED output, and nothing else to check, then
    # assume a failure.
    if expected_fail:
        return [0, "Passed"]
    else:
        return [1, "Failed - No PASSED output, and no gold file"]


def do_run_normal_vlog95(options : dict, expected_fail : bool) -> list:
    '''Run the iverilog and vvp commands.

    In this case, run the compiler with the -tvlog95 flag to generate
    an intermediate verilog file, then run the compiler again to generate
    a vvp out. Run that vvp output to test the simulation results. Collect
    the results and look for a "PASSED" string.'''

    it_key = options['key']
    it_dir = options['directory']
    it_iverilog_args = ["-tvlog95"] + options['iverilog_args']
    it_vvp_args = options['vvp_args']
    it_vvp_args_extended = options['vvp_args_extended']

    build_runtime(it_key)

    # Run the first iverilog command, to generate the intermediate verilog
    ivl1_cmd = assemble_iverilog_cmd(options['source'], it_dir, it_iverilog_args, "a.out.v")
    ivl1_res = subprocess.run(ivl1_cmd, capture_output=True)

    log_results(it_key, "iverilog", ivl1_res)
    if ivl1_res.returncode != 0:
        return [1, "Failed - Compile failed"]

    # Run another iverilog command to compile the code generated from the first step.
    ivl2_cmd = assemble_iverilog_cmd("a.out.v", "work", [ ], "a.out")
    ivl2_res = subprocess.run(ivl2_cmd, capture_output=True)

    log_results(it_key, "iverilog-vlog95", ivl2_res)
    if ivl2_res.returncode != 0:
        return [1, "Failed - Compile of generated code failed"]

    # Run the vvp command
    vvp_cmd = assemble_vvp_cmd(it_vvp_args, it_vvp_args_extended)
    vvp_res = subprocess.run(vvp_cmd, capture_output=True)
    log_results(it_key, "vvp", vvp_res);
        
    if vvp_res.returncode != 0:
        return [1, "Failed - Vvp execution failed"]

    it_stdout = vvp_res.stdout.decode('ascii')
    log_list = ["iverilog-stdout", "iverilog-stderr",
                "iverilog-vlog95-stdout", "iverilog-vlog95-stderr",
                "vvp-stdout", "vvp-stderr"]

    return check_run_outputs(options, expected_fail, it_stdout, log_list)


def do_run_normal(options : dict, expected_fail : bool) -> list:
    '''Run the iverilog and vvp commands.

    In this case, run the compiler to generate a vvp output file, and
    run the vvp command to actually execute the simulation. Collect
    the results and look for a "PASSED" string.'''

    it_key = options['key']
    it_dir = options['directory']
    it_iverilog_args = options['iverilog_args']
    it_vvp_args = options['vvp_args']
    it_vvp_args_extended = options['vvp_args_extended']

    build_runtime(it_key)

    # Run the iverilog command
    ivl_cmd = assemble_iverilog_cmd(options['source'], it_dir, it_iverilog_args)
    ivl_res = subprocess.run(ivl_cmd, capture_output=True)

    log_results(it_key, "iverilog", ivl_res)
    if ivl_res.returncode != 0:
        return [1, "Failed - Compile failed"]

    # run the vvp command
    vvp_cmd = assemble_vvp_cmd(it_vvp_args, it_vvp_args_extended)
    vvp_res = subprocess.run(vvp_cmd, capture_output=True)
    log_results(it_key, "vvp", vvp_res);
        
    if vvp_res.returncode != 0:
        return [1, "Failed - Vvp execution failed"]

    it_stdout = vvp_res.stdout.decode('ascii')
    log_list = ["iverilog-stdout", "iverilog-stderr",
                "vvp-stdout", "vvp-stderr"]

    return check_run_outputs(options, expected_fail, it_stdout, log_list)

def run_normal(options : dict) -> list:
    return do_run_normal(options, False)

def run_EF(options : dict) -> list:
    return do_run_normal(options, True)

def run_normal_vlog95(options : dict) -> list:
    return do_run_normal_vlog95(options, False)

def run_EF_vlog95(options : dict) -> list:
    return do_run_normal_vlog95(options, True)
