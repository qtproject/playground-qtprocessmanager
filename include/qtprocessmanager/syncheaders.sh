#!/bin/sh -e

DIR="$(cd $(dirname $0); echo $PWD)"
for i in `ls -1 $DIR/../../src/core/*.h` ; do
    header=`basename $i`
    echo "#include \"../../src/core/$header\"" > $DIR/$header
done

for i in `ls -1 $DIR/../../src/declarative/*.h` ; do
    header=`basename $i`
    echo "#include \"../../src/declarative/$header\"" > $DIR/$header
done

