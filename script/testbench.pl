#!/usr/bin/perl

use strict;
use warnings;

use Getopt::Long;
use File::Path qw(make_path remove_tree);

sub usage {
	print << 'USAGE';
usage: testbench.pl [--fixture=dir] [--help] [--no-delete] [--valgrind]
		[testprog] [...]

  --fixture=dir Use 'dir' for creating fixtures.
  --help	Display this help message and exit.
  --no-delete	Do not delete fixture directories on success.
  --valgrind	Run all tests under valgrind, reporting failures for any memory
		errors or leaks.
USAGE
	exit 0;
}

my $valgrind = 0;
my $nodelete = 0;
my $fixture = '.fixture';
GetOptions (
	"fixture|f=s"	=> \$fixture,
	"help|h"	=> \&usage,
	"no-delete|n"	=> \$nodelete,
	"valgrind|v"	=> \$valgrind
) or die ("error parsing arguments\n");

my $total = 0;
my $passed = 0;

my $command = '';
if ($valgrind) {
	$command = 'valgrind --error-exitcode=1 --leak-check=full '
		. '--errors-for-leak-kinds=all --quiet';
}

remove_tree($fixture);

foreach (@ARGV) {
	$total++;

	my $testdir = "$fixture/$_";
	make_path($testdir) or die "cannot create fixture '$testdir'!";
	chomp (my $result = `$command ./$_ "$testdir" 2>&1`);
	if ($result or $?) {
		print "\n--------\n$_ failed: $result\n--------\n";
	} else {
		print ".";
		unless ($nodelete) {
			remove_tree($testdir) or warn "cannot remove $testdir";
		}
		$passed++;
	}
}

print "\n--------\nPassed $passed / $total.\n";

remove_tree($fixture) if (!$nodelete && $passed == $total);
exit (($passed == $total) ? 0 : -1);

