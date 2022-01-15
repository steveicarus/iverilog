#!/usr/bin/env perl
#
# Script to automatically generate regression_report-msys2.txt
# from regression_report-devel.txt and regress-msys2.list.

#use strict;

use lib './perl-lib';

use RegressionList;

my $version = $ARGV[0] || 'devel';

read_regression_list("regress-msys2.list", "any", 0, "");

my $input_name = 'regression_report-' . $version . '.txt';
open(my $input,  '<', $input_name)
  or die "ERROR - can't open '$input_name'";

my $output_name = 'regression_report-msys2-' . $version . '.txt';
open(my $output, '>', $output_name)
  or die "ERROR - can't open '$output_name'";

# Copy header.
my $line_count = 0;
while (my $line = <$input>) {
    print $output $line;
    last if ++$line_count == 2;
}

# Output results for MSYS2 test exceptions.
my $passed = 0;
my $failed = 0;
my $not_impl = 0;
my $exp_fail = 0;
my %skip_test;
foreach my $name (@testlist) {
    seek($input, 0, 0);
    while (my $line = <$input>) {
        my ($prefix, $result) = split(':', $line);
        my $test_name = $prefix =~ s/^\s+//r;  # strip leading spaces
        next if $test_name ne $name;
        if ($testtype{$test_name} eq "NI") {
            print $output "$prefix: Not Implemented.\n";
            $not_impl++;
        } elsif ($testtype{$test_name} eq "EF") {
            print $output "$prefix: Passed - expected fail.\n";
            $exp_fail++;
        } elsif ($testtype{$test_name} eq "CO") {
            print $output "$prefix: Passed - CO.\n";
            $passed++;
        } elsif ($testtype{$test_name} eq "CE") {
            print $output "$prefix: Passed - CE.\n";
            $passed++;
        } elsif ($testtype{$test_name} eq "RE") {
            print $output "$prefix: Passed - RE.\n";
            $passed++;
        } else {
            print $output "$prefix: Passed.\n";
            $passed++;
        }
    }
    $skip_test{$name} = 1;
}

# Output remaining results.
seek($input, 0, 0);
while (my $line = <$input>) {
    my ($prefix, $result) = split(':', $line);
    next if !$result;
    my $test_name = $prefix =~ s/^\s+//r;  # strip leading spaces
    next if $skip_test{$test_name};
    if ($line =~ /Not Implemented/) {
        $not_impl++;
    } elsif ($line =~ /expected fail/) {
        $exp_fail++;
    } elsif ($line =~ /Failed/) {
        $failed++;
    } elsif ($line =~ /Passed/) {
        $passed++;
    } else {
        next;
    }
    print $output $line;
}

my $total = $passed + $failed + $not_impl + $exp_fail;

print $output "=" x 76 . "\n";
print $output "Test results:\n  Total=$total, Passed=$passed, Failed=$failed, Not Implemented=$not_impl, Expected Fail=$exp_fail\n";
