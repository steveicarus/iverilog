#
# Module for parsing and loading regression test lists.
#

package RegressionList;

use strict;
use warnings;

our $VERSION = '1.01';

use base 'Exporter';

our @EXPORT = qw(read_regression_list @testlist %srcpath %testtype
                 %args %plargs %diff %gold %unordered %testmod %offset);

# Properties of each test.
# It may be nicer to have read_regression_list return an array
# of hashes with these as keys.
our (@testlist, %srcpath, %testtype, %args, %plargs,
     %diff, %gold, %unordered, %testmod, %offset) = ();

#
#  Parses the regression list file
#
#  Parameters:
#    $regress_fn = file name to read tests from.
#    $ver = iverilog version.
#
#  (from left-to-right in regression file):
#
#  test_name type,opt_ivl_args test_dir opt_module_name log/gold_file
#
#  type can be:
#    normal
#    CO = compile only.
#    CE = compile error.
#    CN = compile null.
#    RE = runtime error.
#    EF = expected fail.
#    NI = not implemented.
#
sub read_regression_list {
    my $regress_fn = shift
        or die "No regression list file name specified";
    my $ver = shift
        or die "No iverilog version specified";
    my $force_sv = shift;
    my $opt = shift;

    my ($line, @fields, $tname, $tver, %nameidx, $options);
    open (REGRESS_LIST, "<$regress_fn") or
        die "Error: unable to open $regress_fn for reading.\n";

    while ($line = <REGRESS_LIST>) {
	# can't use chomp here - in MSYS2 it only consumes the LF, not the CR
	$line =~ s/\r?\n?$//;
	# recognise a trailing '\' as a line continuation
	if ($line =~ s/\\$//) {
	    my $next_line = <REGRESS_LIST>;
	    $next_line =~ s/^\s+//;
	    $line .= $next_line;
	    redo unless eof(REGRESS_LIST);
	}
        next if ($line =~ /^\s*#/);  # Skip comments.
        next if ($line =~ /^\s*$/);  # Skip blank lines.

        $line =~ s/#.*$//;  # Strip in line comments.
        $line =~ s/\s+$//;  # Strip trailing white space.

        @fields = split(' ', $line);
        if (@fields < 2) {
            die "Error: $fields[0] must have at least 3 fields.\n";
        }

        $tname = $fields[0];
        if ($tname =~ /:/) {
            ($tver, $tname) = split(":", $tname);
            # Skip if this is not our version or option.
            next if (($tver ne "v$ver") && ($tver ne $opt));
        } else {
            next if (exists($testtype{$tname}));  # Skip if already defined.
        }

        # Get the test type and the iverilog argument(s). Separate the
        # arguments with a space.
        if ($fields[1] =~ ',') {
            ($testtype{$tname},$args{$tname}) = split(',', $fields[1], 2);
            if ($args{$tname} =~ ',') {
                my @args = split(',', $args{$tname});
                $plargs{$tname} = join(' ', grep(/^\+/, @args));
                $args{$tname} = join(' ', grep(!/^\+/, @args));
            } elsif ($args{$tname} =~ /^\+/) {
                $plargs{$tname} = $args{$tname};
                $args{$tname} = "";
            } else {
                $plargs{$tname} = "";
            }
        } else {
            $testtype{$tname} = $fields[1];
            $plargs{$tname} = "";
            $args{$tname} = "";
        }
        if ($opt ne "std") {
            $args{$tname} = $opt . $args{$tname};
        }

        $srcpath{$tname} = $fields[2];
        $srcpath{$tname} = "" if (!defined($srcpath{$tname}));

        # The four field case.
        if (@fields == 4)  {
           if ($fields[3] =~ s/^diff=//) {
               $testmod{$tname} = "" ;
               ($diff{$tname}, $gold{$tname}, $offset{$tname}) =
                   split(':', $fields[3]);
               # Make sure this is numeric if it is not given.
               if (!$offset{$tname}) {
                   $offset{$tname} = 0;
               }
           } elsif ($fields[3] =~ s/^gold=//) {
               $testmod{$tname} = "" ;
               $diff{$tname} = "";
               $gold{$tname} = "gold/$fields[3]";
               $offset{$tname} = 0;
           } elsif ($fields[3] =~ s/^unordered=//) {
               $testmod{$tname} = "" ;
               $diff{$tname} = "";
               $gold{$tname} = "gold/$fields[3]";
               $unordered{$tname} = 1;
               $offset{$tname} = 0;
           } else {
               $testmod{$tname} = $fields[3];
               $diff{$tname} = "";
               $gold{$tname} = "";
               $offset{$tname} = 0;
           }
        # The five field case.
        } elsif (@fields == 5) {
           if ($fields[4] =~ s/^diff=//) {
               $testmod{$tname} = "" ;
               ($diff{$tname}, $gold{$tname}, $offset{$tname}) =
                   split(':', $fields[4]);
               # Make sure this is numeric if it is not given.
               if (!$offset{$tname}) {
                   $offset{$tname} = 0;
               }
           } elsif ($fields[4] =~ s/^gold=//) {
               $testmod{$tname} = "" ;
               $diff{$tname} = "";
               $gold{$tname} = "gold/$fields[4]";
               $offset{$tname} = 0;
           } elsif ($fields[4] =~ s/^unordered=//) {
               $testmod{$tname} = "" ;
               $diff{$tname} = "";
               $gold{$tname} = "gold/$fields[4]";
               $unordered{$tname} = 1;
               $offset{$tname} = 0;
           }
        } else {
           $testmod{$tname} = "";
           $diff{$tname} = "";
           $gold{$tname} = "";
           $offset{$tname} = 0;
        }

        # If the name exists this is a replacement so skip the original one.
        if (exists($nameidx{$tname})) {
            splice(@testlist, $nameidx{$tname}, 1, "");
        }
        push (@testlist, $tname);
        $nameidx{$tname} = @testlist - 1;

        # The generation to use is passed if it does not match
        # the default. To make sure the tests are protable we
        # use the force SV flag to force all tests to be run
        # as the latest SystemVerilog generation. This assumes
        # the correct `begin_keywords has been added to the
        # various files.
        if ($force_sv) {
            my $fsv_flags = "-g2012";
            $args{$tname} =~ s/-g2012//;
            $args{$tname} =~ s/-g2009//;
            $args{$tname} =~ s/-g2005-sv//;
            $args{$tname} =~ s/-g2005//;
            $args{$tname} =~ s/-g2001-noconfig//;
            $args{$tname} =~ s/-g2001//;
            $args{$tname} =~ s/-g1995//;
            $args{$tname} =~ s/-g2x/-gicarus-misc/;  # Deprecated for 2001
            $args{$tname} =~ s/-g2//;  # Deprecated for 2001
            $args{$tname} =~ s/-g1//;  # Deprecated for 1995
            if ($args{$tname}) {
                $args{$tname} = "$fsv_flags $args{$tname}";
            } else {
                $args{$tname} = "$fsv_flags";
            }
        }
    }

    close (REGRESS_LIST);
}

1;   # Module loaded OK
