#!/bin/bash

#
# Formatted Print.
#
# First arg is the string to print. Required.
# Second arg is the foreground color. Defaults to white.
# Third arg is the background color. Defaults to black.
# Fourth arg is the style. Defaults to normal.
#
#  Example: fpnt "Red on white, bold text" $RED $WHT $BLD
#
#

BLK="0" # black
RED="1" # red
GRN="2" # green
YEL="3" # yellow
BLU="4" # blue
MAG="5" # magenta
CYN="6" # cyan
WHT="7" # white

NRM="0" # default style. What you'd expect.
BLD="1" # bold lettering.
UND="4" # underlined lettering.
BNK="5" # blinking lettering.
INV="7" # color codes inverted (forecolor <=> backcolor)

function fstart {
    STYL=$4
    FORE=$2
    BACK=$3

    if [ "$STYL" == "" ]; then STYL=$NRM; fi
    if [ "$FORE" == "" ]; then FORE=$WHT; fi
    if [ "$BACK" == "" ]; then BACK=$BLK; fi

    printf "\033[%s;3%s;4%sm%s" $STYL $FORE $BACK "$1"
}

function fend {
    printf "\033[0m"
}

function fpnt {
    fstart "$@"
    fend
}

function fecho {
    fstart "$@"
    fend
    echo "" # newline
}

function ferror {
    fpnt "*** Error: " $RED
    fecho "$1" $RED
    exit $2
}

if [ "$1" == "help" ]; then
    echo ""
    fpnt "P4 Checkout" $WHT $BLK $BLD
    echo " -- the CVS-like anonymous snapshot download script for stlab.adobe.com"
    echo ""
    echo "    This script is intended for those who would like a read-only snapshot of the"
    echo "    source code depots at stlab.adobe.com:10666. It creates a temporary client"
    echo "    specification on the Perforce server to mimic the beahvior of CVS 'checkout'"
    echo "    command, and then deletes the client spec once the sync has completed. The"
    echo "    files will be placed in a folder at the current working directory of the"
    echo "    script."
    echo ""
    fpnt "Requirements" $WHT $BLK $BLD
    echo ""
    echo "    In order to use this script you must first have a Perforce command-line client"
    echo "    installed on your computer. To download this client for free please visit"
    echo "    <http://www.perforce.com>. Once you have this client downloaded it must be"
    echo "    visible from your computer's PATH environment variable."
    echo ""
    fpnt "Specifying P4PORT" $WHT $BLK $BLD
    echo ""
    echo "    If you already have a P4PORT specified in your environment, the script will use"
    echo "    that value instead of the default (stlab.adobe.com:10666)"
    echo ""
    fpnt "Syncing With Other ASL Depots" $WHT $BLK $BLD
    echo ""
    echo "    The script takes one (optional) parameter, which is the path of the depot to"
    echo "    which you would like to sync. If you do not specify a depot, the official"
    echo "    distribution depot (release) will be used. For instance, running:"
    echo ""
    echo "        $0 //source_release/..."
    echo ""
    echo "    ... will sync to the source_release depot, where you are likely to find patches"
    echo "    and updates to the sources between official releases."
    echo ""
    fpnt "Note to ASL Developers" $WHT $BLK $BLD
    echo ""
    echo "    Please note: if you are an Adobe Source Libraries developer, this is not"
    echo "    the tool for you, as the client specification is not retained, and you will"
    echo "    not be able to check in any changes to any depot."
    echo ""
    printf "$ANSI_RESET"
    exit 0;
fi

fstart "" $WHT $BLK
echo "***"
echo "*** P4 Checkout Script. Type '$0 help' for help."
printf "*** Copyright 2005-2007 "
fpnt "Adobe" $RED
fstart " Systems Incorporated" $WHT $BLK
echo ""
echo "*** Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt"
echo "*** or a copy at http://stlab.adobe.com/licenses.html)"
echo "***"
fend

MD5APP=""

if [ -e /sbin/md5 ]; then
    MD5APP="/sbin/md5"
elif [ -e /usr/bin/openssl ]; then
    MD5APP="/usr/bin/openssl md5"
fi

if [ "$MD5APP" == "" ]; then
    ferror "Could not find a binary to compute an md5 sum. Sorry." 1
fi

USER_DATA=`date; hostname; pwd; ps`
TEMP_CLIENT_NAME="temp_"`echo $USER_DATA | $MD5APP`

# echo $TEMP_CLIENT_NAME

if [ "$P4PORT" == "" ]; then export P4PORT=stlab.adobe.com:10666; fi

export P4USER=guest

echo "AdobeGuest" | p4 login > /dev/null

TEMP_CLIENTSPEC=`p4 client -o $TEMP_CLIENT_NAME`

echo "$TEMP_CLIENTSPEC" | p4 client -i > /dev/null

export P4CLIENT=$TEMP_CLIENT_NAME



if [ "$1" != "" ]; then
    p4 sync -f $1
else
    p4 sync -f //source_release/...
    p4 sync -f //platform_release/...
fi

p4 client -d $TEMP_CLIENT_NAME > /dev/null

p4 logout > /dev/null

exit 0
