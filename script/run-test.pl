#!/usr/bin/perl

use strict;
use warnings;

use Getopt::Long;
use Cwd;
use File::Path qw(make_path remove_tree);

sub usage {
	print << 'USAGE';
usage: run-test.pl [--fixture=dir] [--help] [--no-delete] [--output]
	[--valgrind] <testprog>

  --fixture=dir Use 'dir' for creating fixtures.
  --help	Display this help message and exit.
  --no-delete	Do not delete fixture directories on success.
  --output=file	Write pass output to 'file'.
  --valgrind	Run all tests under valgrind, reporting failures for any memory
		errors or leaks.
USAGE
	exit 0;
}

my $fixture = '.fixture';
my $nodelete = 0;
my $output = '';
my $valgrind = 0;
GetOptions (
	"fixture|f=s"	=> \$fixture,
	"help|h"	=> \&usage,
	"no-delete|n"	=> \$nodelete,
	"output|o=s"	=> \$output,
	"valgrind|v"	=> \$valgrind
) or die ("error parsing arguments\n");

my $command = '';
if ($valgrind) {
	$command = 'valgrind --error-exitcode=1 --leak-check=full '
		. '--errors-for-leak-kinds=all --quiet';
}

my $test = shift(@ARGV);
die 'no test specified' if !$test;

# set up test fixture
my $testdir = "$fixture/$test";
make_path($testdir) or die "cannot create fixture '$testdir'";
my $root = getcwd;
chdir($testdir);

# run the test
my $passed = 1;
chomp (my $result = `$command $root/$test 2>&1`);
if ($result or $?) {
	$passed = 0;
	print "$test failed ($?):\n$result\n";
} elsif ($output) {
	# my OUTPUT;
	open(OUTPUT, '>', "$root/$output") or die "cannot open $output: $!";
	print OUTPUT "PASSED\n";
}

# clean up the test fixture
chdir($root);
remove_tree($testdir) if (!$nodelete);

exit ($passed ? 0 : -1);

