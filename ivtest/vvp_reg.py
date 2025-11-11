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
import json
import argparse
import test_lists
import run_ivl


def process_test(suffix: str, item: list) -> str:
    '''Process a single test

    This takes in the list of tokens from the tests list file, and converts
    them (interprets them) to a collection of values.'''

    # This is the name of the test, and the name of the main sorce file
    it_key = item[0]
    test_path = item[1]
    with open(test_path, 'rt', encoding='ascii') as fd:
        it_dict = json.load(fd)

    # Get the test type from the json configuration.
    it_type = it_dict['type']

    # Wrap all of this into an options dictionary for ease of handling.
    it_options = {
        'suffix'            : suffix,
        'key'               : it_key,
        'type'              : it_type,
        'iverilog_args'     : it_dict.get('iverilog-args', [ ]),
        'directory'         : "ivltests",
        'source'            : it_dict['source'],
        'modulename'        : None,
        'gold'              : it_dict.get('gold', None),
        'diff'              : None,
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
        # pylint: disable-next=consider-using-f-string
        res = "{key}: I don't understand the test type ({type}).".format(key=it_key, type=it_type)
        raise Exception(res)

    return res


if __name__ == "__main__":
    argp = argparse.ArgumentParser(description='')
    argp.add_argument('--suffix', type=str, default='',
                      help='The Icarus executable suffix, default "%(default)s".')
    argp.add_argument('--strict', action='store_true',
                      help='Force strict standard compliance, default "%(default)s".')
    argp.add_argument('--with-valgrind', action='store_true',
                      help='Run the test suite with valgrind, default "%(default)s".')
    argp.add_argument('--force-sv', action='store_true',
                      help='Force tests to be run as SystemVerilog, default "%(default)s".')
    argp.add_argument('--vlog95', action='store_true',
                      help='Convert tests to Verilog 95 and then run, default "%(default)s".')
    argp.add_argument('files', nargs='*', type=str, default=['regress-vvp.list'],
                      help='File(s) containing a list of the tests to run, default "%(default)s".')
    args = argp.parse_args()

    if args.strict:
        print('Sorry: Forcing strict compatiability is not currently supported!')
        sys.exit(1)
    if args.with_valgrind:
        print('Sorry: Running with valgrind is not currently supported!')
        sys.exit(1)
    if args.force_sv:
        print('Sorry: Forcing SystemVerilog is not currently supported!')
        sys.exit(1)
    if args.vlog95:
        print('Sorry: Converting to Verilog-95 and running is not currently supported!')
        sys.exit(1)

    # This returns 13 or similar
    ivl_version = run_ivl.get_ivl_version(args.suffix)

    print("Running compiler/VVP tests for Icarus Verilog ", end='')
    # pylint: disable-next=consider-using-f-string
    print("version: {ver}".format(ver=ivl_version), end='')
    if args.suffix:
        # pylint: disable-next=consider-using-f-string
        print(", suffix: {suffix}".format(suffix=args.suffix), end='')
    # FIXME: add strict, force SV and with valgrind
    print("")
    # pylint: disable-next=consider-using-f-string
    print("Using list(s): {files}".format(files=', '.join(args.files)))
    print("-" * 76)

    # Read the list files, to get the tests.
    tests_list = test_lists.read_lists(args.files)

    # We need the width of the widest key so that we can figure out
    # how to align the key:result columns.
    # pylint: disable-next=invalid-name
    width = 0
    for cur in tests_list:
        if len(cur[0]) > width:
            width = len(cur[0])

    # pylint: disable-next=invalid-name
    error_count = 0
    for cur in tests_list:
        result = process_test(args.suffix, cur)
        error_count += result[0]
        # pylint: disable-next=consider-using-f-string
        print("{name:>{width}}: {result}".format(name=cur[0], width=width, result=result[1]))

    print("=" * 76)
    # pylint: disable-next=consider-using-f-string
    print("Test results: Ran {ran}, Failed {failed}.".format(ran=len(tests_list), \
                                                             failed=error_count))
    sys.exit(error_count)
