#!/usr/bin/env python3

# This script reads multiple .csv files produced by keyuser, extracts relevant
# information and calculate metrics to compare the different hash functions

import os
import argparse
import importlib
import re

# Data science boys
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from scipy import stats

def distribution_test(key, value):

    n = len(value)
    chi_worst_case = ((n-1)**2) + (n-1)

    # Calculate the observed frequencies of each value
    observed_freq, _ = np.histogram(value, bins=len(np.unique(value)))

    # Calculate the expected frequencies for a uniform distribution
    expected_freq = np.full_like(observed_freq, len(value) / len(np.unique(value)))

    # Perform the Chi-Square Goodness of Fit Test
    try:
        chi2, p = stats.chisquare(observed_freq, expected_freq)
    except Exception:
        p = 0.0
        chi2 = chi_worst_case

    skewness = stats.skew(value)

    return pd.DataFrame(pd.DataFrame({"Hash Function": [key], "Skewness": [skewness], "Chi-Test": [chi2], "Uniform?": [p > 0.05]}))
    
def handle_distribution_analysis(args):
    try:
        arrays = {}
        for file in args.distribution:
            abs_path = os.path.abspath(file)
            spec = importlib.util.spec_from_file_location(os.path.basename(file.replace(".py","")),abs_path)
            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)
            arrays[file] = module
    except ImportError:
        print(args.distribution, "is an invalid import! Please make sure that the module exists and is in the same directory as this script.")
        print("If it does not exist, you can obtain it by running keyuser with --test-distribution or bench-runner with --histogram ", args.distribution)
        exit(1)

    result_array = None
    for module_name, current_module in arrays.items():
        regex_name = os.path.basename(module_name).replace(".py", "")

        if args.hash_functions is None:
            args.hash_functions = []
            for key, _ in current_module.distributions.items():
                args.hash_functions.append(key)

        result = None
        if args.plot_graph:
            file_destination = args.output_destination + regex_name + "_histogram.svg"
            _, ax = plt.subplots(figsize=(10, 5))
        
        for key, value in current_module.distributions.items():
            key = key.replace("array_", "")

            if result is None:
                result = distribution_test(key, value)
            else:
                result = pd.concat([result, distribution_test(key, value)], ignore_index=True)

        print(result)

        # Normalize "Chi-Test" column using the "STDHashSrc" as the reference
        result["Chi-Test"] = result["Chi-Test"] / result[result["Hash Function"] == "STDHashSrc"]["Chi-Test"].values[0]

            # Remove all entries from the dataframe that contain Simd Keyword
        result = result[~result['Hash Function'].str.contains("Simd")]
        result = result[~result['Hash Function'].str.contains("Murmur")]

        # Replace all instances of AbseilHash with Abseil
        result['Hash Function'] = [re.sub(r'AbseilHash.*', 'Abseil', x) for x in result['Hash Function']]
        result['Hash Function'] = [re.sub(r'FNVHash.*', 'FNV', x) for x in result['Hash Function']]
        result['Hash Function'] = [re.sub(r'CityHash.*', 'City', x) for x in result['Hash Function']]

        # Concatenate all entries on the dataframe HashFunction that start with the same Prefix
        result['Hash Function'] = [re.sub(r'Pext.*', 'Pext', x) for x in result['Hash Function']]
        result['Hash Function'] = [re.sub(r'OffXor.*', 'OffXor', x) for x in result['Hash Function']]
        result['Hash Function'] = [re.sub(r'Naive.*', 'Naive', x) for x in result['Hash Function']]
        result['Hash Function'] = [re.sub(r'Gperf.*', 'Gperf', x) for x in result['Hash Function']]
        result['Hash Function'] = [re.sub(r'Gpt.*', 'Gpt', x) for x in result['Hash Function']]
        result['Hash Function'] = [re.sub(r'STDHashSrc.*', 'STL', x) for x in result['Hash Function']]
        result['Hash Function'] = [re.sub(r'Aes.*', 'Aes', x) for x in result['Hash Function']]

        if result_array is None:
            result_array = result
        else:
            result_array = pd.concat([result_array, result], ignore_index=True)

    # Remove all entries from the dataframe that contain Simd Keyword
    result_array = result_array.groupby("Hash Function")["Chi-Test"].sum().reset_index()
    result_array["Chi-Test"] = result_array["Chi-Test"] / len(arrays)

    print("Below DataFrame from distribution file: ", args.distribution)
    print(result_array)
    output_path = args.output_destination + regex_name + "_chitest.csv"
    print("See all results in: ", output_path)
    result_array.to_csv(output_path, index=False)

    # if args.plot_graph:
    #     ax.legend(loc='upper left', bbox_to_anchor=(1, 1))
    #     plt.tight_layout()
    #     plt.savefig(file_destination)
    #     print("Histograms saved in: ", file_destination)

########################
# Performance Analysis #
########################

def mannwhitneyu_from_dataframe(args, regex_name, df, column_to_group, column_to_compare):
    grouped = df.groupby(column_to_group)

    # Extract the groups from the DataFrame
    groups = {}
    groups = [group for group in grouped.groups]

    # Initialize an empty DataFrame
    result = pd.DataFrame()

    for hashFunc1 in groups:
        for hashFunc2 in groups:
            if hashFunc1 == hashFunc2:
                continue

            # Calculate the Mann-Whitney U test between the first two groups
            _, p = stats.mannwhitneyu(grouped.get_group(hashFunc1)[column_to_compare], grouped.get_group(hashFunc2)[column_to_compare])

            # interpret
            alpha = 0.05

            # Append a new row to the DataFrame
            result = pd.concat([result, pd.DataFrame({"Hash Function 1": [hashFunc1], "Hash Function 2": [hashFunc2], "p-value": [p], "Same Distribution?": [p > alpha]})], ignore_index=True)


    
    print("Below DataFrame from Regex: ", regex_name)
    print(result)
    column_to_compare = column_to_compare.replace(" ", "_")
    output_path = args.output_destination + regex_name + column_to_compare + "_mannwhitneyu.csv"
    print("See all results in: ", output_path)
    result.to_csv(output_path, index=False)


def containers_boxplot(args, df):

    # grouped = df.groupby(['Execution Mode', 'Num Operations', 'Num Keys', 'Insertions (%)', 'Searches (%)', 'Eliminatons(%)', 'Hash Container'])
    # # Extract the groups from the DataFrame
    # groups = {}
    # groups = [group for group in grouped.groups]

    # all_data = {}

    # for group in groups:
        
    #     temp = grouped.get_group(group)[['Execution Time (s)', 'Collision Count']]

    #     hash_func_name = group[-1]

    #     if hash_func_name not in all_data:
    #         all_data[hash_func_name] = [(temp['Execution Time (s)'].mean(), temp['Collision Count'].mean())]
    #     else:
    #         all_data[hash_func_name].append((temp['Execution Time (s)'].mean(), temp['Collision Count'].mean()))

    # # Geometric mean of all_data
    # result = pd.DataFrame()

    # for data in all_data:
    #     samples_geotime = 1.0
    #     samples_collision = 1.0
    #     for sample in all_data[data]:
    #         samples_geotime *= sample[0]
    #         if sample[1] != 0:
    #             samples_collision *= sample[1]
    #     samples_geotime = samples_geotime ** (1/len(all_data[data]))
    #     samples_collision = samples_collision ** (1/len(all_data[data]))
    #     result = pd.concat([result, pd.DataFrame({"Func Name": [data], "GeoTime": [samples_geotime], "GeoCollision": [samples_collision]})], ignore_index=True)

    # print("Below DataFrame from Regex: ", regex_name)
    # print(result)
    # output_path = args.output_destination + regex_name + "_geomean.csv"
    # print("See all results in: ", output_path)
    # result.to_csv(output_path, index=False)

    plt.rcParams['font.size'] = 14
    df.boxplot(column='Execution Time (s)', by='Hash Container', rot=45, showmeans=True, showfliers=False)
    plt.ylabel('Execution Time (s)')
    plt.xlabel('')
    plt.xticks([1, 2, 3, 4], ["U_Map", "UM_Map", "UM_Set", "U_Set"])
    plt.title('')   
    plt.suptitle('')
    plt.tight_layout()
    plt.savefig(args.output_destination+'containers.pdf')

    print("Container Boxplots saved in: ", args.output_destination+'containers.pdf')


def performance_from_dataframe(args, df, regex_name):
    grouped = df.groupby(['Execution Mode', 'Num Operations', 'Num Keys', 'Insertions (%)', 'Searches (%)', 'Eliminatons(%)', 'Hash Function'])

    # Extract the groups from the DataFrame
    groups = {}
    groups = [group for group in grouped.groups]

    all_data = {}

    for group in groups:
        temp = grouped.get_group(group)[['Execution Time (s)', 'Collision Count']]
        
        hash_func_name = group[-1]

        if hash_func_name not in all_data:
            all_data[hash_func_name] = [(temp['Execution Time (s)'].mean(), temp['Collision Count'].mean())]
        else:
            all_data[hash_func_name].append((temp['Execution Time (s)'].mean(), temp['Collision Count'].mean()))

    # Geometric mean of all_data
    result = pd.DataFrame()

    for data in all_data:
        samples_geotime = 1.0
        samples_collision = 1.0
        for sample in all_data[data]:
            samples_geotime *= sample[0]
            if sample[1] != 0:
                samples_collision *= sample[1]
        samples_geotime = samples_geotime ** (1/len(all_data[data]))
        samples_collision = samples_collision ** (1/len(all_data[data]))
        result = pd.concat([result, pd.DataFrame({"Func Name": [data], "GeoTime": [samples_geotime], "GeoCollision": [samples_collision]})], ignore_index=True)

    print("Below DataFrame from Regex: ", regex_name)
    print(result)
    output_path = args.output_destination + regex_name + "_geomean.csv"
    print("See all results in: ", output_path)
    result.to_csv(output_path, index=False)

def handle_performance_analysis(args):

    # Load CSV files into pandas dataframe
    csv_files = args.performance
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

    if args.hash_performance:
        # Iterate Hash Functions in the dataframe
        grouped = df.groupby(['Hash Function'])
        for hashFunc in grouped.groups:
            mean = grouped.get_group(hashFunc)['Elapsed Time (seconds)'].mean()
            mean = mean * 1000
            print(f"{hashFunc},{mean:.4f}")
        return

    # Calculate the Mann-Whitney U test
    mannwhitneyu_from_dataframe(args, regex_name, df, 'Hash Function', 'Execution Time (s)')
    mannwhitneyu_from_dataframe(args, regex_name, df, 'Hash Function', 'Collision Count')

    if args.rq6:
        containers_boxplot(args, df)

    performance_from_dataframe(args, df, regex_name)

    plt.rcParams['font.size'] = 14
    df.boxplot(column='Collision Count', by='Hash Function', rot=45, showmeans=True, showfliers=False)
    plt.ylabel('Collision Count')
    plt.xlabel('')
    plt.title('')   
    plt.suptitle('')
    plt.tight_layout()
    plt.savefig(args.output_destination+'global_collision_count.pdf')
    print("Collision Count Boxplots saved in: ", args.output_destination+'global_collision_count.pdf')

    df = df[df['Hash Function'] != 'Gperf']
    df.boxplot(column='Execution Time (s)', by='Hash Function', rot=45, showmeans=True, showfliers=False)
    plt.ylabel('Execution Time (s)')
    plt.xlabel('')
    plt.title('')
    plt.suptitle('')
    plt.tight_layout()
    plt.savefig(args.output_destination+'global_performance.pdf')
    print("Execution Time Boxplots saved in: ", args.output_destination+'global_performance.pdf')

def main():
    parser = argparse.ArgumentParser(description="Keyuser Interpreter")
    parser.add_argument("-d", "--distribution", nargs='*', type=str, default="", help="Name of the distribution files to interpret. Exclusive with -p option.")
    parser.add_argument("-p", "--performance", nargs='*', type=str, default="", help="Name of the csv performance files to interpret. Exclusive with -d option.")
    parser.add_argument("-hp", "--hash-performance", action='store_true', help="Name of the csv performance files to interpret.")
    parser.add_argument("-rq6", action='store_true', help="Group performance by data structure.")
    parser.add_argument("-pg", "--plot-graph", action='store_true', help="Option to plot the results in graphs.")
    parser.add_argument("-od", "--output-destination", type=str, default="results/", help="Output path to output graphs. Default is current file.")
    parser.add_argument("-fp", "--full-print", action='store_true', help="Print the entire dataframe.")
    parser.add_argument("-hf", "--hash-functions", nargs='*', type=str, help="Name of the hash functions to analyze.")

    args = parser.parse_args()

    if args.full_print:
        pd.set_option('display.max_rows', None)
        pd.set_option('display.max_columns', None)
        pd.set_option('display.width', None)
        pd.set_option('display.max_colwidth', None)

    if args.performance:
        handle_performance_analysis(args)
    elif args.distribution:
        handle_distribution_analysis(args)

main()
