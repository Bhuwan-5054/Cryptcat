FROM ubuntu:22.04 AS builder

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    libssl-dev \
    git \
    valgrind \
    gdb \
    lcov \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

# Copy source
COPY . .

# Build
RUN mkdir -p build && cd build && \
    cmake .. && \
    make

# Runtime stage
FROM ubuntu:22.04 AS runtime

RUN apt-get update && apt-get install -y \
    libssl3 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

COPY --from=builder /workspace/build/cryptcat /usr/local/bin/

ENTRYPOINT ["cryptcat"]
CMD ["--help"]
