#!/usr/bin/perl
$read_items=0;
while (<>) {
	chomp;
	$line = $_;
	if ($line =~ /^ITEMS/) {
		$read_items=1;
		next;
	}
	next if (!$read_items);
	if ($line !~ /[^\\]/) {
		$read_items=0;
	}
	$items .= $_;
}

$items =~ s/\\//g;
$items =~ s/\t//g;
$items =~ s/\s\s+/ /g;

print $items;