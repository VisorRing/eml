#! /usr/local/bin/perl

my @ID;
my @SRCS;
my %CClass = ('MLFunc' => 1);
my $Target;

while ($_ = shift) {
    if (/^-/) {
	if ($_ eq "-o") {
	    $Target = shift;
	} elsif (/^-[DI]/) {
#	    push (@CPPFLAGS, $_);
	}
    } else {
	if (-f $_) {
	    push (@SRCS, $_);
#	} elsif (-f "../ext/$_") {
#	    push (@SRCS, "../ext/$_");
#	} elsif (-f "../modules/$_") {
#	    push (@SRCS, "../modules/$_");
	} else {
	    die "$_: not found\n";
	}
    }
}

foreach (@SRCS) {
    &p ($_);
}

my $targetTmp = ".tmp-$Target";
open (OUT, "> $targetTmp");
print OUT "#ifndef ML_ID_H\n";
print OUT "#define ML_ID_H\n\n";
my $c = 0;
foreach (@ID) {
    print OUT "#define $_	", (sum ($_) * 389 + $c), "\n";
    $c ++;
}
print OUT "\n#endif /* ML_ID_H */\n";
close (OUT);

rename ($targetTmp, $Target);

exit;

sub  p {
    my ($file) = @_;
    my ($hdr);

    $hdr = $file;

    open (IN, $hdr);
    while (<IN>) {
	chomp;
	if (/class\s+([a-zA-Z0-9_]+)\s*:\s*(public)\s*([a-zA-Z0-9_]+)/ && $CClass{$3}) {
	    push (@ID, "c" . $1 . "ID");
	    $CClass{$1} = 1;
	}
    }
    close (IN);
}

sub  sum {
    my ($text) = @_;
    my ($sum);

    foreach (unpack ("C*", $text)) {
	$sum += $_;
	$sum &= 0xffff;
    }
    $sum;
}

sub  cmpFile {
    my ($f1, $f2) = @_;
    my ($rc);

    $rc = system ('/usr/bin/cmp', '-s', $f1, $f2) >> 8;
    return $rc;
}

sub  mtime {
    my ($file) = @_;

    (stat ($file))[9];
}
