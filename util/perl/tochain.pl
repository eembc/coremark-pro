#!perl

%old = (
"init_ctrl\\\(p," => "chain_init_ctrl\(p->map_out,p->connections,"   ,
"signal_in_ready\\\(p," => "chain_signal_in_ready\(p->map_in,p->connections,"   ,
"signal_in\\\(p," => "chain_signal_in\(p->map_in,p->connections,"   ,
"ready_in\\\(p," => "chain_ready_in\(p->map_in,p->connections,"   ,
"signal_out\\\(p," => "chain_signal_out\(p->map_out,p->connections,"   ,
"wait_prev_ready\\\(p," => "chain_wait_prev_ready\(p->map_in,p->connections,"   ,
"wait_prev_start\\\(p," => "chain_wait_prev_start\(p->map_in,p->connections,"   ,
"get_next\\\(p," => "chain_get_next\(p->map_in, p->connections," ,
"set_out\\\(p," => "chain_set_out\(p->map_out,p->connections,"   ,
"signal_out_ready\\\(p," => "chain_signal_out_ready\(p->map_out,p->connections,"   ,
"lock_in\\\(p," => "chain_lock_in\(p->map_in,p->connections,"   ,
"lock_out\\\(p," => "chain_lock_out\(p->map_out,p->connections,"   ,
"wait_next_ready\\\(p," => "chain_wait_next_ready\(p->map_out,p->connections,"   
);

while (<>) {
	$line = $_;
	if ($line =~ /"wait_prev_ready\(p,"/) {
		print "***\n";
	}
	foreach $term (keys %old) {
		if ($line =~ /$term/) {
			$newterm=$old{$term};
			$line =~ s/$term/$newterm/;
		}
	}
	print $line;
}
