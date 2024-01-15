#!/bin/bash

# This script runs the keygen and keyuser programs with different regexes

set -e # Exit on error

# Keygen Parameters

    NUM_KEYS_TO_GENERATE=1000000 # Number of random keys to generate
    KEYGEN_SEED=223554           # Chosen by a fair roll of the dice

    CPF_REGEX="(([0-9]{3})\.){2}[0-9]{3}-[0-9]{2}"
    SSN_REGEX="[0-9]{3}-[0-9]{2}-[0-9]{4}"
    IPV4_REGEX="(([0-9]{3})\.){3}[0-9]{3}"
    IPV6_REGEX="([0-9a-fA-F]{4}:){7}[0-9a-fA-F]{4}"
    INTEGER64_REGEX="[0-9]{1,20}"
    INTEGER32_REGEX="[0-9]{1,10}"
    # FLOAT64_REGEX="[0-9]+\.[0-9]+([Ee]([+-])?[0-9])?"
    FLOAT64_REGEX="[1-9][0-9]{1,200}\.[0-9]{1,100}"
    FLOAT32_REGEX="[1-9][0-9]{1,30}\.[0-9]{1,10}"
    FLOAT32_REGEX="[0-9]+\.[0-9]+"
    OCTAL64_REGEX="[0-7]{1,22}"
    OCTAL32_REGEX="[0-7]{1,11}"
    HEX32_REGEX="[0-9a-fA-F]{1,8}"
    HEX64_REGEX="[0-9a-fA-F]{1,16}"
    ALL_REGEXES="$CPF_REGEX $SSN_REGEX $IPV4_REGEX $IPV6_REGEX $INTEGER64_REGEX
                 $INTEGER32_REGEX $FLOAT64_REGEX $FLOAT32_REGEX $OCTAL64_REGEX 
                 $OCTAL32_REGEX $HEX32_REGEX $HEX64_REGEX"
    # EXEC_REGEXES=$ALL_REGEXES
    EXEC_REGEXES="$SSN_REGEX"

# KeyUser Parameters
    TARGET=keyuser # keyuser or keyuser-debug
    sed -i "s/TARGET=.*/TARGET="${TARGET}"/g" keyuser/Makefile



    GENERIC_HASHES="STDHash FNVHash"
    IPV4_HASHES="IPV4HashGeneric IPV4HashMove IPV4HashUnrolled"
    SSN_HASHES="SSNHashBitOps"
    CPF_HASHES="CPFHashBitOps CPFHashVectorizedMul"
    ALL_HASHES="$GNERIC_HASHES $SSN_HASHES $CPF_HASHES $IPV4HASHES" # All hashes in customHashes.hpp
    EXEC_HASHES="$GENERIC_HASHES $SSNHASHES" # Hashes to execute
    NUM_OPERATIONS=1000000  # Total number of KEYGEN_INSERT, KEYGEN_SEARCH, and KEYGEN_ELIMINATION operations
    KEYGEN_INSERT=50        # Percentage of KEYGEN_INSERT operations
    KEYGEN_SEARCH=30        # Percentage of KEYGEN_SEARCH operations
    KEYGEN_ELIMINATION=20   # Percentage of KEYGEN_ELIMINATION operations
    KEYUSER_SEED=223554     # Chosen by a fair roll of the dice
    VERBOSE=""              # Verbose flag

# Build KeyGen
    cd keygen/ && cargo build --release && cd -
# Build KeyUser
    cd keyuser/ && make clean && make && cd -

# Iterate Regex types and Execute
    for REGEX in $EXEC_REGEXES; do
        echo
        echo "Running with regex: $REGEX"
        KEYGEN="./keygen/target/release/keygen $REGEX -n $NUM_KEYS_TO_GENERATE -s $KEYGEN_SEED"
        $KEYGEN | ./keyuser/$TARGET --hashes $EXEC_HASHES -i $KEYGEN_INSERT -s $KEYGEN_SEARCH -e $KEYGEN_ELIMINATION -n $NUM_OPERATIONS -seed $KEYUSER_SEED $VERBOSE
    done
