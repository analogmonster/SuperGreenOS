# Use the standard Ubuntu 20.04 image
FROM ubuntu:20.04

# Avoid interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Set up UTF-8 locale and install needed packages as root
RUN apt-get update && \
    apt-get install -y sudo bash locales && \
    locale-gen en_US.UTF-8 && \
    update-locale LANG=en_US.UTF-8

# Install system dependencies and development tools (non-ESP-IDF related)
RUN apt-get update && apt-get install -y \
    apt-utils \
    git \
    wget \
    curl \
    make \
    libffi-dev \
    libssl-dev \
    libncurses-dev \
    flex \
    bison \
    gperf \
    cmake \
    ninja-build \
    ccache \
    libsodium-dev \
    libusb-1.0-0 \
	usbutils \
	pkg-config \
	xz-utils \
    && apt-get clean

# Install Python 2.7 and required dependencies
RUN apt-get update && apt-get install -y \
    python2.7 \
    libpython2.7 \
    curl \
    && apt-get clean

# Set python2.7 as the default
RUN update-alternatives --install /usr/bin/python python /usr/bin/python2.7 1

# Manually install pip for Python 2.7
RUN curl -sS https://bootstrap.pypa.io/pip/2.7/get-pip.py -o get-pip.py && \
    python2.7 get-pip.py && \
    rm get-pip.py

# Fix pip for Python 2.7 and install virtualenv
RUN python2.7 -m pip install --upgrade "pip==20.3.4" && \
    pip install virtualenv==16.7.10

# Set UTF-8 encoding to prevent UnicodeDecodeError during tool installation
ENV PYTHONIOENCODING=utf-8

# Install Python packages required by ESP-IDF v3.3.1 (June 2020 snapshot)
RUN python2.7 -m pip install --upgrade pip==20.3.4 && \
    python2.7 -m pip install --user \
      pyserial==3.4 \
      future==0.18.2 \
      cryptography==2.9.2 \
      pyparsing==2.4.7 \
      pyelftools==0.26 \
      gdbgui==0.13.2.0 \
      pygdbmi==0.9.0.3 \
      brotli==1.0.7 \
      bidict==0.18.4

# Install CUE v0.0.8
WORKDIR /tmp
RUN wget https://github.com/cue-lang/cue/releases/download/v0.0.8/cue_0.0.8_Linux_x86_64.tar.gz && \
    tar -xzf cue_0.0.8_Linux_x86_64.tar.gz && \
    mv ./cue /usr/local/bin/ && \
    rm -rf /tmp/cue_0.0.8_Linux_x86_64.tar.gz

# Install mkspiffs v0.2.3
WORKDIR /tmp
RUN wget https://github.com/igrr/mkspiffs/releases/download/0.2.3/mkspiffs-0.2.3-arduino-esp32-linux64.tar.gz && \
    tar -xzf mkspiffs-0.2.3-arduino-esp32-linux64.tar.gz && \
    mv mkspiffs-0.2.3-arduino-esp32-linux64/mkspiffs /usr/local/bin/mkspiffs && \
    chmod +x /usr/local/bin/mkspiffs && \
    rm -rf mkspiffs-0.2.3-arduino-esp32-linux64*

# Install Node.js v12.18.0
WORKDIR /tmp
RUN wget https://nodejs.org/dist/v12.18.0/node-v12.18.0-linux-x64.tar.xz && \
    tar -xf node-v12.18.0-linux-x64.tar.xz && \
    cp -r node-v12.18.0-linux-x64/bin /usr/local/ && \
    cp -r node-v12.18.0-linux-x64/include /usr/local/ && \
    cp -r node-v12.18.0-linux-x64/lib /usr/local/ && \
    cp -r node-v12.18.0-linux-x64/share /usr/local/ && \
    rm -rf node-v12.18.0-linux-x64*

# Install ejs-cli v2.1.0 (June 2020 version)
# Consider switching to v2.2.0 (released August 2020) if compatibility allows
RUN npm install -g ejs-cli@2.1.0

# --- ESP-IDF Setup ---
# We install a known working version of ESP-IDF (commit from June 2020).
# WARNING: Do NOT run the install of requirements.txt automatically here —
# we manually specify and version those Python dependencies instead.
# Reference: https://docs.espressif.com/projects/esp-idf/en/v3.3.2/get-started/index.html

# ESP-IDF v3.3.1 setup (do not source export.sh during build)
ENV IDF_PATH=/opt/esp-idf_release_3.3.1

# Clone, checkout specific commit, fix permissions and safe.directory (as root)
WORKDIR /opt
RUN git clone --recursive https://github.com/espressif/esp-idf.git ${IDF_PATH} && \
    cd ${IDF_PATH} && \
    git checkout 143d26aa49df524e10fb8e41a71d12e731b9b71d && \
    git config --system --add safe.directory ${IDF_PATH} && \
    chmod -R a+rx ${IDF_PATH}

# Create user and allow passwordless sudo
RUN useradd -ms /bin/bash devuser && \
    echo 'devuser:devuser' | chpasswd && \
    usermod -aG sudo devuser && \
    echo 'devuser ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers

# Switch to devuser and install ESP-IDF tools to their home
USER devuser
RUN /opt/esp-idf_release_3.3.1/install.sh esp32

# Add IDF paths to devuser's .bashrc
RUN echo "export IDF_PATH=${IDF_PATH}" >> /home/devuser/.bashrc && \
    echo '. ${IDF_PATH}/export.sh' >> /home/devuser/.bashrc

# Switch back to root and fix permissions
USER root
RUN chown -R devuser:devuser /home/devuser/.espressif

# Set locale and environment
ENV LANG=en_US.UTF-8
ENV LANGUAGE=en_US:en
ENV LC_ALL=en_US.UTF-8

# Switch to devuser
USER devuser

# Set working directory
WORKDIR /workspace
