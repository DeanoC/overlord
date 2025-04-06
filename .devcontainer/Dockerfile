FROM mcr.microsoft.com/vscode/devcontainers/java:17

# Install required packages
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install curl wget git zip unzip python3 python3-pip

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

# Clean up
RUN apt-get autoremove -y \
    && apt-get clean -y \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /workspaces/overlord