#!/usr/bin/env python3

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.ticker import MaxNLocator


import argparse
import re

def handle_distribution(files):
    # Create empty pandas for Key Name, Shift, Hash Function and Collision Count
    shift = ""
    hash_function = ""
    collisions = ""
    df = pd.DataFrame(columns=['Shift', 'Hash Function', 'Collision Count'])

    # Get the 10 last lines from the files
    for file in files:
        with open(file, 'r') as f:
            lines = f.readlines()
            last_lines = lines[-10:]

            # Extract shift amount from file name
            # IPV6_0_SHIFT_16.csv
            shift = file.split("_")[-1].split(".")[0]
            # Extract the Hash Function name and number of collisions
            # Hash Function: AbseilHash, Collision Count: 0
            for line in last_lines:
                if "Hash Function" in line:
                    hash_function = line.split(",")[0].split(":")[1].strip()
                    collisions = int(line.split(",")[1].split(":")[1].strip())
                    df = df._append({'Shift': shift, 'Hash Function': hash_function, 'Collision Count': collisions}, ignore_index=True)

    # Replace all instances of AbseilHash with Abseil
    df['Hash Function'] = [re.sub(r'AbseilHash.*', 'Abseil', x) for x in df['Hash Function']]
    df['Hash Function'] = [re.sub(r'FNVHash.*', 'FNV', x) for x in df['Hash Function']]
    df['Hash Function'] = [re.sub(r'CityHash.*', 'City', x) for x in df['Hash Function']]

    # Concatenate all entries on the dataframe HashFunction that start with the same Prefix
    df['Hash Function'] = [re.sub(r'Pext.*', 'Pext', x) for x in df['Hash Function']]
    df['Hash Function'] = [re.sub(r'OffXor.*', 'OffXor', x) for x in df['Hash Function']]
    df['Hash Function'] = [re.sub(r'Naive.*', 'Naive', x) for x in df['Hash Function']]
    df['Hash Function'] = [re.sub(r'Gperf.*', 'Gperf', x) for x in df['Hash Function']]
    df['Hash Function'] = [re.sub(r'Gpt.*', 'Gpt', x) for x in df['Hash Function']]
    df['Hash Function'] = [re.sub(r'STDHashSrc.*', 'STL', x) for x in df['Hash Function']]
    df['Hash Function'] = [re.sub(r'Aes.*', 'Aes', x) for x in df['Hash Function']]

    # Aggregate all Collision Count with the same Hash Function and Shift
    result = df.groupby(['Hash Function', 'Shift']).agg({'Collision Count': 'mean'}).reset_index()
    result['Collision Count'] = result['Collision Count'].astype(int)

    return result

def handle_performance_analysis(files):

    # Load CSV files into pandas dataframe
    csv_files = files
    dataframes = [pd.read_csv(file) for file in csv_files]

    regex_name = "global"

    # Concatenate dataframes
    df = pd.concat(dataframes, ignore_index=True)

    # Remove all entries from the dataframe that contain Simd Keyword
    df = df[~df['Hash Function'].str.contains("Simd")]

    # Replace all instances of AbseilHash with Abseil
    df['Hash Function'] = [re.sub(r'AbseilHash.*', 'Abseil', x) for x in df['Hash Function']]
    df['Hash Function'] = [re.sub(r'FNVHash.*', 'FNV', x) for x in df['Hash Function']]
    df['Hash Function'] = [re.sub(r'CityHash.*', 'City', x) for x in df['Hash Function']]

    # Concatenate all entries on the dataframe HashFunction that start with the same Prefix
    df['Hash Function'] = [re.sub(r'Pext.*', 'Pext', x) for x in df['Hash Function']]
    df['Hash Function'] = [re.sub(r'OffXor.*', 'OffXor', x) for x in df['Hash Function']]
    df['Hash Function'] = [re.sub(r'Naive.*', 'Naive', x) for x in df['Hash Function']]
    df['Hash Function'] = [re.sub(r'Gperf.*', 'Gperf', x) for x in df['Hash Function']]
    df['Hash Function'] = [re.sub(r'Gpt.*', 'Gpt', x) for x in df['Hash Function']]
    df['Hash Function'] = [re.sub(r'STDHashSrc.*', 'STL', x) for x in df['Hash Function']]
    df['Hash Function'] = [re.sub(r'Aes.*', 'Aes', x) for x in df['Hash Function']]

    # Rename UPPER_SHIFT column to Shift
    df.rename(columns={'UPPER_SHIFT': 'Shift'}, inplace=True)

    # Remove from Key distrib all uniform and incremental keys
    df = df[~df['Key Distribution'].str.contains("uniform")]
    df = df[~df['Key Distribution'].str.contains("incremental")]

    # Aggregate all Collision Count with the same Hash Function and Shift
    df = df.groupby(['Hash Function', 'Shift']).agg({'Collision Count': 'mean'}).reset_index()
    df['Collision Count'] = df['Collision Count'].astype(int)

    return df

def main():

    parser = argparse.ArgumentParser(description="Keyuser Interpreter")
    parser.add_argument("-p", "--performance", nargs='*', type=str, default="", help="Name of the csv files to interpret.")
    parser.add_argument("-d", "--distribution", nargs='*', type=str, default="", help="Name of the collision files to interpret.")

    args = parser.parse_args()

    # csv_files = args.performance

    true_collisions_df = pd.DataFrame()
    batch_collisions_df = pd.DataFrame()

    true_collisions_df = handle_distribution(args.distribution)
    batch_collisions_df = handle_performance_analysis(args.performance)

    # Print Only 

    print(true_collisions_df)
    print("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$")
    print(batch_collisions_df)

    plt.rcParams['font.size'] = 22
    fig, ax1 = plt.subplots(figsize=(10, 6))

    # Plot true_collisions_df on the primary y-axis
    ax1.set_xlabel('Hash Bits Shifted Right')
    ax1.set_ylabel('True Collisions')

    # Set x-ticks
    xticks = [0, 16, 24, 32, 48]
    ax1.set_xticks(xticks)
    ax1.set_xticklabels(xticks)

    hash_functions = true_collisions_df['Hash Function'].unique()
    for hash_function in hash_functions:
        if hash_function == "Abseil" or hash_function == "FNV" or hash_function == "City" or hash_function == "Gperf" or hash_function == "Gpt":
            continue
        df = true_collisions_df[true_collisions_df['Hash Function'] == hash_function]
        ax1.plot(xticks, df['Collision Count'], marker='o', linestyle='-', label=f'TC ({hash_function})')

    # ax1.tick_params(axis='y', labelcolor='tab:blue')

    # # Reverse the x-axis
    # ax1.invert_xaxis()

    # Create a secondary y-axis
    ax2 = ax1.twinx()
    ax2.set_ylabel('Batch Collisions')

    hash_functions = batch_collisions_df['Hash Function'].unique()
    for hash_function in hash_functions:
        if hash_function == "Abseil" or hash_function == "FNV" or hash_function == "City" or hash_function == "Gperf" or hash_function == "Gpt":
            continue
        df = batch_collisions_df[batch_collisions_df['Hash Function'] == hash_function]
        ax2.plot(xticks, df['Collision Count'], marker='x', linestyle='--', label=f'BC ({hash_function})')

    # ax2.tick_params(axis='y', labelcolor='tab:red')

    # Ensure y-ticks are integers
    ax2.yaxis.set_major_locator(MaxNLocator(integer=True))

    # Add title and grid
    plt.title('Collision Count vs Shift')
    fig.tight_layout()
    plt.grid(True)

    # Add legends in the center left part of the graph without overlapping
    ax1.legend(loc='upper left')
    ax2.legend(loc='upper center')

    # Save the plot
    plt.savefig('rq7_bc_vs_tc.pdf')

main()
