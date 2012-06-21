#!/usr/bin/perl

use warnings ;
use strict ;

use FindBin ;
use lib "$FindBin::Bin/../../adobe_source_libraries/tools/";
use Adobe::Codex2;

die "usage: post_build build-number username ldap-password" unless @ARGV == 3;

my ($build_number, $user, $password)=@ARGV;

for my $platform ("win32", "osx10") {
    my $Results = $Codex->AddBuild(manifestfile => "VersionInfo.xml",
                                   uri => "p4://boxer.corp.adobe.com:1770//adobe_platform_libraries", 
                                   certlevel => "1", 
                                   statusname => "Available", 
                                   ldapcredentials => { userid=>$user, password=>$password} };

    while( my ($k, $v) = each %$Results ) {
        print "key: $k, value: $v.\n";
    }

    my $errs = $Results->{fault};

    while( my ($k, $v) = each %$errs ) {
        print "key: $k, value: $v.\n";
    }  

    my $errsdetail = $errs->{faultdetail};

    while( my ($k, $v) = each %$errsdetail ) {
        print "key: $k, value: $v.\n";
    }  
    
}
