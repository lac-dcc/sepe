 Reproducing Sepe Results via Docker

This folder contains a [Dockerfile](Dockerfile) to reproduce all the experiments in the Sepe artifact.
To create a docker image out of this docker file, you can run the following commands:

```bash
# Clone the Sepe repository artifact branch:
#
git clone --branch artifact https://github.com/lac-dcc/sepe /sepe
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
```
Within the `artifact` branch See the expected results in `sepe/README.md`.
