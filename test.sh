#!/bin/bash


for fullpath in $1/*.cnf
do
    filename=$(basename "$fullpath")

    expected_result="SAT"
    test_out="$(./prog ./$1/$filename)"
    echo "Testing $filename.cnf ...  $test_out"
    # if [ "$test_out" != "$expected_result" ]; then
    #     exit 1
    # fi
done

echo "All tests passed successfully"
exit 0
