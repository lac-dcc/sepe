# Commands to run the artifact in a Docker container
# $ docker build -t sepe-benchmark .
# $ docker run --rm sepe-benchmark

# Start with an official Ubuntu image
FROM ubuntu:latest

# Set environment variables to avoid interactive prompts during installation
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && \
    apt-get install -y \
    build-essential \
    zip \
    unzip \
    git \
    python3 \
    python3-pip \
    curl \
    wget \
    cmake \
    && rm -rf /var/lib/apt/lists/*

# Install Python packages
RUN pip3 install --break-system-packages numpy pandas matplotlib scipy 

# Install Rust and Cargo
RUN curl https://sh.rustup.rs -sSf | sh -s -- -y && . "$HOME/.cargo/env"
ENV PATH="/root/.cargo/bin:${PATH}"

# Clone the repository
RUN git clone --branch artifact https://github.com/lac-dcc/sepe /sepe

# Set the working directory to the artifact directory
WORKDIR /sepe/artifact

# Install abseil deps
RUN ../scripts/install_abseil.sh

# Build sepe-runner in the repository's root directory
RUN make -j"$(nproc)" -C ../
RUN make -j"$(nproc)" -C ../ benchmark

# Run the benchmark script as the container's default command
CMD ["bash", "rq1_rq2_benchmark.sh"]
CMD ["bash", "rq3_benchmark.sh"]
CMD ["bash", "rq5_benchmark.sh"]
CMD ["bash", "rq6_benchmark.sh"]
CMD ["bash", "rq7_benchmark.sh"]
CMD ["bash", "rq8_benchmark.sh"]
CMD ["bash", "rq9_benchmark.sh"]
