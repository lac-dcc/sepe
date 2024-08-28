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

    # For all entires in the dataframe with the same Hash Function and Shift, calculate the geometric mean of the Collision Count
    # df['Collision Count'] = df['Collision Count'].astype(float)
    # result = df.groupby(['Hash Function', 'Shift'])['Collision Count'].apply(gmean).reset_index()
    # result['Collision Count'] = result['Collision Count'].astype(int)

    return result

def performance_from_dataframe(df):
    grouped = df.groupby(['Execution Mode', 'Num Operations', 'Num Keys', 'Insertions (%)', 'Searches (%)', 'Eliminatons(%)', 'Hash Function', 'Shift'])

    # Extract the groups from the DataFrame
    groups = {}
    groups = [group for group in grouped.groups]

    all_data = {}

    for group in groups:
        temp = grouped.get_group(group)[['Execution Time (s)', 'Collision Count']]
        
        hash_func_name = group[-2]
        shift = group[-1]

        if (hash_func_name,shift) not in all_data:
            all_data[(hash_func_name,shift)] = [(temp['Execution Time (s)'].mean(), temp['Collision Count'].mean())]
        else:
            all_data[(hash_func_name,shift)].append((temp['Execution Time (s)'].mean(), temp['Collision Count'].mean()))

    # Geometric mean of all_data
    result = pd.DataFrame()

    for data in all_data:

        hash_func_name = data[0]
        shift_val = data[1]

        samples_geotime = 1.0
        samples_collision = 1.0
        for sample in all_data[data]:
            samples_geotime *= sample[0]
            if sample[1] != 0:
                samples_collision *= sample[1]

        samples_geotime = samples_geotime ** (1/len(all_data[data]))
        samples_collision = samples_collision ** (1/len(all_data[data]))
        result = pd.concat([result, pd.DataFrame({"Hash Function": [hash_func_name], "GeoTime": [samples_geotime], "Collision Count": [samples_collision], "Shift": [shift_val]})], ignore_index=True)

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

    result = performance_from_dataframe(df)

    # result = pd.DataFrame(columns=['Hash Function', 'Shift', 'Collision Count'])

    # # For all entires in the dataframe with the same Hash Function and Shift, calculate the geometric mean of the Collision Count
    # grouped = df.groupby(['Hash Function', 'Shift'])
    # for name, group in grouped:
    #     print("Group name:", name[0])
    #     print("Shift:", name[1])
    #     # print(group)
    #     # Calculate the geomean of the Collision Count
    #     collision_count = 1.0
    #     for index, row in group.iterrows():
    #         if row['Collision Count'] != 0:
    #             collision_count *= row['Collision Count']
    #     collision_count = collision_count ** (1/len(group))
    #     print("Collision Count:", collision_count)
    #     result = pd.concat([result, pd.DataFrame({'Hash Function': name[0], 'Shift': name[1], 'Collision Count': collision_count})], ignore_index=True)

    # Aggregate all Collision Count with the same Hash Function and Shift
    # df = df.groupby(['Hash Function', 'Shift']).agg({'Collision Count': 'mean'}).reset_index()
    # df['Collision Count'] = df['Collision Count'].astype(int)

    return result

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

    # Remove GeoTime column from batch_collisions_df
    batch_collisions_df = batch_collisions_df.drop(columns=['GeoTime'])

    plt.rcParams['font.size'] = 22

    fig1, ax1 = plt.subplots(figsize=(10, 6))
    ax1.set_xlabel('Hash Bits Shifted Right')
    ax1.set_ylabel('True Collisions')
    xticks = [0, 16, 24, 32, 48]
    ax1.set_xticks(xticks)
    ax1.set_xticklabels(xticks)

    hash_functions = true_collisions_df['Hash Function'].unique()
    for hash_function in hash_functions:
        if hash_function == "Abseil" or hash_function == "FNV" or hash_function == "City" or hash_function == "Gperf" or hash_function == "Gpt":
            continue
        df = true_collisions_df[true_collisions_df['Hash Function'] == hash_function]
        ax1.plot(xticks, df['Collision Count'], marker='o', linestyle='-', label=f'{hash_function}')

    plt.grid(True)
    ax1.legend()
    
    fig1.tight_layout()
    plt.savefig('rq7_true_collisions.pdf')

    fig2, ax2 = plt.subplots(figsize=(10, 6))
    ax2.set_xticks(xticks)
    ax2.set_xticklabels(xticks)
    ax2.set_xlabel('Hash Bits Shifted Right')
    ax2.set_ylabel('Bucket Collisions')

    hash_functions = batch_collisions_df['Hash Function'].unique()
    for hash_function in hash_functions:
        if hash_function == "Abseil" or hash_function == "FNV" or hash_function == "City" or hash_function == "Gperf" or hash_function == "Gpt":
            continue
        df = batch_collisions_df[batch_collisions_df['Hash Function'] == hash_function]
        ax2.plot(xticks, df['Collision Count'], marker='o', linestyle='-', label=f'{hash_function}')

    # Ensure y-ticks are integers
    ax2.yaxis.set_major_locator(MaxNLocator(integer=True))

    # Add title and grid

    plt.grid(True)
    fig2.tight_layout()
    ax2.legend()
    plt.savefig('rq7_bucket_collisions.pdf')

main()