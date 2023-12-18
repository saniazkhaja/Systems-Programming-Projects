#!/bin/bash

EXEC=a1
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
    PASS=1
    for F in $(ls $TEST_DIR/$T | grep ".in$" | sort)
    do
        echo -n -e "Running ${GREEN}positive test${RESET} $T/$F"
        echo -n $'\t'

        TESTFILE="$TEST_DIR/$T/$F"
        EXPECTED=$(sed 's/.in$/.expected/g' <<<"$TESTFILE")
        MD5=$(sed 's/.in$/.md5/g' <<<"$TESTFILE")


        cp $TESTFILE $DUMMY
        mv $EXEC.exe $EXEC &> /dev/null
        T1=$(date +%s%N | cut -b1-13 | sed s/N/000/g)
        ./$EXEC $(cat $TESTFILE) &> $OUTPUT
        RET=$?
        T2=$(date +%s%N | cut -b1-13 | sed s/N/000/g)
        TT=$((T2-T1))

        if [ $RET -eq 0 ]
        then
            echo -e -n "${GREEN} RUN OK in ${TT}ms ${RESET}\t"
        else
            echo -n -e "Running ${GREEN}positive test${RESET} $T/$F"
            echo -n $'\t'
            echo -e "${RED} RUN FAIL in ${TT}ms ${RESET}"
            cat $OUTPUT
            PASS=0
            continue
        fi

        TITLE="DIFF"

        if test -f "$MD5"; then
            cat $OUTPUT | md5sum > $DIFF_FILE
            mv $DIFF_FILE $OUTPUT
            EXPECTED=$MD5
            TITLE="MD5"
        fi

         # Remove possible Windows \r from output and expected files
         sed 's/\r$//' -i $OUTPUT   &> /dev/null
         sed 's/\r$//' -i $EXPECTED &> /dev/null
         diff -y $EXPECTED $OUTPUT &> $DIFF_FILE
         DIFF=$?
         rm $DUMMY $OUTPUT &> /dev/null

         if [ $DIFF -eq 0 ]
         then
             echo -e " ${GREEN} $TITLE OK ${RESET}\t"
             if test -f "$MD5"; then
               ./$EXEC $(cat $TESTFILE)
             fi
         else
             echo -e " ${RED} $TITLE FAIL ${RESET}"
             cat $DIFF_FILE
         fi


        if [ $RET -ne 0 ] || [ $DIFF -ne 0 ]
        then
            PASS=0
            #continue 2
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
