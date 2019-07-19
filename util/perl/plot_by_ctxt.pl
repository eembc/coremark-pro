#!/usr/bin/perl

# File: util/perl/plot_by_ctxt.pl
#	Description:
#	Small sample utility to plot execution speedup by context.
use Chart::Plot;

my @alldata=qw();

$switcher=0;

$name=$ARGV[0];
$name=~s/.inc//;

my $img = Chart::Plot->new; 
$img->setGraphOptions (	'title' => $name,
                        'vertAxisLabel' => 'Time(secs)' ,
						'horAxisLabel' => 'Contexts'
						) or die ($img->error);

$i=1;
open(TXT, ">$name.csv") or die "could not open $name.csv\n";
print TXT "$name\nContexts,Time\n";
while (<>) {
	chomp;
	$line=$_;
	@vals = split /=/,$_;
	if ($line =~ /Template\:contexts/) {
		$x=$vals[1];
	}
	if ($line =~ /Template\:time\(secs\)/) {
		$y=$vals[1];
		print TXT "$x,$y\n";
		push @alldata,$x;
		push @alldata,$y;
	}
}
close TXT;
for ($i=1; $i<=$x ;$i++) {
	$xTickLabels{$i}=$i;
}
$img->setData (\@alldata, 'Points Line Black') or print $img->error;
$img->setGraphOptions ('xTickLabels' => \%xTickLabels);

open (WR,">${name}.gif") or die ("Failed to write file: $!");
binmode WR;
print WR $img->draw('gif');
close WR;

