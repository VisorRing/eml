#! /usr/local/bin/perl

my @CMD;
my @FN;
my @FN2;
my @HDR;
my @SRCS;
#my $target = "wikitable.cc";
#my $targetTmp = "$target-$$";
my $Target;
my $TargetTmp;

while ($_ = shift) {
    if (/^-/) {
	if ($_ eq "-o") {
	    $Target = shift;
	    $TargetTmp = ".tmp-$Target";
	} elsif (/^-[DI]/) {
	    push (@CPPFLAGS, $_);
	}
    } else {
	if (-f $_) {
	    push (@SRCS, $_);
	} else {
	    die "$_: not found\n";
	}
    }
}

foreach (@SRCS) {
    &p ($_);
}

open (OUT, "> $TargetTmp");
print OUT "#include \"wikitable.h\"\n";
foreach (@HDR) {
    print OUT "#include \"$_\"\n";
}
print OUT "//============================================================\n";
print OUT "static WikiCmdTableSupport::wikicmd_t  wikitable[] = {\n";
foreach (@CMD) {
    if ($#$_ == 1) {
	print OUT "    {CharConst (\"$_->[0]\"), NULL, 0, NULL, 0, $_->[1], NULL},\n";
    } elsif ($#$_ == 2) {
	print OUT "    {CharConst (\"$_->[0]\"), NULL, 0, CharConst (\"$_->[1]\"), $_->[2], NULL},\n";
    } elsif ($#$_ == 3) {
	if ($_->[2] =~ /^\$/) {
	    print OUT "    {CharConst (\"$_->[0]\"), CharConst (\"$_->[1]\"), CharConst (\"$_->[2]\"), $_->[3], NULL},\n";
	} else {
	    print OUT "    {CharConst (\"$_->[0]\"), NULL, 0, CharConst (\"$_->[1]\"), $_->[2], $_->[3]},\n";
	}
    } else {
	die "error: $_\n";
    }
}
print OUT "    {NULL, 0, NULL, 0, NULL},\n";
print OUT "};\n";
print OUT "WikiCmdTable  GWikiCmdTable (wikitable);\n";
print OUT "//============================================================\n";
print OUT "static WikiCmdTableSupport::wikifunc_t  wftable[] = {\n";
foreach (@FN) {
    print OUT "    {CharConst (\"$_->[0]\"), $_->[2], (bool(*)())$_->[1]},\n";
    die "error\n" if ($_->[2] eq '');
}
print OUT "    {NULL, 0, WikiCmdTableSupport::WikiArg1, NULL},\n";
print OUT "};\n";
print OUT "WikiFuncTable  GWikiFuncTable (wftable);\n";
print OUT "//============================================================\n";
close (OUT);

rename ($TargetTmp, $Target);

exit;

sub  p {
    my ($file) = @_;
    my ($hdr, @a, $ffn, $x, $c, $name);

    $hdr = $file;
    $hdr =~ s/\.cc$/.h/;
    $ffn = 0;

    open (IN, $file);
    while (<IN>) {
	chomp;
	if (/^\/\/\#WIKICMD\s+/) {
	    @a = split (/\s+/, $');
	    push (@CMD, [@a]);
	    $ffn = 1;
	} elsif (/^\/\/\#WIKILINE2?\s+/) {
	    @a = split (/\s+/, $');
	    push (@FN, [$a[0], $a[1]]);
	    $ffn = 1;
	} elsif (/^bool\s+(wl_[^ \t\(]+)/) {
	    $name = $1;
	    $c = 0;
	    $x = $_;
	    while ($x !~ /\)/) {
		chomp ($x .= <IN>);
	    }
	    if ($x =~ /\(\s*WikiMotorObjVec\*\s*\w+\s*,\s*WikiMotorObjVec&/) {
		&setFN (\@FN, $name, WikiCmdTableSupport::WikiArg1);
	    } elsif ($x =~ /\(\s*WikiMotorObjVecVec\*\s*\w+\s*,\s*WikiMotorObjVec&/) {
		&setFN (\@FN, $name, WikiCmdTableSupport::WikiArgM);
	    } elsif ($x =~ /\(\s*WikiMotorObjVecVec\*\s+\w+\s*,\s*WikiMotorObjVec\*\s+\w+\s*,\s*WikiMotorObjVec&/) {
		&setFN (\@FN, $name, WikiCmdTableSupport::WikiArgM2);
	    }
	}
    }
    close (IN);

    if (-f $hdr) {
	if ($ffn) {
	    push (@HDR, $hdr);
	}
    }
}

sub  countComma {
    my ($str) = @_;
    my (@a) = split (/,/, $str, -1);
    return ($#a);
}

sub  setFN {
    my ($rfn, $name, $val) = @_;
    foreach (@$rfn) {
	if ($_->[1] eq $name) {
	    $_->[2] = $val;
	}
    }
}

sub  cmpFile {
    my ($f1, $f2) = @_;
    my ($rc);

    $rc = system ('/usr/bin/cmp', '-s', $f1, $f2) >> 8;
    return $rc;
}
