#!/usr/bin/env python3

# This script plots the distribution generated by running keyuser with the
# --test-distribution flag. You must pass the python file generated by keyuser
# as an argument

import argparse
import importlib

# Create a parser
parser = argparse.ArgumentParser(description="Import a module")

# Add an argument for the module name
parser.add_argument("module", help="The name of the module to import")

# Parse the command-line arguments
args = parser.parse_args()

try:
    arrays = importlib.import_module(args.module.replace(".py", ""))
except ImportError:
    print(args.module, "not found! Please make sure that the module exists and is in the same directory as this script.")
    print("If it does not exist, you can get it by running keyuser with --test-distribution and redirect to", args.module)
    exit(1)

import matplotlib.pyplot as plt
from scipy import stats
import numpy as np

def goodness_fit_test(key, value):

    n = len(value)
    print("Chi Worst case: ", ((n-1)**2) + (n-1) )

    key = key.replace("array_", "")
    # Calculate the observed frequencies of each value
    observed_freq, _ = np.histogram(value, bins=len(np.unique(value)))

    # Calculate the expected frequencies for a uniform distribution
    expected_freq = np.full_like(observed_freq, len(value) / len(np.unique(value)))

    # Perform the Chi-Square Goodness of Fit Test
    try:
        chi2, p = stats.chisquare(observed_freq, expected_freq)
    except Exception:
        print("Chi-Test failed for key ", key)
        p = 0.0

    skewness = stats.skew(value)
    if abs(skewness) > 1.0:
        print(f" {key:<20} skewness:   {skewness:.2f} --> Highly skewed. ")
    elif abs(skewness) > 0.5:
        print(f" {key:<20} skewness:   {skewness:.2f} --> Moderately skewed. ")
    else:
        print(f" {key:<20} skewness:   {skewness:.2f} --> Approximately symmetric. ")

    if p > 0.05:
        print(f" {key:<20} p-value:    {p:.2f}   Chi-Test {chi2:.2f} --> SYMMETRIC")
    else:
        print(f" {key:<20} p-value:    {p:.2f}   Chi-Test {chi2:.2f} --> ASYMMETRIC")

def main():
    _, ax = plt.subplots(figsize=(10, 5))

    for key, value in arrays.distributions.items():
        ax.hist(value, label=key, alpha=0.5)
        goodness_fit_test(key, value)

    ax.legend(loc='upper left', bbox_to_anchor=(1, 1))

    plt.tight_layout()
    plt.savefig('distributions.svg')

main()
