#!/usr/bin/perl
$idx=0;
use Getopt::Long; 
GetOptions (
	"idx=i" => \$idx
);

$indata=0;
while (<>) {
	if (/START DATASET/) {$indata=1;}
	$_ =~ s/index/$idx/g;
	print $_ if ($indata);
	if (/END DATASET/) {last;}
}
print "\n";

