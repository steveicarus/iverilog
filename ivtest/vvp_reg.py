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


class InvalidTestType(Exception):
    '''Exception to raise when the test type is not supported.'''
    def __init__(self, test, ttype, msg='Invalid test type!'):
        self.test = test
        self.ttype = ttype
        self.msg = msg
        super().__init__(self.msg)

    def __str__(self):
        # pylint: disable-next=consider-using-f-string
        return "Given test type '{ttype}' for test {test}".format(ttype=self.ttype,test=self.test)

class InvalidJSON(Exception):
    '''Exception to raise if the JSON is not parsed properly.'''
    def __init__(self, test, path, msg='Invalid JSON file!'):
        self.test = test
        self.path = path
        self.msg = msg
        super().__init__(self.msg)

    def __str__(self):
        # pylint: disable-next=consider-using-f-string
        res = "Unable to parse JSON file '{path}' for test {test}:".format(path=self.path,
                                                                           test=self.test)
        # pylint: disable-next=consider-using-f-string
        res += "\n    {msg}".format(msg=self.msg)
        return res


def process_overrides(group: str, it_dict: dict, it_opts: dict):
    '''Override the gold file or type if needed.'''
    if group in it_dict:
        overrides = ['gold', 'type']
        for override in overrides:
            if override in it_dict[group]:
                if override == 'gold' and it_dict[group][override] == "":
                    it_opts[override] = None
                else:
                    it_opts[override] = it_dict[group][override]


def force_gen(it_opts: dict):
    '''Remove the current generation and force it to the latest.'''
    generations = ['-g2012', '-g2009', '-g2005-sv',
                   '-g2005', '-g2001-noconfig', '-g2001', '-g1995',
                   '-g2', '-g1']
    for gen in generations:
        if gen in it_opts['iverilog_args']:
            it_opts['iverilog_args'].remove(gen)

    if '-g2x' in it_opts['iverilog_args']:
        idx_to_replace = it_opts['iverilog_args'].index('-g2x')
        it_opts[idx_to_replace] = '-gicarus-misc'

    it_opts['iverilog_args'].insert(0, '-g2012')


def process_test(item: list, cfg: list) -> str:
    '''Process a single test

    This takes in the list of tokens from the tests list file, and converts
    them (interprets them) to a collection of values.'''

    # This is the name of the test, and the name of the main sorce file
    it_key = item[0]
    test_path = item[1]
    with open(test_path, 'rt', encoding='ascii') as fd:
        try:
            it_dict = json.load(fd)
        except json.decoder.JSONDecodeError as exception:
            raise InvalidJSON(it_key, test_path, exception) from exception

    # Wrap all of this into an options dictionary for ease of handling.
    it_opts = {
        'key'               : it_key,
        'type'              : it_dict['type'],
        'iverilog_args'     : it_dict.get('iverilog-args', [ ]),
        'directory'         : "ivltests",
        'source'            : it_dict['source'],
        'modulename'        : None,
        'gold'              : it_dict.get('gold', None),
        'diff'              : None,
        'vvp_args'          : it_dict.get('vvp-args', [ ]),
        'vvp_args_extended' : it_dict.get('vvp-args-extended', [ ])
    }

    if cfg['strict']:
        it_opts['iverilog_args'].append("-gstrict-expr-width")
        it_opts['vvp_args_extended'].append("-compatible")
        process_overrides('strict', it_dict, it_opts)
    else:
        it_opts['iverilog_args'].append('-D__ICARUS_UNSIZED__')

    if cfg['force-sv']:
        force_gen(it_opts)
        process_overrides('force-sv', it_dict, it_opts)

    # Get the overridden test type.
    it_type = it_opts['type']

    if it_type == "NI":
        res = [0, "Not Implemented."]

    elif it_type == "normal":
        res = run_ivl.run_normal(it_opts, cfg)

    elif it_type == "normal-vlog95":
        res = run_ivl.run_normal_vlog95(it_opts, cfg)

    elif it_type == "CE":
        res = run_ivl.run_CE(it_opts, cfg)

    elif it_type == "EF":
        res = run_ivl.run_EF(it_opts, cfg)

    elif it_type == "EF-vlog95":
        res = run_ivl.run_EF_vlog95(it_opts, cfg)

    else:
        raise InvalidTestType(it_key, it_type)

    return res


def print_header(cfg: dict, files: list):
    '''Print all the header information. '''
    # This returns 13 or similar
    ivl_version = run_ivl.get_ivl_version(cfg['suffix'])

    print("Running compiler/VVP tests for Icarus Verilog ", end='')
    # pylint: disable-next=consider-using-f-string
    print("version: {ver}".format(ver=ivl_version), end='')
    if cfg['suffix']:
        # pylint: disable-next=consider-using-f-string
        print(", suffix: {suffix}".format(suffix=cfg['suffix']), end='')
    if cfg['strict']:
        if cfg['force-sv']:
            print(" (strict, force SV)", end='')
        else:
            print(" (strict)", end='')
    elif cfg['force-sv']:
        print(" (force SV)", end='')
    if cfg['with-valgrind']:
        print(" (valgrind)", end='')
    print("")
    # pylint: disable-next=consider-using-f-string
    print("Using list(s): {files}".format(files=', '.join(files)))
    print("-" * 76)


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

    # FIXME: need to add real vlog95 support
    if args.vlog95:
        print('Sorry: Converting to Verilog-95 and running is not currently supported!')
        sys.exit(1)

    ivl_cfg = {
        'suffix'        : args.suffix,
        'strict'        : args.strict,
        'with-valgrind' : args.with_valgrind,
        'force-sv'      : args.force_sv
              }

    print_header(ivl_cfg, args.files)

    # Read the list files, to get the tests.
    tests_list = test_lists.read_lists(args.files)

    # We need the width of the widest key so that we can figure out
    # how to align the key:result columns.
    # pylint: disable-next=invalid-name
    width = 0
    for cur in tests_list:
        width = max(width, len(cur[0]))

    # pylint: disable-next=invalid-name
    error_count = 0
    for cur in tests_list:
        result = process_test(cur, ivl_cfg)
        error_count += result[0]
        # pylint: disable-next=consider-using-f-string
        print("{name:>{width}}: {result}".format(name=cur[0], width=width, result=result[1]))

    print("=" * 76)
    # pylint: disable-next=consider-using-f-string
    print("Test results: Ran {ran}, Failed {failed}.".format(ran=len(tests_list), \
                                                             failed=error_count))
    sys.exit(error_count)
