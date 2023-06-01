
'''Functions for processing test list files

The read_lists() function is the main export of this package. This function
takes a list of file names, reads all the test items in the files, and
puts the result into a sorted list.

The tests list file is formatted like so:

<key> <type> <directory> <other>

The <key> is the name of the test. This is used to generate the source file
name for the test program.

The <directory> is the name of a subdirectory were we search for the test.
So for example, if <key>==foo and <directory>==bar, then the Verilog source
file will be inferred to be bar/foo.v.

The <type> is the test type.

The <other> field sets up how the tests will be checked. Things like gold
files and working directories are given here.
'''

def read_list(fd) -> list:
    '''Return a list of test items (each in list form) from the file.

    The input fd is the file opened in text mode. This function will read
    the file, a line at a time, and make a list of lists, with each list
    in the list a list of tokens for the line. This is used by the read_lists
    function.'''

    build_list = list()
    for line_raw in fd:
        # Strip comments and leading/traling white space
        idx = line_raw.find("#")
        if idx < 0:
            idx = len(line_raw)
        line = line_raw[0:idx].strip()

        # Split into tokens
        line_list = line.split()
        if len(line_list) == 0:
            continue

        build_list.append(line_list)

    return build_list


def read_lists(paths: list) -> list:
    '''Read the paths in the list, and return the list of tests.

    The input is a list of list file names, and the result is a list
    of all the tests, sorted, and with duplicates resolved. The order
    of the test file lists is important, as is the order of tests
    within each list file.'''

    tests_list = list()
    for path in paths:
        with open(path, "r") as fd:
            tests_list += read_list(fd)

    # The loop above creates a tests_list to list all of the tests in the
    # order that they appear. Now we go though the list in order and eliminate
    # duplictes by test name. This allows that lists might override tests that
    # are already declared.
    tests_dict = dict()
    for item in tests_list:
        tests_dict[item[0]] = item;

    # Convert the result to a sorted list, and return that.
    tests_list = list(tests_dict.values())
    tests_list.sort()

    return tests_list
