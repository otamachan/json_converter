ARG ROS_DISTRO=jazzy
FROM ros:${ROS_DISTRO}-ros-base

# Install development tools
RUN apt-get update && apt-get install -y \
    # Build tools
    build-essential \
    cmake \
    git \
    wget \
    curl \
    # Clang tools
    clang \
    clang-format \
    clang-tidy \
    clangd \
    # Python tools
    python3-pip \
    python3-pytest \
    python3-pytest-cov \
    python3-setuptools \
    python3-argcomplete \
    python3-flake8 \
    # ROS2 development tools
    python3-colcon-common-extensions \
    python3-rosdep \
    python3-vcstool \
    ros-${ROS_DISTRO}-ament-clang-format \
    ros-${ROS_DISTRO}-ament-clang-tidy \
    # Test packages
    ros-${ROS_DISTRO}-test-msgs \
    # Libraries
    nlohmann-json3-dev \
    # Debugging tools
    gdb \
    valgrind \
    # Additional utilities
    bash-completion \
    vim \
    nano \
    less \
    htop \
    && rm -rf /var/lib/apt/lists/*

# Install additional Python packages not available via apt
RUN pip3 install --no-cache-dir --break-system-packages \
    black==24.10.0 \
    mypy==1.13.0

# Setup sudo for existing user (UID 1000 already exists in base image)
ARG USERNAME=ros
ARG USER_UID=1000
ARG USER_GID=$USER_UID

RUN apt-get update \
    && apt-get install -y sudo \
    && rm -rf /var/lib/apt/lists/* \
    && EXISTING_USER=$(getent passwd $USER_UID | cut -d: -f1) \
    && if [ -n "$EXISTING_USER" ]; then \
         echo "$EXISTING_USER ALL=(root) NOPASSWD:ALL" > /etc/sudoers.d/$EXISTING_USER; \
         chmod 0440 /etc/sudoers.d/$EXISTING_USER; \
       else \
         groupadd --gid $USER_GID $USERNAME; \
         useradd --uid $USER_UID --gid $USER_GID -m $USERNAME; \
         echo "$USERNAME ALL=(root) NOPASSWD:ALL" > /etc/sudoers.d/$USERNAME; \
         chmod 0440 /etc/sudoers.d/$USERNAME; \
       fi

# Setup workspace
RUN mkdir -p /workspace/src
WORKDIR /workspace

# Change ownership of workspace
RUN EXISTING_USER=$(getent passwd $USER_UID | cut -d: -f1) \
    && chown -R $EXISTING_USER:$(getent group $USER_GID | cut -d: -f1) /workspace

# Switch to non-root user (using UID directly)
USER 1000

# Source ROS2 setup in bashrc
RUN echo "source /opt/ros/${ROS_DISTRO}/setup.bash" >> ~/.bashrc \
    && echo "if [ -f /workspace/install/setup.bash ]; then source /workspace/install/setup.bash; fi" >> ~/.bashrc \
    && echo "export ROS_DOMAIN_ID=0" >> ~/.bashrc

# Initialize rosdep
RUN sudo rosdep init || true \
    && rosdep update

CMD ["/bin/bash"]
