import argparse
import importlib

# Create a parser
parser = argparse.ArgumentParser(description="Import a module")

# Add an argument for the module name
parser.add_argument("module", help="The name of the module to import")

# Parse the command-line arguments
args = parser.parse_args()

try:
    arrays = importlib.import_module(args.module.replace(".py","")) 
except ImportError:
    print(args.module, "not found! Please make sure that the module exists and is in the same directory as this script or you are passing the absolute path of the array file.")
    print("If it does not exist, you can get it by running keyuser with --test-distribution and redirect to", args.module)
    exit(1)

import matplotlib.pyplot as plt
from scipy import stats
import numpy as np

def goodness_fit_test(key, value):
    key = key.replace("array_","")
    # Calculate the observed frequencies of each value
    observed_freq = np.bincount(abs(value))

    # Calculate the expected frequencies for a uniform distribution
    expected_freq = np.full_like(observed_freq, len(value) / len(np.unique(value)))

    # Perform the Chi-Square Goodness of Fit Test
    chi2, p = stats.chisquare(observed_freq, expected_freq)

    skewness = stats.skew(value)
    print(f" {key} skewness: {skewness}")
    print(f" {key} Chi-Square: {chi2}")
    print(f" {key} P-value: {p}")

def main():
    _, ax = plt.subplots()

    for key,value in arrays.distributions.items():
        ax.hist(value, label=key, alpha=0.5)
        goodness_fit_test(key,value)

    ax.legend()

    plt.savefig('distributions.svg')

main()
