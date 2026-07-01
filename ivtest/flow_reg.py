''' Regression runner for the flow (-tflow) dataflow exporter target.

    For each test named in flow.list, this compiles flow/<name>.v with
    `iverilog -tflow` and validates the resulting flowtracer1.verilog.v0
    JSON: every output must be well-formed and carry the expected schema,
    a top, and a non-empty module/hierarchy set; per-test checks then
    assert the dataflow facts the exporter is meant to capture (clocked
    processes, port-map continuity, continuous-assign cone reads, and
    generate frames).

    Run with: python3 flow_reg.py '''

import os
import re
import json
import subprocess

# Name of the iverilog command. May vary in different installations.
IVERILOG = "iverilog"


def get_tests() -> list:
    '''Read the test names from flow.list (first word of each line).'''
    match_prog = re.compile(r"^([a-zA-Z0-9_.]+).*$")
    tests = []
    with open("flow.list", encoding='ascii') as fd:
        for line in fd:
            if line[0] == "#":
                continue
            match = match_prog.search(line)
            if match:
                tests.append(match.group(1))
    return tests


# ---- per-test structural checks ----------------------------------------
# Each returns (ok, message). `d` is the parsed .flow document.

def _module(d, name):
    for m in d["modules"]:
        if m["name"] == name:
            return m
    return None


def _find_in_hier(d, pred):
    '''Depth-first search of hierarchy[] for a node matching pred.'''
    stack = list(d["hierarchy"])
    while stack:
        n = stack.pop()
        if pred(n):
            return n
        stack.extend(n.get("children", []))
    return None


def check_basic(d):
    if d["top"] != "top":
        return False, "top != 'top'"
    cnt = _module(d, "counter")
    if not cnt:
        return False, "no 'counter' module"
    proc = [p for p in cnt["processes"]
            if "count" in p["drives"] and p.get("sensitivity")]
    if not proc:
        return False, "no clocked process driving count"
    # port_map continuity: u_cnt.count maps to parent net q
    node = _find_in_hier(d, lambda n: n.get("module") == "counter")
    if not node:
        return False, "counter instance not in hierarchy"
    pm = {a["formal"]: a["actuals"] for a in node["port_map"]}
    if "q" not in pm.get("count", []):
        return False, "port_map count->q missing"
    return True, "clocked process + port_map ok"


def check_comb(d):
    if d["top"] != "chip":
        return False, "top != 'chip'"
    dp = _module(d, "datapath")
    if not dp:
        return False, "no 'datapath' module"
    res = [a for a in dp["assignments"]
           if a["target"] == "result" and "alu_out" in a["reads"]]
    if not res:
        return False, "assign result<-alu_out (cone) missing"
    return True, "continuous-assign cone read ok"


def check_generate(d):
    frame = _find_in_hier(d, lambda n: n.get("scope") is True
                          and "generate" in n)
    if not frame:
        return False, "no generate frame in hierarchy"
    kind = frame["generate"].get("kind")
    if kind not in ("for", "if"):
        return False, "generate.kind not for/if"
    return True, "generate frame ({}) ok".format(kind)


def check_genpath(d):
    # if-generate with whole-signal connection: the instance under the
    # frame must resolve its actuals to the parent nets.
    inst = _find_in_hier(d, lambda n: n.get("module") == "inv8")
    if not inst:
        return False, "inv8 instance not found under frame"
    pm = {a["formal"]: a["actuals"] for a in inst["port_map"]}
    if "din" not in pm.get("d", []) or "dout" not in pm.get("q", []):
        return False, "port_map through frame not resolved"
    return True, "generate-frame port_map ok"


CHECKS = {
    "flow_basic": check_basic,
    "flow_comb": check_comb,
    "flow_generate": check_generate,
    "flow_genpath": check_genpath,
}


def run_test(test_name: str) -> bool:
    '''Compile one test with -tflow and validate its JSON.'''
    src = "flow/" + test_name + ".v"
    out = "flow/" + test_name + ".flow"

    cmd = IVERILOG + " -g2012 -tflow -o" + out + " " + src
    rc = subprocess.call(cmd + " > log/" + test_name + ".log 2>&1", shell=True)
    if rc != 0:
        return False

    try:
        with open(out, encoding='ascii') as fd:
            doc = json.load(fd)
    except (OSError, ValueError):
        return False

    # Invariants common to every well-formed output.
    if doc.get("schema") != "flowtracer1.verilog.v0":
        return False
    if not doc.get("top") or not doc.get("modules") or not doc.get("hierarchy"):
        return False
    # Every position is the compact "start:begin:end" byte-offset string.
    if not _positions_well_formed(doc):
        return False

    check = CHECKS.get(test_name)
    if check:
        ok, _msg = check(doc)
        return ok
    return True


_POS_RE = re.compile(r"^[0-9]*:[0-9]*:[0-9]*$")


def _positions_well_formed(node) -> bool:
    '''Every "pos"/"inst_pos" value, anywhere in the document, must be the
       compact "start:begin:end" byte-offset string (parts may be empty).'''
    if isinstance(node, dict):
        for key, val in node.items():
            if key in ("pos", "inst_pos"):
                if not (isinstance(val, str) and _POS_RE.match(val)):
                    return False
            elif not _positions_well_formed(val):
                return False
        return True
    if isinstance(node, list):
        return all(_positions_well_formed(x) for x in node)
    return True


def get_ivl_version() -> list:
    '''Get the iverilog version.'''
    text = subprocess.check_output([IVERILOG, "-V"])
    match = re.search(b'Icarus Verilog version ([0-9]+)\\.([0-9]+)', text)
    if not match:
        return None
    items = match.groups()
    return [str(items[0], 'ascii'), str(items[1], 'ascii')]


def run_tests(tests: list):
    '''Run all tests in the list.'''
    ivl_ver = get_ivl_version()
    print("Running flow tests for Icarus Verilog version: {}.{}".format(
        ivl_ver[0], ivl_ver[1]))
    print("=" * 50)

    if not os.path.exists("log"):
        os.mkdir("log")

    count_passed = 0
    count_failed = 0
    width = max(len(name) for name in tests)
    for test in tests:
        passed = run_test(test)
        if passed:
            count_passed += 1
            res = "Passed"
        else:
            count_failed += 1
            res = "Failed"
        print("{name:>{width}}: {res}.".format(name=test, width=width, res=res))

    print("=" * 50)
    print("Tests results: Passed {}, Failed {}".format(count_passed, count_failed))


if __name__ == "__main__":
    run_tests(get_tests())
