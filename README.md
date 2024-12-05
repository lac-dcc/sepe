<p align="center">
  <img alt="Sepe Banner" src="assets/images/SepeBanner.png" width="95%" height="auto"/></br>
</p>

# Reproducing Sepe Results

This directory contains scripts to reproduce the results depicted in section 4 of the Sepe paper.
The recommended way of reproducing the experiments is through [Docker](docker).

This setup has been successfully reproduced in the following Linux distribution:

We executed the experiments in the following machine:

```
Ubuntu 20.04.4 LTS

Kernel: Linux 5.4.0-105-generic

Architecture: x86-64

CPU: Intel(R) Xeon(R) Silver 4210 CPU @ 2.20GHz
Socket(s): 2
Core(s) per socket: 10
Total number cores with hyperthreading: 40
Active Energy Governor: performance

Total memory space (MB): 144674
```

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
  * T-Coll: requires the execution of RQ3 script. After executing it, run
    `true_collisions.sh normal`.

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

This script can take a long time to run.

The results for the `BT` columns in the paper can be found at:

  * Inc:     `output-rq5/results/global_geomean_incremental.csv`, second column.
  * Normal:  `output-rq5/results/global_geomean_normal.csv`, second column.
  * Uniform: `output-rq5/results/global_geomean_uniform.csv`, second column.

**Note the results are in seconds, while in the paper they are in milliseconds.**

The `TC` columns are calculated based on the results of `RQ3`. `RQ5` will run the `RQ3`
if it deems necessary. After `RQ5` executes, `TC` column results can be produced by
executing `true_collisions.sh <normal|uniform|incremental>`

To obtain the TC column from Table 3, for each distribution:
    ```
    # First, make sure that output-rq3 exists
    # Inside sepe/artifact/
    # TC is the second column from:
    ./true_collisions.sh incremental
    ./true_collisions.sh normal
    ./true_collisions.sh uniform
    ```

#### RQ6

RQ6 produces a graph that should show the same patterns as the ones in the
paper. The graph is in the file: `output-rq6/rq6.pdf`.

#### RQ7

RQ7 results will be in `output-rq7`, in several directories.
They are named according to their x-axis value. For example,
`output-rq7/results_0` or `output-rq7/results_16`.

The results will be in `output-rq7/results_*/global_geomean.csv`.

#### RQ8

RQ8 produces a graph that should show the same patterns as the ones in the
paper. The graph is in the file: `**results/rq8.pdf**`.

#### RQ9

RQ9's output is a pdf file in `output-rq9/containers.pdf`. It depends on RQ1 and
RQ2's execution. RQ9's script will call it automatically if necessary.
