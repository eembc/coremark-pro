#!/usr/bin/perl

#extract all results from the log file
my $workers=0;
while (<>) {
	chomp;
	$lastline=$_;
	my @fields = split /\t/;
	my $nfields=scalar @fields;
	#collect field indexes
	if ($id==0) {
		foreach my $f (@fields) {
			$field_id{"$f"}=$id;
			$id++;
		}
		$id--;
		$iid=$field_id{"UID"};
		$ires=$field_id{"It/s"};
		$ictx=$field_id{"Contexts"};
		$iname=$field_id{"Name"};
		next;
	}
	#now save all results
	if (/^\#/) {
		if (/XCMD=([^\n]*)/) {
			if ($1 =~ /-w(\d+)/) {
				$workers=$1;
			}
		} else {
			$workers=0;
		}
		next;
	}
	next if ($nfields < $ires);
	$lastuid=$fields[$iid];
	$all_fields{$lastuid}=\@fields;
	$name{$lastuid}=$fields[$iname];
	$uids{$fields[$iname]}=$lastuid;
	$ctx{$lastuid}.="$fields[$ictx],";
	$its{$lastuid}.="$fields[$ires],";
	$work{$lastuid}.="$workers,";
	push @{$res{$lastuid}}, $fields[$ires];
}

#for each workload, get all values
$ctx{$lastuid}=~ s/,[^\d]//;
#print "LAST:$name{$lastuid}:$ctx{$lastuid}\n";
my @ct=split /,/,$ctx{$lastuid};
my @sct=sort { $a <=> $b } @ct;
my $pct=join ',',@sct;
print "Name,$pct\n";
foreach $name (sort keys %uids) {
	next if ($name =~ /empty/ || $name eq "Name");
	$uid=$uids{$name};
	my $numres=scalar @{$res{$uid}};
	$ctx{$uid}=~ s/,[^\d]//;
	$itx{$uid}=~ s/,[^\d]//;
	my @re=split /,/,$its{$uid};
	my @ct=split /,/,$ctx{$uid};
	my $i=0;
	my %sres=();
	foreach $ct (@ct) {
		$sres{$ct}=$re[$i++];
	}
	my $fres="";
	foreach $ct (sort { $a <=> $b } keys %sres) {
		$fres.=$sres{$ct}.",";
	}
	print "$name{$uid},$fres\n";
}

if ($work{$lastuid} =~ /[^0,]/) {
	$work{$lastuid}=~ s/,[^\d]//;
	my @ct=split /,/,$work{$lastuid};
	my @sct=sort { $a <=> $b } @ct;
	my $pct=join ',',@sct;
	print "By Worker\nName,$pct\n";
	foreach $name (sort keys %uids) {
		next if ($name =~ /empty/ || $name eq "Name");
		$uid=$uids{$name};
		my $numres=scalar @{$res{$uid}};
		$work{$uid}=~ s/,[^\d]//;
		$itx{$uid}=~ s/,[^\d]//;
		my @re=split /,/,$its{$uid};
		my @ct=split /,/,$work{$uid};
		my $i=0;
		my %sres=();
		foreach $ct (@ct) {
			$sres{$ct}=$re[$i++];
		}
		my $fres="";
		foreach $ct (sort { $a <=> $b } keys %sres) {
			$fres.=$sres{$ct}.",";
		}
		print "$name{$uid},$fres\n";
	}
}
