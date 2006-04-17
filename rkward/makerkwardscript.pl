#!/usr/bin/perl

$template = $ARGV[0];
$r_home = "\"$ARGV[1]\"";

open (TEMPLATE, "< $template") or die "Can't open template";
while (<TEMPLATE>) {
	$line = $_;
	$line =~ s/###R_HOME_DIR###/$r_home/;
	print ($line);
}
close (TEMPLATE);