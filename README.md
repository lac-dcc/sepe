<p align="center">
  <img alt="Sepe Banner" src="assets/images/SepeBanner.png" width="95%" height="auto"/></br>
</p>

# Reproducing Sepe Results

This directory contains scripts to reproduce the results depicted in section 4 of the Sepe paper.
The recommended way of reproducing the experiments is through [Docker](docker).

This setup has been successfully reproduced in the following Linux distribution:

**TODO: Linux setup.**

## Prerequisites

To run the experiments, the following dependencies must be installed:

| Dependency | Version   |
|------------|-----------|
| clang      | >= 14.0.0 |
| CMake      | >= 3.20   |
| Rust       | >= 1.7    |
| Python     | >= 3.10   |
| Git        | >= 2.0    |
| zip        | >= 3.0    |

Additionally, to analyze the results the following python libraries must be available:

| Dependency | Version   |
|------------|-----------|
| matplotlib | == 3.9.2  |
| numpy      | == 2.1.2  |
| pandas     | == 2.2.2  |
| scipy      | == 1.14.1 |

It may be possible to reproduce the results with different library versions. However,
in case any incompatibilities arise, it is recommended to use the exact versions
listed above.

**TODO: commands to install dependencies**.

## Reproducing the experiments

Each experiment can be reproduced by executing the script in the directory of the same name.
There is no experiment for RQ4 because it is simply RQs 1 and 2 executed in a machine with an
aarch64 CPU.

Below follows specific notes about each research question.

### Before starting

**All scripts assume that `scripts/install_abseil.sh` has already finished running**.
Make sure to do that once before starting.

#### RQ1 and R2

RQ1 and RQ2 use the same script.

For RQ1, the result of every column in our paper can be found in the following places:

  * B-Time: in file`results/global_geoman.csv`, second column.
  * H-Time: in file `results/hash-performance.txt`
  * B-Coll: in file `results/global_geoman.csv`, final column.
  * T-Coll: requires the execution of RQ3 script. After executing it, run the
    `true_collisions.sh` script.

RQ2 results will be in `results/global_collision_count.pdf`, as a graph.

#### RQ3

While executing the incremental execution, `keyuser` will print a warning
that the distribution is incorrect. This is an implementation detail that can be ignored.
The incremental distribution is implemented by the key generator, `keygen`, not `keyuser`,
but `sepe-runner` passes an `-d incremental` flag to `keyuser` anyway, which generates
the warning.

The result of each column in RQ3's table can be found in:

  * incremental: `output-rq3/incremental-result.csv`
  * uniform:     `output-rq3/uniform-result.csv`
  * normal:      `output-rq3/normal-result.csv`

#### RQ5

**TODO**

#### RQ6

**TODO**

#### RQ7

**TODO**

#### RQ8

**TODO**

#### RQ9

**TODO**
