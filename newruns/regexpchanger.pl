#!/usr/bin/perl -w

use strict;
$^I = ".bak";

while(<>){
    s/> //g;
    s/^[^Run]+//g;
    print;
}
