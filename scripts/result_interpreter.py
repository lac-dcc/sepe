import sys
import pandas as pd
import matplotlib.pyplot as plt
from scipy.stats import mannwhitneyu

def mannwhitneyu_from_dataframe(df, column_to_group, column_to_compare):
    grouped = df.groupby(column_to_group)

    # Extract the groups from the DataFrame
    groups = {}
    groups = [group for group in grouped.groups]

    for hashFunc1 in groups:
        for hashFunc2 in groups:
            if hashFunc1 != hashFunc2:
                # Calculate the Mann-Whitney U test between the first two groups
                stat, p = mannwhitneyu(grouped.get_group(hashFunc1)[column_to_compare], grouped.get_group(hashFunc2)[column_to_compare])
                # print('Statistics=%.3f, p=%.3f' % (stat, p))
                # interpret
                alpha = 0.05
                if p > alpha:
                    print('{} and {} : p-value {} . Same distribution (fail to reject H0)'.format(hashFunc1, hashFunc2, p))
                # else:
                #     print('{} and {} : p-value {} . Different distribution (reject H0)'.format(hashFunc1, hashFunc2, p))

def box_plot_dataframe(df, regex_name):
    grouped = df.groupby(['Execution Mode', 'Num Operations', 'Num Keys', 'Insertions (%)', 'Searches (%)', 'Eliminatons(%)'])

    # Extract the groups from the DataFrame
    groups = {}
    groups = [group for group in grouped.groups]

    for group in groups:
        ax = grouped.get_group(group)[['Execution Time (s)', 'Hash Function']].boxplot(by='Hash Function', rot=45, showmeans=True)
        ax.set_ylabel('Execution Time (s)')
        ax.set_xlabel('')
        ax.set_title(group)
        plt.suptitle('')
        plt.tight_layout()
        plt.savefig(regex_name+str(group)+'.svg')

def get_table(df, regex_name):
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
    print("Results for Regex: ", regex_name)
    print("{:<20} {:<30} {:<20}".format("Func Name", "GeoTime", "GeoCollision"))
    for data in all_data:
        samples_time = 1.0
        samples_collision = 1.0
        for sample in all_data[data]:
            samples_time *= sample[0]
            if sample[1] != 0:
                samples_collision *= sample[1]
        samples_time = samples_time ** (1/len(all_data[data]))
        samples_collision = samples_collision ** (1/len(all_data[data]))
        print("{:<20} {:<30} {:<20}".format(data, samples_time, samples_collision))

def main():
    # Read CSV file names as arguments
    csv_files = sys.argv[1:]

    # Load CSV files into pandas dataframe
    dataframes = [pd.read_csv(file) for file in csv_files]

    # Concatenate dataframes
    df = pd.concat(dataframes, ignore_index=True)

    regex_name = sys.argv[1].replace("output/","").replace("0.csv","")

    # calculate geomean from a df column
    mannwhitneyu_from_dataframe(df, 'Hash Function', 'Execution Time (s)')
    print("------------------")
    # mannwhitneyu_from_dataframe(df, 'Hash Function', 'Collision Count')
    # box_plot_dataframe(df, regex_name)
    get_table(df, regex_name)

main()
