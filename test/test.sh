#!/bin/sh

if [ "$#" -ne 1 ]; then
    echo "Path to executable is required" >&2
    exit 1
fi

executable=$1

get_test_file_key_val() {
    grep "^$2=" "$1" | cut -d= -f2
}

run_test() {
    file=$1
    name=$( get_test_file_key_val "$file" "NAME" )
    if [ -z "$name" ]; then
        name="Unknown"
    fi

    args=$( get_test_file_key_val "$file" "ARGS" )

    rc=$( get_test_file_key_val "$file" "RC" )

    check_output=$( get_test_file_key_val "$file" "CHECK_OUTPUT" )

    printf "$file: $name..."

    eval "$executable" $args 2> err.tmp > out.tmp
    actual_rc=$?

    if [ "$rc" -eq "$actual_rc" ]; then
        if [ "$check_output" -eq 1 ]; then
            out_file=`echo "$file" | sed -e 's/.test$/.out/g'`
            if diff "out.tmp" "$out_file" > diff.tmp; then
                echo " OK"
            else
                echo " ERROR"
                echo "    DIFF:"
                cat diff.tmp
                rm diff.tmp
            fi
        else
            echo " OK"
        fi
    else
        echo " ERROR"
        echo "    Expected RC: $rc, Actual RC: $actual_rc"
    fi
}


test_files=`find . -type f -regex ".*\.test$"`


echo "Testing utility for ISA project."
echo "Author: Martin Omacht"
echo ""

ntests=`echo "$test_files" | wc -l`
echo "Running $ntests tests..."

echo "$test_files" | while read -r file; do
    run_test "$file"
done

rm err.tmp
rm out.tmp
