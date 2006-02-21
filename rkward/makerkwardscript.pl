#!/usr/bin/perl

$template = $ARGV[0];
$r_home = "\"$ARGV[1]\"";
$r_includes = "\"$ARGV[2]\"";
$r_share = "\"$ARGV[3]\"";
$r_doc = "\"$ARGV[4]\"";

open (TEMPLATE, "< $template") or die "Can't open template";
while (<TEMPLATE>) {
	$line = $_;
	$line =~ s/###R_HOME_DIR###/$r_home/;
	$line =~ s/###R_INCLUDE_DIR###/$r_includes/;
	$line =~ s/###R_SHARE_DIR###/$r_share/;
	$line =~ s/###R_DOC_DIR###/$r_doc/;
	print ($line);
}
close (TEMPLATE);