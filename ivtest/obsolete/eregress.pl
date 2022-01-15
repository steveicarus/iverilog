#!/usr/bin/env perl -s
#
# Copyright (c) 1999 Guy Hutchison (stevew@home.com)
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

$total_count = 0;

$num_opts = $#ARGV ;

if($num_opts ne -1) {
   # Got here cuz there is a command line option
   $regress_fn = $ARGV[0];
   if(!( -e "$regress_fn")) {
       print("Error - Command line option file $num_opts doesn't exist.\n");
       exit(1);
   }
} else {
   $regress_fn = "./elist";
}


$logdir = "errlog";
$bindir = "bin";  # not currently used
$report_fn = "./err_regress.txt";

$comp_name = "IVL" ;	# Change the name of the compiler in use here.
                        # this may change to a command line option after
			# I get things debugged!

# Debug variables
$dbg1 = 0;
$dbg2 = 0;

#  Main script

print ("Reading/parsing test list\n");
&read_regression_list;
&execute_regression;
print ("Checking logfiles\n");
&check_results;
print("Testing $testname ********");

#
#  parses the regression list file
#
#  First line
#  splits the data into a list of names (@testlist), and a
#  number of hashes, indexed by name of test.  Hashes are
#  (from left-to-right in regression file):
#
#    %testtype     type of test.  compile = compile only
#                                 normal = compile & run, expect standard
#                                     PASSED/FAILED message at EOT.
#    %testpath     path to test, from root of test directory.  No
#                  trailing slash on test path.
#
#    %testmod = main module declaration (optional)
#
#  Second line
#
#  The error you expect to find.

sub read_regression_list {
    open (REGRESS_LIST, "<$regress_fn");
    local ($found, $testname);

    while (<REGRESS_LIST>) {	# read first line
	chop;
	if (!/^#/) {
	    # strip out any comments later in the file
	    s/#.*//g;
	    @found = split;
        $compare_line = <REGRESS_LIST>; # Read 2nd line
        chop($compare_line);

        # Now spread things out a bit
        $testname = $found[0];
        $testpath{$testname} = $found[1];
        $testmod{$testname} = $found[2];
        $compare{$testname} = $compare_line;
        push (@testlist, $testname);
        if($dbg1 == 1) {
             print $testname,"-",$testpath{$testname},"-",
                   $testmod{$testname},"=",$compare{$testname},"\n";
           }
        }
    }
    close (REGRESS_LIST);
}

#
#  execute_regression sequentially compiles and executes each test in
#  the regression.  Regression is done as a two-pass run (execute, check
#  results) so that at some point the execution part can be parallelized.
#

sub execute_regression {
    local ($testname, $rv);
    local ($bpath, $lpath, $vpath);

    foreach $testname (@testlist) {

        #
        # First lets clean up the "last" test run.
        #
        if(-e "core")  {
              system("rm -f core");
           }
        if(-e "simv")  {
              system("rm -f simv");
           }
		if(-e "simv.exe") {
              system("rm -f simv.exe"); # And we support DOS too!!
           }

        #
        # This is REALLY only an IVL switch...
        #
        # vermod is used to declare the "main module"
        #
        $vermod = "-s ".$testmod{$testname} ;
        $vername = "iverilog ";
        $verout = "-o simv";
        $redir = "&>";

		print "Test $testname:";
		if ($testpath{$testname} eq "") {
			$vpath = "./$testname.v";
		} else {
			$vpath = "./$testpath{$testname}/$testname.v";
		}

		$lpath = "./$logdir/$testname.log";
        system("rm -rf $lpath");
        system("rm -rf *.out");


        #
        # if we have a logfile - remove it first
        #
        if(-e "$lpath") {
           system("rm $lpath");
        }

        #
        # Now build the command up
        #
		$cmd = "$vername $versw $verout $vermod $vpath $redir $lpath ";
		print "$cmd\n";
		system("$cmd");	  # and execute it.


    }

}

sub check_results {
    local ($testname, $rv);
    local ($bpath, $lpath, $vpath);
    local ($pass_count, $fail_count, $crash_count);
    local ($result);

    $pass_count  = 0;
    $no_sorry  = 0;
    $no_parse_err =0;
    $no_run      = 0;
    $crash_count = 0;
    $comperr_cnt = 0;
    $comp_err = 0;

    open (REPORT, ">$report_fn");

    print REPORT "Test Results:\n";

    $num_tests = 0;
    foreach $testname (@testlist) {
	  $lpath = "$logdir/$testname.log";
      $num_tests++;		# count tests

      # Read in the logfile into infile.
      open(FILE_D,$lpath);
      @infile = <FILE_D>;
      close(FILE_D);

      $num_lines = $#infile ;
      for($indx=0; $indx <= $num_lines; $indx++)  {
         chop($infile[$indx]);
      }

      if($dbg1 == 1) {
         print "Number lines = ",$num_lines,"\n";
      }

      #Now scan the log file for the error
      $error_found = 0;

      for($indx=0; $indx <= $num_lines; $indx++)  {
         if($dbg2 == 1) {
            print "Comparing:\n";
            print "read:",$infile[$indx],"\n";
            print "cmpr:",$compare{$testname},"\n";
         }
         if($infile[$indx] eq $compare{$testname}) {
              $error_found = 1;
         }
      }

      if($error_found == 1) {
        $pass_count++ ;
        print REPORT "$testname\t\tPASSED\n";
        print "$testname\t\tPASSED\n";
      } else {
        print REPORT "$testname\t\tFAILED\n";
        print "$testname\t\tFAILED\n";
      }

    }

    $total = $pass_count + $no_compile + $no_run + $crash_count;
    print REPORT "Tests passed: $pass_count of $num_tests total\n";
    print "Tests passed: $pass_count of $num_tests total\n";
    close (REPORT);
}
