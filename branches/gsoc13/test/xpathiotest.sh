#!/bin/bash
# Test driver to test stdin, data uri

source ./functions.source

##
#function to test read access for data uri path
DataUriTest()
{
    arg=$1
    scheme=${arg:0:4}

    src=$(basename "$arg")
    filename=${src%.*}
    test=${filename}.txt
    good=$datapath/${filename}.txt
    dot=.

    # print out the metadata
    data=$(cat $datapath/$1)
    runTest exiv2 -pa $data > $test

    #check results
    diffCheckAscii $test $good
    printf $dot
}

##
#function to test read access for Stdin test
StdinTest()
{
    arg=$1
    scheme=${arg:0:4}

    src=$(basename "$arg")
    filename=${src%.*}
    test=${filename}.txt
    good=$datapath/${filename}.txt
    dot=.

    # print out the metadata
    cat $datapath/$1 | runTest exiv2 -pa - > $test

    #check results
    diffCheckAscii $test $good
    printf $dot
}

(   cd "$testdir"

    printf 'Data Uri '
    errors=0
    base64Files+=(xpath{0..4}.base64)
    for i in ${base64Files[@]}; do DataUriTest $i; done
    if [ $errors -eq 0 ]; then
        echo ' All test cases passed'
    else
        printf "\n---------------------------------------------------------\n"
        echo $errors 'Data Uri failed!'
    fi

    printf 'Stdin    '
    errors=0
    stdinFiles+=(xpath{0..4}.jpg)
    for i in ${stdinFiles[@]}; do StdinTest $i; done
    if [ $errors -eq 0 ]; then
        echo ' All test cases passed'
    else
        printf "\n---------------------------------------------------------\n"
        echo $errors 'Stdin failed!'
    fi
)

# That's all Folks!
##