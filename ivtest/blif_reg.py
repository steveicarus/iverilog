''' This is a python script for testing the blif code generator with
    programs specifically set aside for it. The general pattern is that
    the test program comes in two parts: the test bench and the device
    to be tested. The files blif/*_tb.v are the test benches for the
    corresponding files blif/*.v.

    This script requires the "abc" command available here:
       <http://www.eecs.berkeley.edu/~alanmi/abc/>

    Run this script with the command: python3 blif_reg.py '''

import os
import subprocess
import re

# This is the name of the iverilog, vvp, and abc command. These may
# vary in different installations.
IVERILOG = "iverilog"
VVP = "vvp"
ABC = "berkeley-abc"

def get_tests() -> list:
    '''Get the test names'''
    # The list file contains a list of test names. The first word in the
    # line is the name of the test.
    match_prog = re.compile(r"^([a-zA-Z0-9_.]+).*$")

    tests = []
    with open("blif.list", encoding='ascii') as fd:
        for line in fd:
            if line[0] == "#":
                continue
            match = match_prog.search(line)
            if match:
                tests.append(match.group(1))
    return tests

def run_test(test_name: str) -> bool:
    '''Run each test using this routine.
       Returns True if the test passes.'''
    # Assemble the paths for the test-bench and DUT.
    dut = "blif/" + test_name + ".v"
    tb  = "blif/" + test_name + "_tb.v"

    redirect = "log/" + test_name + ".log 2>&1"

    # Process the DUT into a .blif file
    ivl_blif_cmd = IVERILOG + " -g2009 -tblif -otmp_blif.blif " + dut + " > " + redirect
    rc = subprocess.call(ivl_blif_cmd, shell=True)

    if rc == 0:
        # Use ABC to convert the .blif file to Verilog
        abc_cmd = ABC +" -c 'read_blif tmp_blif.blif ; write_verilog tmp_blif.v' >> " + redirect
        rc = subprocess.call(abc_cmd, shell=True)

    if rc == 0:
        # Compile
        ivl_blif_tb_cmd = IVERILOG + " -g2009 -otmp_blif.vvp " + tb + " tmp_blif.v >> " + redirect
        rc = subprocess.call(ivl_blif_tb_cmd, shell=True)

    if rc == 0:
        # Now simulate to make sure the tranlation worked properly.
        vvp_cmd = VVP + " tmp_blif.vvp"
        output = subprocess.check_output(vvp_cmd, shell=True, text=True)
        echo_cmd = "echo " + output.rstrip() + " >> " + redirect
        rc = subprocess.call(echo_cmd, shell=True)

    for tmp in ["tmp_blif.blif", "tmp_blif.v", "tmp_blif.vvp", "abc.history"]:
        if os.path.exists(tmp):
            os.remove(tmp)

    return not (rc or output != "PASSED\n")

def get_ivl_version() -> list:
    '''Get the iverilog version'''
    text = subprocess.check_output([IVERILOG, "-V"])
    match = re.search(b'Icarus Verilog version ([0-9]+)\\.([0-9]+)', text)
    if not match:
        return None

    items = match.groups()
    return [str(items[0], 'ascii'), str(items[1], 'ascii')]

def get_abc_version() -> list:
    '''Get the berkeley-abc version'''
    res = subprocess.run([ABC, "-h"], capture_output=True, check=False)
    match = re.search(b'UC Berkeley, ABC ([0-9]+)\\.([0-9]+)', res.stdout)
    if not match:
        return None

    items = match.groups()
    return [str(items[0], 'ascii'), str(items[1], 'ascii')]

def run_tests(tests: list):
    '''Run all the tests in the test list'''
    ivl_ver = get_ivl_version()
    abc_ver = get_abc_version()
    # pylint: disable-next=consider-using-f-string
    print("Running blif tests for Icarus Verilog version: {ver}.{sver}".format(ver=ivl_ver[0],
                                                                               sver=ivl_ver[1]))
    # pylint: disable-next=consider-using-f-string
    print("Using berkely-abc version: {ver}.{sver}".format(ver=abc_ver[0], sver=abc_ver[1]))
    print("=" * 50)

    count_passed = 0
    count_failed = 0

    width = max(len(name) for name in tests)

    for test in tests:
        passed = run_test(test)
        if passed:
            res = "Passed"
            count_passed += 1
        else:
            res = "Failed"
            count_failed += 1
        # pylint: disable-next=consider-using-f-string
        print("{name:>{width}}: {res}.".format(name=test, width=width, res=res))

    print("=" * 50)
    # pylint: disable-next=consider-using-f-string
    print("Tests results: Passed {passed}, Failed {failed}".format(passed=count_passed, \
                                                                   failed=count_failed))

if __name__ == "__main__":
    run_tests(get_tests())
