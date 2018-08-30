#!/usr/bin/perl

use strict;
use warnings;

my $major = -1;
my $minor = -1;
my $patch = -1;

for (<>) {
	if (/#define\s+CO_MAJOR\s+([0-9]+)/) {
		$major = $1;
	} elsif (/#define\s+CO_MINOR\s+([0-9]+)/) {
		$minor = $1;
	} elsif (/#define\s+CO_PATCH\s+([0-9]+)/) {
		$patch = $1
	}
}

if ($major == -1 || $minor == -1 || $patch == -1) {
	exit 1;
} else {
	print "$major.$minor.$patch\n";
	exit 0;
}
