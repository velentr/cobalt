#!/usr/bin/perl

use strict;
use warnings;

use Getopt::Long;

sub usage {
	print << 'USAGE';
usage: testbench.pl [--help] [--valgrind] [testprog] [...]

  --help	Display this help message and exit.
  --valgrind	Run all tests under valgrind, reporting failures for any memory
		errors or leaks.
USAGE
	exit 0;
}

my $valgrind = 0;
GetOptions (
	"help|h"	=> \&usage,
	"valgrind|v"	=> \$valgrind
) or die ("error parsing arguments\n");

my $total = 0;
my $passed = 0;

my $command = '';
if ($valgrind) {
	$command = 'valgrind --error-exitcode=1 --leak-check=full '
		. '--errors-for-leak-kinds=all --quiet';
}

foreach (@ARGV) {
	$total++;

	chomp (my $result = `$command ./$_ 2>&1`);
	if ($result or $?) {
		print "\n--------\n$_ failed: $result\n--------\n";
	} else {
		print ".";
		$passed++;
	}
}

print "\n--------\nPassed $passed / $total.\n";

exit (($passed == $total) ? 0 : -1);

