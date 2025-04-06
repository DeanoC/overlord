FROM mcr.microsoft.com/vscode/devcontainers/java:17

# Install required packages
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install curl wget git zip unzip python3 python3-pip \
    # Additional packages that might be needed for overlord
    build-essential libffi-dev pkg-config

# Install Scala
ARG SCALA_VERSION=3.3.1
RUN wget https://github.com/lampepfl/dotty/releases/download/${SCALA_VERSION}/scala3-${SCALA_VERSION}.tar.gz \
    && tar -xzf scala3-${SCALA_VERSION}.tar.gz -C /usr/local \
    && ln -s /usr/local/scala3-${SCALA_VERSION}/bin/scala /usr/local/bin/scala \
    && ln -s /usr/local/scala3-${SCALA_VERSION}/bin/scalac /usr/local/bin/scalac \
    && rm scala3-${SCALA_VERSION}.tar.gz

# Install SBT
ARG SBT_VERSION=1.9.6
RUN curl -L -o sbt-${SBT_VERSION}.deb https://repo.scala-sbt.org/scalasbt/debian/sbt-${SBT_VERSION}.deb \
    && dpkg -i sbt-${SBT_VERSION}.deb \
    && rm sbt-${SBT_VERSION}.deb \
    && apt-get update \
    && apt-get install -y sbt

# Install overlord if specified
ARG INSTALL_OVERLORD=false
RUN if [ "$INSTALL_OVERLORD" = "true" ]; then \
    mkdir -p /tmp/overlord-install && \
    cd /tmp/overlord-install && \
    # Create a setup script that will be executed as part of container startup
    echo '#!/bin/bash\n\
# Build overlord if not already built\n\
if [ ! -f /workspaces/overlord/target/universal/stage/bin/overlord ]; then\n\
    echo "Building overlord..."\n\
    cd /workspaces/overlord && sbt "stage; universal:packageBin"\n\
fi\n\
# Create symbolic link to the overlord command\n\
if [ -f /workspaces/overlord/target/universal/stage/bin/overlord ]; then\n\
    echo "Creating symlink to built overlord..."\n\
    sudo ln -sf /workspaces/overlord/target/universal/stage/bin/overlord /usr/local/bin/overlord\n\
    chmod +x /workspaces/overlord/target/universal/stage/bin/overlord\n\
fi\n\
# Fix permission for run.sh if it exists\n\
if [ -f /workspaces/overlord/overlord_template_project/run.sh ]; then\n\
    chmod +x /workspaces/overlord/overlord_template_project/run.sh\n\
fi\n\
# Fix permission for clean.sh if it exists\n\
if [ -f /workspaces/overlord/overlord_template_project/clean.sh ]; then\n\
    chmod +x /workspaces/overlord/overlord_template_project/clean.sh\n\
fi\n'\
    > /usr/local/bin/setup-overlord.sh && \
    chmod +x /usr/local/bin/setup-overlord.sh && \
    echo "/usr/local/bin/setup-overlord.sh" >> /home/vscode/.bashrc; \
    fi

# Clean up
RUN apt-get autoremove -y \
    && apt-get clean -y \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /workspaces/overlord