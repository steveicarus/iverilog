#
# Module for writing to the regression report file.
#

package Reporting;

use strict;
use warnings;

our $VERSION = '1.00';

use base 'Exporter';

our @EXPORT = qw(open_report_file print_rpt close_report_file);

use constant DEF_REPORT_FN => './regression_report.txt';

$| = 1;  # This turns off buffered I/O

#
# Open the report file for writing.
# If no argument is given, DEF_REPORT_FN is the filename.
#
sub open_report_file {
    my $report_fn = shift || DEF_REPORT_FN;
    open (REGRESS_RPT, ">$report_fn") or
        die "Error: unable to open $report_fn for writing.\n";
}

#
# Print the argument to both the normal output and the report file.
#
sub print_rpt {
    print @_;
    print REGRESS_RPT @_;
}

#
# Close the report file once we're done with it.
#
sub close_report_file {
    close (REGRESS_RPT);
}


1;  # Module loaded OK
