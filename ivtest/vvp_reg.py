#! python3
'''
Usage:
    vvp_reg
    vvp_reg <list-paths>...

<list-paths> is a list of files in the current working directory that
            each contain a list of tests. By convention, the file has the
            suffix ".list". The files will be processed in order, so tests
            can be overridden if listed twice. If no files are given, a
            default list is used.
'''

import sys
# The docopt library is not available on msys2 or cygwin installations, so
# skip it completely on win32/cygwin platforms.
if sys.platform != 'win32' and sys.platform != 'cygwin':
    from docopt import docopt
import test_lists
import json
import run_ivl


def process_test(item: list) -> str:
    '''Process a single test

    This takes in the list of tokens from the tests list file, and converts
    them (interprets them) to a collection of values.'''

    # This is the name of the test, and the name of the main sorce file
    it_key = item[0]
    test_path = item[1]
    with open(test_path, 'rt') as fd:
        it_dict = json.load(fd)

    # Get the test type from the json configuration.
    it_type = it_dict['type']

    # Wrap all of this into an options dictionary for ease of handling.
    it_options = {
        'key'           : it_key,
        'type'          : it_type,
        'iverilog_args' : it_dict.get('iverilog-args', [ ]),
        'directory'     : "ivltests",
        'source'        : it_dict['source'],
        'modulename'    : None,
        'gold'          : it_dict.get('gold', None),
        'diff'          : None,
        'vvp_args'          : it_dict.get('vvp-args', [ ]),
        'vvp_args_extended' : it_dict.get('vvp-args-extended', [ ])
    }

    if it_type == "NI":
        res = [0, "Not Implemented."]

    elif it_type == "normal":
        res = run_ivl.run_normal(it_options)

    elif it_type == "normal-vlog95":
        res = run_ivl.run_normal_vlog95(it_options)

    elif it_type == "CE":
        res = run_ivl.run_CE(it_options)

    elif it_type == "EF":
        res = run_ivl.run_EF(it_options)

    elif it_type == "EF-vlog95":
        res = run_ivl.run_EF_vlog95(it_options)

    else:
        res = "{key}: I don't understand the test type ({type}).".format(key=it_key, type=it_type)
        raise Exception(res)

    return res


if __name__ == "__main__":
    print("Running tests on platform: {platform}".format(platform=sys.platform))
    if sys.platform == 'win32' or sys.platform == 'cygwin':
        args = { "<list-paths>" : [] }
    else:
        args = docopt(__doc__)

    # This returns [13, 0] or similar
    ivl_version = run_ivl.get_ivl_version()
    ivl_version_major = ivl_version[0]
    print("Icarus Verilog version: {ivl_version}".format(ivl_version=ivl_version_major))

    # Select the lists to use. If any list paths are given on the command
    # line, then use only those. Otherwise, use a default list.
    list_paths = args["<list-paths>"]
    if len(list_paths) == 0:
        list_paths = list()
        list_paths += ["regress-vvp.list"]

    print("Use lists: {list_paths}".format(list_paths=list_paths))

    # Read the list files, to get the tests.
    tests_list = test_lists.read_lists(list_paths)

    # We need the width of the widest key so that we can figure out
    # how to align the key:result columns.
    width = 0
    for cur in tests_list:
        if len(cur[0]) > width:
            width = len(cur[0])

    error_count = 0
    for cur in tests_list:
        result = process_test(cur)
        error_count += result[0]
        print("{name:>{width}}: {result}".format(name=cur[0], width=width, result=result[1]))

    print("===================================================")
    print("Test results: Ran {ran}, Failed {failed}.".format(ran=len(tests_list), failed=error_count))
    exit(error_count)
