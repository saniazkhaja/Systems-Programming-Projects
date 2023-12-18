#!/bin/bash

DUMMY=test
OUTPUT=output
TEST_DIR=tests
TIMEOUT=1s
DIFF_FILE=d

PASSING=0

RED='\033[41;37m'
GREEN='\033[42m'
RESET='\033[0m'

make clean && make all

START=$(date +%s%N | cut -b1-13 | sed s/N/000/g)
for T in $(ls $TEST_DIR | grep -E "$1" | sort)
do
    make -C $TEST_DIR/$T clean && make -C $TEST_DIR/$T all
    PASS=1
    for F in $(ls $TEST_DIR/$T | grep ".c$" | sort)
    do
        echo -n -e "Running ${GREEN}positive test${RESET} $T/$F"
        echo -n $'\t'

        TESTFILE="$TEST_DIR/$T/$F"
        EXEC=$(sed 's/.c$//g' <<<"$TESTFILE")


        cp $TESTFILE $DUMMY
        mv $EXEC.exe $EXEC &> /dev/null
        T1=$(date +%s%N | cut -b1-13 | sed s/N/000/g)
        GLIBC_TUNABLES="glibc.malloc.tcache_count=0" ./$EXEC < $TESTFILE &> $OUTPUT
        RET=$?
        T2=$(date +%s%N | cut -b1-13 | sed s/N/000/g)
        TT=$((T2-T1))

        if [ $RET -eq 0 ]
        then
            echo -e "${GREEN} RUN OK in ${TT}ms ${RESET}"
            cat $OUTPUT
        else
            echo -n -e "Running ${GREEN}positive test${RESET} $T/$F"
            echo -n $'\t'
            echo -e "${RED} RUN FAIL in ${TT}ms ${RESET}"
            cat $OUTPUT
            PASS=0
            continue
        fi
    done

    if [ $PASS -eq 0 ]
    then
            echo -e "${RED}                   TEST ${T} FAILING                  ${RESET}"
            echo -e "${RED}                   TEST ${T} FAILING                  ${RESET}"
            echo -e "${RED}                   TEST ${T} FAILING                  ${RESET}"
    else
            echo -e "${GREEN}                   TEST ${T} PASSING                ${RESET}"
            echo -e "${GREEN}                   TEST ${T} PASSING                ${RESET}"
            echo -e "${GREEN}                   TEST ${T} PASSING                ${RESET}"
    fi

    PASSING=$(($PASSING+$PASS))
done
END=$(date +%s%N | cut -b1-13 | sed s/N/000/g)
echo "Finished in $((END-START))ms"

echo $PASSING
