#
# Module for processing command line arguments, etc.
#

package Environment;

use strict;
use warnings;

our $VERSION = '1.03';

use base 'Exporter';

our @EXPORT = qw(get_args get_regress_fn get_ivl_version);

use constant DEF_REGRESS_FN => './regress.list';  # Default regression list.
use constant DEF_SUFFIX => '';  # Default suffix.
use constant DEF_STRICT => 0;   # Default strict option.
use constant DEF_WITH_VALG => 0;  # Default valgrind usage (keep this off).
use constant DEF_FORCE_SV => 0;  # Default is use the generation supplied.

use Getopt::Long;

#
# Get the executable/etc. suffix.
#
sub get_args {
    my $suffix = DEF_SUFFIX;
    my $strict = DEF_STRICT;
    my $with_valg = DEF_WITH_VALG;
    my $force_sv = DEF_FORCE_SV;

    if (!GetOptions("suffix=s" => \$suffix,
                    "strict" => \$strict,
                    "with-valgrind" => \$with_valg,
                    "force-sv" => \$force_sv,
                    "help" => \&usage)) {
        die "Error: Invalid argument(s).\n";
    }

    return ($suffix, $strict, $with_valg, $force_sv);
}

sub usage {
    my $def_sfx = DEF_SUFFIX;
    my $def_opt = DEF_STRICT ? "yes" : "no";
    my $def_reg_fn = DEF_REGRESS_FN;
    my $def_with_valg = DEF_WITH_VALG ? "on" : "off";
    my $def_force_sv = DEF_FORCE_SV ? "yes" : "no";
    warn "$0 usage:\n\n" .
         "  --suffix=<suffix>  # The Icarus executable suffix, " .
         "default \"$def_sfx\".\n" .
         "  --strict           # Force strict standard compliance, " .
         "default \"$def_opt\".\n" .
         "  --with-valgrind    # Run the test suite with valgrind, " .
         "default \"$def_with_valg\".\n" .
         "  --force-sv         # Force tests to be run as SystemVerilog, " .
         "default \"$def_force_sv\".\n" .
         "  <regression file>  # The regression file, " .
         "default \"$def_reg_fn\".\n\n";
    exit;
}

#
# Get the name of the regression list file. Either the default
# or the file specified in the command line arguments.
#
sub get_regress_fn {
    my $regress_fn = DEF_REGRESS_FN;

    # Is there a command line argument (alternate regression list)?
    if ($#ARGV != -1) {
        $regress_fn = $ARGV[0];
        -e "$regress_fn" or
            die "Error: command line regression file $regress_fn doesn't exist.\n";
        -f "$regress_fn" or
            die "Error: command line regression file $regress_fn is not a file.\n";
        -r "$regress_fn" or
            die "Error: command line regression file $regress_fn is not ".
            "readable.\n";
        if ($#ARGV > 0) {
            warn "Warning: only using first file argument to script.\n";
        }
    }

    return $regress_fn;
}

#
# Get the current version from iverilog.
#
sub get_ivl_version {
    my $sfx = shift(@_);
    if (`iverilog$sfx -V` =~ /^Icarus Verilog version (\d+)\.(\d+)/) {
        if ($1 == 0) {
            return $1.".".$2;
        } else {
            return $1;
        }
    } else {
        die "Failed to get version from iverilog$sfx -V output";
    }
}

1;  # Module loaded OK
