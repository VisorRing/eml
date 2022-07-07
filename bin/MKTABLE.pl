#! /usr/local/bin/perl

use strict;

my @FN;
my @MD;
my @SF;
my @MTFN;
my @MTFN2;
my @WFN;
my @HDR;
my @SRCS;
my @CPPFLAGS;
#my $target = "ftable.cc";
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
#	} elsif (/^-std=/) {
#	    push (@CPPFLAGS, $_);
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
print OUT "#include \"ftable.h\"\n";
print OUT "#include \"mftable.h\"\n";
foreach (@HDR) {
    print OUT "#include \"$_\"\n";
}
print OUT "//============================================================\n";
&ftable;
print OUT "//============================================================\n";
&mftable;
print OUT "//============================================================\n";
&wikiftable;
close (OUT);

rename ($TargetTmp, $Target);

exit;

sub  fnopt {
    my (@list) = @_;
    my $nofn = 0;
    my $norval = 0;
    foreach my $i (@list) {
	if ($i eq 'nofn') {
	    $nofn = 1;
	} elsif ($i eq 'norval') {
	    $norval = 1;
	}
    }
    ($nofn, $norval);
}

sub  p {
    my ($file) = @_;
    my ($hdr, @a, $md, $ffn, $fmfn);

    $hdr = $file;
    $hdr =~ s/\.cc$/.h/;
    $ffn = $fmfn = 0;

    open (IN, $file);
    while (<IN>) {
	chomp;
	if (/^\/\/#AWFUNC\s+/) {
	    die;
	    @a = split (/\s+/, $');
	    push (@FN, [$a[0], $a[1]]);
	    push (@WFN, $a[0]);
	    $ffn = 1;
	    if ($a[0] eq '' || $a[1] eq '') {
		die "$_: bad parameter.\n";
	    }
	} elsif (/^\/\/\#XAFUNC\s+/) { 
	    @a = split (/\s+/, $');
	    push (@FN, [$a[0], $a[1], &fnopt (@a)]);
	    $ffn = 1;
	    if ($a[0] eq '' || $a[1] eq '') {
		die "$_: bad parameter.\n";
	    }
	} elsif (/^\/\/\#XMFUNC\s+/) {
	    @a = split (/\s+/, $');
	    push (@MD, [$a[0], $a[1], &fnopt (@a), $a[2]]);
	    $md = $a[0];
	    $ffn = 1;
	    if ($a[0] eq '' || $a[1] eq '' || $a[2] eq '') {
		die "$_: bad parameter.\n";
	    }
	} elsif (/^\/\/\#XSFUNC\s+/) {
	    @a = split (/\s+/, $');
	    if ($a[2] ne '') {
		push (@SF, [$a[0], $a[1], &fnopt (@a), $a[2]]);
	    } else {
		push (@SF, [$a[0], $a[1], &fnopt (@a), $md]);
	    }
	    $ffn = 1;
	    if ($a[0] eq '' || $a[1] eq '' || ($a[2] eq '' && $md eq '')) {
		die "$_: bad parameter.\n";
	    }
	} elsif (/^\/\/\#MTFUNC\s+/) {
	    @a = split (/\s+/, $');
	    push (@MTFN, [$a[0], $a[1]]);
	    $fmfn = 1;
	    if ($a[0] eq '' || $a[1] eq '') {
		die "$_: bad parameter.\n";
	    }
	} elsif (/^\/\/\#MTFUNC2\s+/) {
	    @a = split (/\s+/, $');
	    push (@MTFN2, [$a[0], $a[1]]);
	    $fmfn = 1;
	    if ($a[0] eq '' || $a[1] eq '') {
		die "$_: bad parameter.\n";
	    }
	} elsif (/^\/\/\#XWIKIFUNC\s+/) {
	    @a = split (/\s+/, $');
	    push (@WFN, $a[0]);
	    $ffn = 1;
	    if ($a[0] eq '') {
		die "$_: bad parameter.\n";
	    }
	}
    }
    close (IN);

    if (-f $hdr) {
	if ($ffn || $fmfn) {
	    push (@HDR, $hdr);
	}
#	if ($fmfn) {
#	    push (@MTHDR, $hdr);
#	}
    }
}

sub  ftable {
    print OUT "static XFTableVal  ftable[] = {\n";
    foreach (@FN) {
	print OUT "    {CharConst (\"$_->[0]\"), NULL, 0, $_->[1], $_->[2], $_->[3]},\n";
    }
    print OUT "    {NULL, 0, NULL, 0, NULL},\n";
    print OUT "};\n";

    print OUT "static XFTableVal  mtable[] = {\n";
    foreach (@MD) {
	print OUT "    {CharConst (\"$_->[0]\"), NULL, 0, $_->[1], $_->[2], $_->[3], $_->[4]},\n";
    }
    print OUT "    {NULL, 0, NULL, 0, NULL},\n";
    print OUT "};\n";

    print OUT "static XFTableVal  stable[] = {\n";
    foreach (@SF) {
	print OUT "    {CharConst (\"$_->[0]\"), CharConst (\"$_->[4]\"), (MNode*(*)(bool,MNode*,MlEnv*))$_->[1], $_->[2], $_->[3]},\n";
    }
    print OUT "    {NULL, 0, NULL, 0, NULL},\n";
    print OUT "};\n";
    print OUT "XFTable  GFTable (ftable);\nXMTable  GMTable (mtable, stable);\n";

}

sub  mftable {
    print OUT "static MFTableVal  mftable[] = {\n";
    foreach (@MTFN) {
	print OUT "    {CharConst (\"$_->[0]\"), $_->[1]},\n";
    }
    print OUT "    {NULL, 0, NULL},\n";
    print OUT "};\n";
    print OUT "static MFTable2Val  mftable2[] = {\n";
    foreach (@MTFN2) {
	print OUT "    {CharConst (\"$_->[0]\"), $_->[1]},\n";
    }
    print OUT "    {NULL, 0, NULL},\n";
    print OUT "};\n";
    print OUT "MFTable  IMFTable (mftable);\n";
    print OUT "MFTable2  IMFTable2 (mftable2);\n";
}

sub  wikiftable {
    my (%fn);

    foreach (@FN) {
	if (defined ($fn{$_->[0]})) {
	    die "$_->[0]: defined twice.\n";
	}
	$fn{$_->[0]} = "$_->[1], $_->[2], $_->[3]";
    }
    print OUT "static XFTableVal  wikiftable[] = {\n";
    foreach (@WFN) {
	if (defined ($fn{$_})) {
	    print OUT "    {CharConst (\"$_\"), NULL, 0, $fn{$_}},\n";
	} else {
	    die "$_: undefined wiki function.\n";
	}
    }
    print OUT "    {NULL, 0, NULL, 0, NULL},\n";
    print OUT "};\n";
    print OUT "XFTable  GWikiFTable (wikiftable);\n";
    
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
