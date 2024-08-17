#!/usr/bin/env perl
#
# Script to handle regression for normal Verilog files.
#
# This script is based on code with the following Copyright.
#
# Copyright (c) 1999-2024 Guy Hutchison (ghutchis@pacbell.net)
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
&open_report_file;
my ($suffix, $strict, $with_valg, $force_sv) = &get_args;
my $ver = &get_ivl_version($suffix);
my $opt = $force_sv ? " (force SV)" : "";
my $msg = $with_valg ? " (with valgrind)" : "";
&print_rpt("Running vlog95 compiler/VVP tests for Icarus Verilog " .
           "version: $ver$opt$msg.\n");
&print_rpt("-" x 76 . "\n");
if ($#ARGV != -1) {
    my $regress_fn = &get_regress_fn;
    &read_regression_list($regress_fn, $ver, $force_sv, "");

} else {
    if ($force_sv) {
        &read_regression_list("regress-fsv.list", $ver, $force_sv, "");
    }
    &read_regression_list("regress-vlog95.list", $ver, $force_sv, "");
    &read_regression_list("regress-ivl1.list",   $ver, $force_sv, "");
    &read_regression_list("regress-vlg.list",    $ver, $force_sv, "");
    &read_regression_list("regress-sv.list",     $ver, $force_sv, "");
    &read_regression_list("regress-vhdl.list",   $ver, $force_sv, "");
    &read_regression_list("regress-synth.list",  $ver, $force_sv, "-S");
}
my $failed = &execute_regression($suffix, $with_valg);
&close_report_file;

exit $failed;

#
#  execute_regression sequentially compiles and executes each test in
#  the regression. It then checks that the output matches the gold file.
#
sub execute_regression {
    my $sfx = shift(@_);
    my $with_valg = shift(@_);
    my ($tname, $total, $passed, $failed, $expected_fail, $not_impl,
        $len, $cmd, $diff_file);

    $total = 0;
    $passed = 0;
    $failed = 0;
    $expected_fail = 0;
    $not_impl = 0;
    $len = 0;

    foreach $tname (@testlist) {
        $len = length($tname) if (length($tname) > $len);
    }

    # Make sure we have a log and work directory.
    if (! -d 'log') {
        mkdir 'log' or die "Error: unable to create log directory.\n";
    }
    if (! -d 'work') {
        mkdir 'work' or die "Error: unable to create work directory.\n";
    }

    foreach $tname (@testlist) {
        my $pass_type;
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

        if (! -e "./$srcpath{$tname}/$tname.v") {
            &print_rpt("Failed - missing source file.\n");
            $failed++;
            next;
        }

        #
        # Build up the iverilog command line and run it.
        #
        $pass_type = 0;
        $cmd = $with_valg ? "valgrind --trace-children=yes " : "";
        $cmd .= "iverilog$sfx -o vlog95.v";
        $cmd .= " -s $testmod{$tname}" if ($testmod{$tname} ne "");
        $cmd .= $testtype{$tname} eq "CN" ? " -t null" : " -t vlog95";
        $cmd .= " -pfileline=1 -pspacing=4" if ($testtype{$tname} ne "CN");
        $cmd .= " -D__ICARUS_UNSIZED__ $args{$tname}";
        $cmd .= " ./$srcpath{$tname}/$tname.v > log/$tname.log 2>&1";
#        print "$cmd\n";
        if (system("$cmd")) {
            if ($testtype{$tname} eq "CE") {
                # Check if the system command core dumped!
                if ($? >> 8 & 128) {
                    &print_rpt("==> Failed - CE (core dump).\n");
                    $failed++;
                    next;
                } else {
                    $pass_type = 1;
                }
            } else {
                &print_rpt("==> Failed - running iverilog.\n");
                $failed++;
                next;
            }
        } else {
            if ($testtype{$tname} eq "CE") {
                &print_rpt("==> Failed - CE (no error reported).\n");
                $failed++;
                next;
            }
        }

        if ($testtype{$tname} eq "CO") {
            &print_rpt("Passed - CO.\n");
            $passed++;
            next;
        }
        if ($testtype{$tname} eq "CN") {
            &print_rpt("Passed - CN.\n");
            $passed++;
            next;
        }

        # Run the translated Verilog code. All compile errors should
        # already be handled. Remove the -S flag if it exists along
        # with any included source file(s) and any -f arguments. The
	# -pallowsigned flag and the various generation flags should
	# also be removed. If we had -pallowsigned=1 then use the
	# -g2001-noconfig to get signed/unsigned otherwise use -g1995.
        my $gen_flag;
	if($args{$tname} =~ m/-pallowsigned=1/) {
	    $gen_flag = "-g2001-noconfig";
	} else {
	    $gen_flag = "-g1995";
	}
        $args{$tname} =~ s/-S//;
        $args{$tname} =~ s/\S+\.vhd//g;
        $args{$tname} =~ s/\S+\.v//g;
        $args{$tname} =~ s/-f\S+//g;
        $args{$tname} =~ s/-pallowsigned=1//g;
        $args{$tname} =~ s/-g2001(-noconfig)?//g;
        $args{$tname} =~ s/-g2005(-sv)?//g;
        $args{$tname} =~ s/-g2009//g;
        $args{$tname} =~ s/-g2012//g;
        $args{$tname} =~ s/-gverilog-ams//g;
        $cmd = "iverilog$sfx -o vsim $gen_flag $args{$tname}";
        $cmd .= " -s $testmod{$tname}" if ($testmod{$tname} ne "");
        $cmd .= " vlog95.v >> log/$tname.log 2>&1";
#        print "$cmd\n";
        if ($pass_type == 0 and system("$cmd")) {
            if ($testtype{$tname} eq "TE") {
                # Check if the system command core dumped!
                if ($? >> 8 & 128) {
                    &print_rpt("==> Failed - TE (core dump).\n");
                    $failed++;
                    next;
                } else {
                    $pass_type = 3;
                }
            } else {
                &print_rpt("==> Failed - running iverilog (translated).\n");
                $failed++;
                next;
            }
        }

        $cmd = "vvp$sfx vsim $plargs{$tname} >> log/$tname.log 2>&1";
#        print "$cmd\n";
        if ($pass_type == 0 and system("$cmd")) {
            if ($testtype{$tname} eq "RE") {
                # Check if the system command core dumped!
                if ($? >> 8 & 128) {
                    &print_rpt("==> Failed - RE (core dump).\n");
                    $failed++;
                    next;
                } else {
                    $pass_type = 2;
                }
            } else {
                &print_rpt("==> Failed - running vvp.\n");
                $failed++;
                next;
            }
        }

        if ($diff{$tname} ne "") {
            $diff_file = $diff{$tname}
        } else {
            if ($pass_type == 1) {
                &print_rpt("Passed - CE.\n");
                $passed++;
                next;
            } elsif ($pass_type == 2) {
                &print_rpt("Passed - RE.\n");
                $passed++;
                next;
            } elsif ($pass_type == 3) {
                &print_rpt("Passed - TE.\n");
                $passed++;
                next;
            }
            $diff_file = "log/$tname.log";
        }
#        print "diff $gold{$tname}, $diff_file, $offset{$tname}, $unordered{$tname}\n";
        if (diff($gold{$tname}, $diff_file, $offset{$tname}, $unordered{$tname})) {
            if ($testtype{$tname} eq "EF") {
                &print_rpt("Passed - expected fail.\n");
                $expected_fail++;
                next;
            }
            &print_rpt("==> Failed -");
            if ($pass_type == 1) {
                &print_rpt(" CE -");
            } elsif ($pass_type == 2) {
                &print_rpt(" RE -");
            } elsif ($pass_type == 3) {
                &print_rpt(" TE -");
            }
            &print_rpt(" output does not match gold file.\n");
            $failed++;
            next;
        }

        if ($pass_type == 1) {
            &print_rpt("Passed - CE.\n");
        } elsif ($pass_type == 2) {
            &print_rpt("Passed - RE.\n");
        } elsif ($pass_type == 3) {
            &print_rpt("Passed - TE.\n");
        } else {
            &print_rpt("Passed.\n");
        }
        $passed++;

    } continue {
        if ($tname ne "") {
            system("rm -f ./vlog95.v ./vsim") and
                die "Error: failed to remove temporary file.\n";
        }
    }

    &print_rpt("=" x 76 . "\n");
    &print_rpt("Test results:\n  Total=$total, Passed=$passed, Failed=$failed,".
               " Not Implemented=$not_impl, Expected Fail=$expected_fail\n");

    return $failed;
}
