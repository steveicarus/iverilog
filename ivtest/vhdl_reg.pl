#!/usr/bin/env perl
#
# Regression script for VHDL output. Based on vvp_reg.pl.
#
# This script is based on code with the following Copyright.
#
# Copyright (c) 1999-2020 Guy Hutchison (ghutchis@pacbell.net)
#
#    This source code is free software; you can redistribute it
#    and/or modify it in source code form under the terms of the GNU
#    General Public License as published by the Free Software
#    Foundation; either version 2 of the License, or (at your option)
#    any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA

use lib './perl-lib';

use RegressionList;
use Diff;
use Reporting;
use Environment;


#
#  Main script
#
&open_report_file('vhdl_regression_report.txt');
my ($suffix, $strict, $with_valg, $force_sv) = &get_args;
my $ver = &get_ivl_version($suffix);
my $msg = $with_valg ? " (with valgrind)" : "";
&print_rpt("Running VHDL tests for Icarus Verilog version: $ver$msg.\n");
&print_rpt("-" x 70 . "\n");
if ($#ARGV != -1) {
    my $regress_fn = &get_regress_fn;
    &read_regression_list($regress_fn, $ver, $force_sv, "");
} else {
    &read_regression_list("vhdl_regress.list", $ver, $force_sv, "");
}
&execute_regression($suffix, $with_valg);
&close_report_file;


#
#  execute_regression sequentially compiles and executes each test in
#  the regression. It then checks that the output matches the gold file.
#
sub execute_regression {
    my $sfx = shift(@_);
    my $with_valg = shift(@_);
    my ($tname, $total, $passed, $failed, $expected_fail, $not_impl,
        $len, $cmd, $diff_file, $outfile, $unit);

    $total = 0;
    $passed = 0;
    $failed = 0;
    $expected_fail = 0;
    $not_impl = 0;
    $len = 0;

    # Check for the VHDL output directory
    mkdir 'vhdl' unless (-d 'vhdl');

    foreach $tname (@testlist) {
        $len = length($tname) if (length($tname) > $len);
    }

    foreach $tname (@testlist) {
        next if ($tname eq "");  # Skip test that have been replaced.

        $total++;
        &print_rpt(sprintf("%${len}s: ", $tname));
        if ($diff{$tname} ne "" and -e $diff{$tname}) {
            unlink $diff{$tname} or
                die "Error: unable to remove old diff file $diff{$tname}.\n";
        }
        if (-e "log/$tname.log") {
            unlink "log/$tname.log" or
                die "Error: unable to remove old log file log/$tname.log.\n";
        }

        if ($testtype{$tname} eq "NI") {
            &print_rpt("Not Implemented.\n");
            $not_impl++;
            next;
        }

        # Store all the output in the vhdl subdirectory for debugging
        $outfile = "vhdl/$tname.vhd";

        #
        # Build up the iverilog command line and run it.
        #
        $cmd = $with_valg ? "valgrind --trace-children=yes " : "";
        $cmd .= "iverilog$sfx -t vhdl -o $outfile $args{$tname}";
        $cmd .= " -s $testmod{$tname}" if ($testmod{$tname} ne "");
        $cmd .= " ./$srcpath{$tname}/$tname.v > log/$tname.log 2>&1";
        #print "$cmd\n";
        if (system("$cmd")) {
            if ($testtype{$tname} eq "CE") {
                # Check if the system command core dumped!
                if ($? >> 8 & 128) {
                    &print_rpt("==> Failed - CE (core dump).\n");
                    $failed++;
                } else {
                    &print_rpt("Passed - CE.\n");
                    $passed++;
                }
                next;
            }

            # Check the log file for an un-translatable construct error
            # We report this separately so we can distinguish between
            # expected and unexpected failures
            $cmd = "grep -q -i -E '(no vhdl translation|cannot be translated)' log/$tname.log";
            if (system($cmd) == 0) {
                &print_rpt("==> Failed - No VHDL translation.\n");
                $not_impl++;
                next;
            }
            else {
                &print_rpt("==> Failed - running iverilog.\n");
                $failed++;
                next;
            }
        }

        #
        # Compile the output with GHDL
        #
        $cmd = "(cd vhdl ; ghdl -a $tname.vhd) >> log/$tname.log 2>&1";
        #print "$cmd\n";
        if (system("$cmd")) {
            if ($testtype{$tname} eq "CE") {
                # Check if the system command core dumped!
                if ($? >> 8 & 128) {
                    &print_rpt("==> Failed - CE (core dump).\n");
                    $failed++;
                } else {
                    &print_rpt("Passed - CE.\n");
                    $passed++;
                }
                next;
            }
            &print_rpt("==> Failed - running ghdl.\n");
            $failed++;
            next;
        }

        # The CO test type now includes compilation of the VHDL
        if ($testtype{$tname} eq "CO") {
            &print_rpt("Passed - CO.\n");
            $passed++;
            next;
        }

        # Try to guess the name of the primary VHDL unit
        # ghdl -f lists all the units in a file: take the first one
        ($unit) = `ghdl -f $outfile` =~ /^entity (\w+)/;
        unless ($unit) {
            &print_rpt("==> Failed -- cannot determine primary VHDL unit.\n");
            $failed++;
            next;
        }
        #print "primary unit is $unit\n";

        # Elaborate the primary unit to produce and executable
        # Could elaborate and run in a single step, but this should
        # provide better error detection.
        $cmd = "(cd vhdl ; ghdl -e $unit) >> log/$tname.log 2>&1";
        #print "$cmd\n";
        if (system($cmd)) {
            &print_rpt("==> Failed - running ghdl elaboration step.\n");
            $failed++;
            next;
        }

        # Finally, run the exectutable
        $cmd = "(cd vhdl ; ghdl -r $unit --stop-delta=10000) >> log/$tname.log 2>&1";
        #print "$cmd\n";
        if (system($cmd)) {
            if ($testtype{$tname} eq "RE") {
                &print_rpt("Passed - RE.\n");
                $passed++;
                next;
            }
            else {
                # If the log contains `SIMULATION FINISHED' then
                # this is OK
                $cmd = "grep -q 'SIMULATION FINISHED' log/$tname.log";
                if (system($cmd)) {
                    &print_rpt("==> Failed - simulating VHDL.\n");
                    $failed++;
                    next;
                }
            }
        }

        if ($diff{$tname} ne "") {
            $diff_file = $diff{$tname}
        } else {
            $diff_file = "log/$tname.log";
        }
#        print "diff $gold{$tname}, $diff_file, $offset{$tname}\n";
        if (diff($gold{$tname}, $diff_file, $offset{$tname})) {
            if ($testtype{$tname} eq "EF") {
                &print_rpt("Passed - expected fail.\n");
                $expected_fail++;
                next;
            }
            &print_rpt("==> Failed - output does not match gold file.\n");
            $failed++;
            next;
        }

        &print_rpt("Passed.\n");
        $passed++;

    } continue {
        if ($tname ne "") {
            # Remove GHDL temporary files
            my $tmpfiles = './vhdl/*.o ./vhdl/work-obj93.cf';
            $tmpfiles .= " ./vhdl/$unit" if $unit;
            system("rm -f $tmpfiles") and
                die "Error: failed to remove temporary files.\n";
        }
    }

    &print_rpt("=" x 70 . "\n");
    &print_rpt("Test results:\n  Total=$total, Passed=$passed, Failed=$failed,".
               " Not Implemented=$not_impl, Expected Fail=$expected_fail\n");
}
