 Reproducing Sepe Results via Docker

This folder contains a [Dockerfile](Dockerfile) to reproduce all the experiments in the Sepe artifact.
To create a docker image out of this docker file, you can run the following commands:

```bash
# Clone the Sepe repository artifact branch:
#
git clone --branch artifact https://github.com/lac-dcc/sepe ./sepe
# Move to where the Docker file is located:
#
cd sepe/artifact/docker/

# Build the image
#
docker build -t sepe-benchmark -f Dockerfile .

# Execute Research Question individually
docker run --rm sepe-benchmark rq1_rq2_benchmark.sh
docker run --rm sepe-benchmark rq3_benchmark.sh
docker run --rm sepe-benchmark rq5_benchmark.sh
docker run --rm sepe-benchmark rq6_benchmark.sh
docker run --rm sepe-benchmark rq7_benchmark.sh
docker run --rm sepe-benchmark rq8_benchmark.sh
docker run --rm sepe-benchmark rq9_benchmark.sh

# Open a bash section of the container
docker run --rm -ti sepe-benchmark bash

# (From container bash) Execute desired RQ scripts
./rq1_rq2_benchmark.sh
./rq3_benchmark.sh
./rq5_benchmark.sh
./rq6_benchmark.sh
./rq7_benchmark.sh
./rq8_benchmark.sh
./rq9_benchmark.sh

# (From container bash) As an example, examine one of the outputs locally
cat output-rq3/incremental_result.csv

# In the host machine, find the container id of sepe-benchmark image
docker ps 
    # An example output is:
        # CONTAINER ID   IMAGE            COMMAND   CREATED         STATUS         PORTS     NAMES
        # 775449ae5ca3   sepe-benchmark   "bash"    9 minutes ago   Up 9 minutes             keen_moser

# In the host machine, copy output from the container id to the host machine
docker cp 775449ae5ca3:/sepe/artifact/output-rq1-rq2 .
docker cp 775449ae5ca3:/sepe/artifact/output-rq3 .
docker cp 775449ae5ca3:/sepe/artifact/output-rq5 .
docker cp 775449ae5ca3:/sepe/artifact/output_rq6 .
docker cp 775449ae5ca3:/sepe/artifact/output-rq7 .
docker cp 775449ae5ca3:/sepe/artifact/output-rq8 .
docker cp 775449ae5ca3:/sepe/artifact/output-rq9 .
docker cp 775449ae5ca3:/sepe/artifact/results .
```

Within the `artifact` branch, we describe how to navigate expected outputs in `sepe/README.md`.

