# Use Ubuntu 20.04 image from release date of ESP-IDF v3.3.5
FROM ubuntu:focal-20210217

# Avoid interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Set up UTF-8 locale and install needed packages as root
RUN apt-get update && \
    apt-get install -y sudo bash locales && \
    locale-gen en_US.UTF-8 && \
    update-locale LANG=en_US.UTF-8

# Install system dependencies and development tools
RUN apt-get install -y \
    apt-utils \
    bison \
    build-essential \
    ccache \
    cmake \
    curl \
    flex \
    git \
    gperf \
    libbz2-dev \
    libffi-dev \
    libncurses-dev \
    libreadline-dev \
    libssl-dev \
    libsodium-dev \
    libusb-1.0-0 \
    make \
    ninja-build \
    pkg-config \
    python3 \
    python3-dev \
    python3-pip \
    python3-venv \
    usbutils \
    wget \
    xz-utils \
    zlib1g-dev \
    && apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Install CUE v0.0.8
WORKDIR /tmp
RUN wget https://github.com/cue-lang/cue/releases/download/v0.0.8/cue_0.0.8_Linux_x86_64.tar.gz && \
    tar -xzf cue_0.0.8_Linux_x86_64.tar.gz && \
    mv ./cue /usr/local/bin/ && \
    rm -rf /tmp/cue_0.0.8_Linux_x86_64.tar.gz

# Build and install mkspiffs v0.2.3 from source using the generic config
WORKDIR /tmp
RUN git clone --branch 0.2.3 https://github.com/igrr/mkspiffs.git && \
cd mkspiffs && \
git submodule update --init && \
make dist BUILD_CONFIG_NAME=generic && \
mv mkspiffs /usr/local/bin/mkspiffs && \
chmod +x /usr/local/bin/mkspiffs && \
cd /tmp && rm -rf mkspiffs

# Install Node.js v14.15.4
WORKDIR /tmp
RUN wget https://nodejs.org/dist/v14.15.4/node-v14.15.4-linux-x64.tar.xz && \
    tar -xf node-v14.15.4-linux-x64.tar.xz && \
    cp -r node-v14.15.4-linux-x64/bin /usr/local/ && \
    cp -r node-v14.15.4-linux-x64/include /usr/local/ && \
    cp -r node-v14.15.4-linux-x64/lib /usr/local/ && \
    cp -r node-v14.15.4-linux-x64/share /usr/local/ && \
    rm -rf node-v14.15.4-linux-x64*

# Install ejs-cli v2.2.1 (Dec 2020 version)
RUN npm install -g ejs-cli@2.2.1

# ESP-IDF v3.3.5 setup (do not source export.sh during build)
ENV IDF_PATH=/opt/esp-idf_v3.3.5
WORKDIR /opt
RUN git clone -b v3.3.5 --recursive https://github.com/espressif/esp-idf.git ${IDF_PATH} && \
    git config --system --add safe.directory ${IDF_PATH} && \
    chmod -R a+rx ${IDF_PATH}

# Install ESP-IDF Python dependencies globally
RUN python3 -m pip install -r ${IDF_PATH}/requirements.txt

# Set Python 3 as the default python
RUN update-alternatives --install /usr/bin/python python /usr/bin/python3 10

# Synchronize the container's system clock to host
RUN apt-get update && apt-get install -y ntpdate && ntpdate ntp.ubuntu.com

# Create user and allow passwordless sudo
RUN useradd -ms /bin/bash devuser && \
    echo 'devuser:devuser' | chpasswd && \
    usermod -aG sudo devuser && \
    echo 'devuser ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers

# Switch to devuser and install ESP-IDF tools to their home
USER devuser
RUN ${IDF_PATH}/install.sh esp32

# Switch back to root and fix permissions
USER root
RUN chown -R devuser:devuser /home/devuser/.espressif

# Add IDF paths to devuser's .bashrc
RUN echo "export IDF_PATH=${IDF_PATH}" >> /home/devuser/.bashrc && \
    echo '. ${IDF_PATH}/export.sh' >> /home/devuser/.bashrc
RUN chown -R devuser:devuser ${IDF_PATH}

# Set locale and environment
ENV LANG=en_US.UTF-8
ENV LANGUAGE=en_US:en
ENV LC_ALL=en_US.UTF-8

# Switch to devuser
USER devuser

# Set working directory
WORKDIR /workspace