#
# Module for comparing expected and actual output of tests.
# It knows how to skip valgrind output ^==\d+== or ^**\d+**
#

package Diff;

use strict;
use warnings;

our $VERSION = '1.00';

use base 'Exporter';

our @EXPORT = qw(diff);

#
# We only need a simple diff, but we need to strip \r at the end of line
# and we need to ignore the valgrind output.
#
sub diff {
    my ($gold, $log, $skip, $unordered) = @_;
    my ($diff, $gline, $lline);
    $diff = 0;

    #
    # If we do not have a gold file then we just look for a log file line
    # with just PASSED on it to indicate that the test worked correctly.
    #
    if ($gold eq "") {
        open (LOG, "<$log") or do {
            warn "Error: unable to open $log for reading.\n";
            return 1;
        };

        $diff = 1;
        # Loop on the log file lines looking for a "passed" by it self.
        # For VHDL tests we need to ignore time, filename, severity, etc.
        # that GHDL prints for report statments
        foreach $lline (<LOG>) {
            if ($lline =~ /^\s*passed\s*$/i) {
                $diff = 0;
            }
            elsif ($lline =~ /@\d+\w+:\(report note\): passed\s*$/i) {
                $diff = 0;
            }
        }

        close (LOG);
    } else {
        open (GOLD, "<$gold") or do {
            warn "Error: unable to open $gold for reading.\n";
            return 1;
        };
        open (LOG, "<$log") or do {
            warn "Error: unable to open $log for reading.\n";
            return 1;
        };

        if ($unordered) {
            my @glines = sort map { s/\r\n$/\n/; $_ } <GOLD>;
            my @llines = sort map { s/\r\n$/\n/; $_ } <LOG>;

            my $gindex = 0;
            my $lindex = 0;
            while ($gindex < @glines) {
                # Skip lines from valgrind ^==\d+== or ^**\d+**
                while ($lindex < @llines && $llines[$lindex] =~ m/^(==|\*\*)\d+(==|\*\*)/) {
                    $lindex++
                }
                if ($lindex == @llines) {
                    $diff = 1;
                    last;
                }
                # Skip initial lines if needed.
                if ($skip > 0) {
                    $lindex++;
                    $skip--;
                    next;
                }
                if ($glines[$gindex] ne $llines[$lindex]) {
                    $diff = 1;
                    last;
                }
                $gindex++;
                $lindex++;
            }

            # Check to see if the log file has extra lines.
            while ($lindex < @llines && $llines[$lindex] =~ m/^(==|\*\*)\d+(==|\*\*)/) {
                $lindex++
            }
            $diff = 1 if $lindex < @llines;
        } else {
            # Loop on the gold file lines.
            foreach $gline (<GOLD>) {
                if (eof LOG) {
                    $diff = 1;
                    last;
                }
                $lline = <LOG>;
                # Skip lines from valgrind ^==\d+== or ^**\d+**
                while ($lline =~ m/^(==|\*\*)\d+(==|\*\*)/) {
                    $lline = <LOG>;
                }
                # Skip initial lines if needed.
                if ($skip > 0) {
                    $skip--;
                    next;
                }
                $gline =~ s/\r\n$/\n/;  # Strip <CR> at the end of line.
                $lline =~ s/\r\n$/\n/;  # Strip <CR> at the end of line.
                if ($gline ne $lline) {
                    $diff = 1;
                    last;
                }
            }

            # Check to see if the log file has extra lines.
            while (!eof LOG and !$diff) {
                $lline = <LOG>;
                $diff = 1 if ($lline !~ m/^(==|\*\*)\d+(==|\*\*)/);
            }
        }
        close (LOG);
        close (GOLD);
    }

    return $diff;
}

1;  # Module loaded OK
