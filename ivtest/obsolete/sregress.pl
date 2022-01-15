#!/usr/bin/env perl -s
##!/utilities/perl/bin/perl -s
#
# Copyright (c) 1999 Guy Hutchison (ghutchis@pacbell.net)
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
#
# 9/17/99 - SDW - Modified to handle multiple compilers.  This is needed
#                 to allow verification of the sweet in other environments.
#                 Right now it works with Verilog-XL. Need to debug w/ ivl
#
#                 Modified the check_results analysis to handle different
#                 compilers.  Works with Veriog-XL. Need to debug w/ ivl
#
# 9/23/99 - SDW - Added command line option to change the source of the
#                 regress.list - If a command line option is present, then
#		  it is used as the name of a file for the regress.list.
#
# 9/27/99 - SDW - Added optional "main module" arguement to the regress.list
#                 format.
#
# 10/5/99 - SDW - Added a "CN" Switch for IVL to pass -t null to the
#                 compiler.  Per Steve Williams' request.
#
# 12/27/99- SDW - added $redir to cmd generation and validated against
#                 XL again. Using &> it was going into backround on solaris.
#                 Changed $redir to -l which is the switch for XL to gen a
#                 log file. This got me a serial run.
#
# 12/31/99 - SDW - Last change of the century! Steve Williams asked for
#                  qualifying Compiler errors. So far I'm now counting
#                  sorry messages and "parse error" messages. Steve will
#                  perhaps be suprised that there are other types appearing
#                  like "failed to elaborate.."   Anyway - seems to work.
#
# 01/01/00 - SDW - Added a grep for "Unable" which should pick up the
#                  elaboration errors.
#
# 03/13/00 - SDW - Fixed REPORT print error for Compiler Error Count
#
# 05/08/00 - SDW - Added gold=filename as 4th option instead of
#                  module name (some time I'll have to make it 4th or
#                  5th to handle place where we need module names too!)
#                  Also rm'd any pre-existing log files
# 06/11/00 - SDW - Added CRASH detection for CE class tests
#
# 10/04/00 - SDW - Added suggested change from Steve Williams
#                  to remove simv.exe for the software to run
#                  on windows.

#  Global setup and paths

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
   $regress_fn = "./regress.list";
}


$logdir = "log";
$bindir = "bin";  # not currently used
$report_fn = "./regression_report.txt";

$comp_name = "IVL" ;	# Change the name of the compiler in use here.
                        # this may change to a command line option after
			# I get things debugged!

if($comp_name eq "XL") {
   $vername = "vlogcmd";	# XL's command name
   $versw   = "";			# switches
   $verout =  "";
   $redir = " -l  ";
} else {
   $vername = "iverilog";	# IVL's shell
   $versw   = "";			# switches
   $verout  = "-o simv";	# output (for IVL )
   $redir = "&>";
#   $redir = "2>&1 > ";
}

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

sub read_regression_list {
    open (REGRESS_LIST, "<$regress_fn");
    local ($found, $testname);

    while (<REGRESS_LIST>) {
	chop;
	if (!/^#/) {
	    # strip out any comments later in the file
	    s/#.*//g;
	    $found = split;
	    if ($found > 2) {
         $total_count++;
		$testname = $_[0];
		$testtype{$testname} = $_[1];
		$testpath{$testname} = $_[2];

        if($#_ eq 3)  {                    # Check for 4 fields
           if(!($_ =~ /gold=/) && !($_ =~ /diff=/ )) {
             $testmod{$testname} = $_[3];  # Module name, not gold
             $opt{$testname} = "";         # or diff
           } elsif ($_ =~ /gold=/) {
             $testmod{$testname} = "" ;	   # It's a gold file
             $opt{$testname} = $_[3] ;
           } elsif ($_ =~ /diff=/) {	   # It's a diff file
             $testmod{$testname} = "";
             $opt{$testname} = $_[3];
           }
        } elsif ($#_ eq 4) {             # Check for 5 fields
           $testmod{$testname} = $_[3];  # Module name - always in this case
           if ($_ =~ /gold=/) {
             $opt{$testname} = $_[4];
           } elsif ($_ =~ /diff=/) {
             $opt{$testname} = $_[4];
           }
        }

		push (@testlist, $testname);
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
        # First lets clean up if its' IVL. We need to know if
        # these are generated on the current pass.
        #
        if($comp_name eq "IVL") {
          if(-e "core")  {
              system("rm -f core");
           }
           if(-e "simv")  {
              system("rm -f simv");
           }
		   if(-e "simv.exe") {
              system("rm -f simv.exe");
           }
        }

        #
        # This is REALLY only an IVL switch...
        #
        # vermod is used to declare the "main module"
        #
        if( $testmod{$testname} ne "") {
           $vermod = "-s ".$testmod{$testname} ;
        } else {
           $vermod = " ";
        }
        if($comp_name eq "XL") { # Just over-ride for XL
           $vermod = " ";
        }


		print "Test $testname:";
		if ($testpath{$testname} eq "") {
			$vpath = "./$testname.v";
		} else {
			$vpath = "./$testpath{$testname}/$testname.v";
		}

		$lpath = "./$logdir/$testname.log";
        system("rm -rf $lpath");
        system("rm -rf *.out");

        # Check here for "compile only" situation and set
        # the switch appropriately.
        #
        # While we're in CO mode - take a snapshot of it. Note
        # this puts a contraint on the order -never can have a CO
        # as the FIRST test in the list for this to work.
        #

        if($testtype{$testname} ne "CO") {	# Capture ONLY
            $versw = $old_versw ;			# the non-compile only
        }									# command here.

        if(($testtype{$testname} eq "CO") ||
           ($testtype{$testname} eq "CN")) {
          if($comp_name eq "XL")  {
             $versw = "-c" ;
          } else {
             if($testtype{$testname} eq "CN") {
                  $versw = "-t null";
             } else {
                  $versw = "";
             }
          }
        } else {
          $versw = $old_versw ;	 # Restore non-compile only state
        }

        #
        # if we have a logfile - remove it first
        #
        if(-e "$lpath") {
           system("rm $lpath");
        }

        #
        # Now build the command up
        #
	#	$cmd = "$vername $versw $vermod $verout $vpath &> $lpath ";
		$cmd = "$vername $versw $vermod $verout $vpath $redir $lpath ";
		print "$cmd\n";
		system("$cmd");

        # Note that with IVL we have to execute the code now
        # that it's compiled - there is GOING to be switch in
        # the verilog switch that will make this unnecessary.

        if($comp_name eq "IVL") {
              if( -e "simv") {
                 if(!($testtype{$testname} eq "CO" ) &&
                    !($testtype{$testname} eq "CN" ) &&
                    !($testtype{$testname} eq "CE" )) {
                   system ("./simv >> $lpath");
                 } else {
                   system ("echo PASSED >> $lpath" );
                 }
              } elsif ( -e "core") {
                  system ("echo CRASHED >> $lpath" );

              } elsif ($testtype{$testname} eq "CN" ) {
                   system ("echo PASSED >> $lpath" );
              } else {
                  system ("echo COMPERR >> $lpath" );
              }
        }

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

    foreach $testname (@testlist) {
	$lpath = "$logdir/$testname.log";

    #
    # This section is used to compare against GOLD FILES
    # We compare the log file against a known GOOD result
    #
    # This section runs if gold=name is the 4th option
    #

    $gold_file = "";
    $gold_file = "";
    $diff_file = "";
    $optname = $opt{$testname} ;
    if(($opt{$testname} ne "")  && ($optname  =~ /gold=/)){
      $gold_file = $opt{$testname};
      $gold_file =~ s/gold=//;		# remove gold= operator
      system("rm -rf ./dfile");
      system("diff $lpath ./gold/$gold_file > ./dfile ");
      if( -z "dfile" ) {
        system ("echo PASSED >> $lpath" );
      } else {
        system ("echo FAILED >> $lpath");
      }
    }

    $gold_file = "";
    $diff_file = "";
    #
    # Now look for difference file requirements - use this for
    # vcd's initially I guess.
    #
    if(($opt{$testname} ne "")  && ($optname  =~ /diff=/)){
      $diff_file = $optname ;
      $diff_file =~ s/diff=//;
      system("rm -rf ./dfile");
      ($out_file,$gold_file) = split(/:/,$diff_file);
      system("diff $out_file $gold_file > ./dfile");
      if( -z "dfile" ) {
        system ("echo PASSED >> $lpath" );
      } else {
        system ("echo FAILED >> $lpath");
      }
    }

	# uncompress the log file, if a compressed log file exists
	if (-f "$lpath.gz") { system "gunzip $lpath.gz"; }

	# check the log file for the test status
	if (-f $lpath) {
		print ("Checking test $lpath\n");
		$result = `tail -150 $lpath`;

		# First do analysis for all tests that SHOULD run
		if(($testtype{$testname} ne "CO")  &&
			($testtype{$testname} ne "CE") &&
			($testtype{$testname} ne "CN")) {
			#
			# This section is true for all tests that execute -
			# no matter the compiler.
			#
			if ($result =~ /PASSED/) {
				printf REPORT "%30s passed\n", $testname;
				$pass_count++;
			} elsif (($result =~ /FAILED/)) {
				printf REPORT "%30s execution failed\n", $testname;
				$no_run++;
			}

			# Need to check for syntax errors in tests that
			# are expected to pass.

			if($comp_name eq "XL") {
				if($result =~ /Error/) {
					printf REPORT "%30s compile errors\n", $testname;
					$no_compile++ ;
				}
			} else {# IVL compile error check goes here
			    if ($result =~ /PASSED/) {
				} elsif ($result =~ /COMPERR/)  {
                   $comp_err = 0;
                   printf REPORT "%30s ",$testname;
				   if($result =~ /parse error/) {
                      printf REPORT "had parse errors:";
                      $no_parse_err++;
					  $no_compile++ ;
                      $comp_err++;
                   }
				   if(($result =~ /Unable/) ||
				      ($result =~ /unhandled/)) {
                      printf REPORT "had elaboration errors:";
					  $no_compile++ ;
                      $comp_err++;
                   }
				   if($result =~ /sorry/) {
                      printf REPORT "had unsupported features";
                      $no_sorry++ ;
					  $no_compile++ ;
                      $comp_err++;
                   }
                   if($comp_err eq 0) {
                      printf REPORT "has C Compiler problem";
					  $no_compile++ ;
                      $comperr_cnt++;
                   }
                   if($result  =~ /CRASHED/ ) {
                      printf REPORT "%30s compile crashed\n", $testname;
					  printf "%30s compile crashed(2)\n", $testname;
                      $crash_count++;
                   }
                   printf REPORT "\n";

				} elsif (($result =~ /CRASHED/)) {
					printf REPORT "%30s compile crashed\n", $testname;
					printf "%30s compile crashed (1)\n", $testname;
					$crash_count++;
				}
			}

		}

		# Now look at Compile only situation - going to be
		# different results for each compiler.

        # Test for CE case first
		if($testtype{$testname} eq "CE") {
			if($comp_name eq "XL") {
               # Deal with XL frist
				if($result =~ /Error/) {
					$pass_count++;
                }
            } else {
               # Deal with IVL here...
               if (($result =~ /CRASHED/)) {
                    printf REPORT "%30s compile crashed\n", $testname;
                    printf "%30s compile crashed (1)\n", $testname;
                    $crash_count++;
                }
            }
        }

		if(($testtype{$testname} eq "CO") ||
		   ($testtype{$testname} eq "CN")) {
			if($comp_name eq "XL") {
				if($result =~ /Error/) {
					printf REPORT "%30s compile failed\n", $testname;
					print "%30s compile failed\n", $testname;
					$no_compile++ ;
				} else {
					printf REPORT "%30s passed\n", $testname;
					$pass_count++ ;
				}
			} else {		# IVL stuff goes here.
			    if ($result =~ /PASSED/) {
					printf REPORT "%30s passed\n", $testname;
					$pass_count++;
				} elsif ($result =~ /COMPERR/) {
                   $comp_err = 0;
                   printf REPORT "%30s ",$testname;
				   if($result =~ /parse error/) {
                      printf REPORT "had parse errors:";
                      $no_parse_err++;
                      $comp_err++;
                   }
				   if(($result =~ /Unable/)  ||
				      ($result =~ /unhandled/)) {
                      printf REPORT "had elaboration errors:";
                      $comp_err++;
                   }
				   if($result =~ /sorry/) {
                      printf REPORT "had unsupported features";
                      $no_sorry++ ;
                      $comp_err++;
                   }
                   if($comp_err eq 0) {
                      printf REPORT "has C Compiler problem";
                      $comperr_cnt++;
                   }
                   if(!(comp_err eq 0)) {
                     $no_compile++;
                   }
                   printf REPORT "\n";
				} elsif ($result =~ /CRASHED/ ) {
					printf REPORT "%30s compile crashed\n", $testname;
					$crash_count++ ;
				}
            }
         }
	} else {
	    printf REPORT "%30s No log file\n", $testname;
	    $crash_count++;
	}
    }

    $total = $pass_count + $no_compile + $no_run + $crash_count;
    print REPORT "Tests passed: $pass_count, Parse Errors: $no_parse_err, Unsupported: $no_sorry, failed execution, $no_run, crashed: $crash_count, C Compiler errors: $comperr_cnt total: $total_count\n";

    print "Tests passed: $pass_count, Parse Errors: $no_parse_err, Unsupported: $no_sorry, failed execution, $no_run, crashed: $crash_count, C compiler err: $comperr_cnt total: $total_count\n";

    close (REPORT);
}
