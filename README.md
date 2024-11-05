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

#### RQ1

**TODO**

#### RQ2

**TODO**

#### RQ3

Note that Aes with incremental distribution is slightly different than the one
reported in the paper due to small implementations changes we've made after
collecting the results the first time. The difference is minimal and we do not believe
they affect our analysis and conclusions.

Furthermore, while executing the incremental execution, `keyuser` will print a warning
that the distribution is incorrect. This is an implementation detail that can be ignored.
The incremental distribution is implemented by the key generator, `keygen`, not `keyuser`,
but `sepe-runner` passes an `-d incremental` flag to `keyuser` anyway, which generates
the warning.

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
