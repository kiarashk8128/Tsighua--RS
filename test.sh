if [ -z "$1" ]
then
    echo "Usage: ./test.sh lab_id"
    exit 1
fi

mkdir fs

# test from 0 to $1
for i in $(seq 0 $1)
do
    # clear the directory.
    cd fs
    rm -rf *
    cd ..
    for test in $(ls tests/lab$i*.py)
    do
        echo "Running test $test..."
        python3 $test
        # if returns non-zero, then exit.
        if [ $? -ne 0 ]
        then
            echo "Test failed for lab $i."
            exit 1
        fi
    done
done
echo "All tests passed."
