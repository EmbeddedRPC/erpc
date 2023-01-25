#!/bin/bash

# exit when any command fails
set -e

unameOut="$(uname -s)"
case "${unameOut}" in
Linux*)
    echo "Linux os detected."
    DEBIAN_FRONTEND=noninteractive sudo apt-get update -qq --yes
    echo "Installing dependencies."
    DEBIAN_FRONTEND=noninteractive sudo apt-get install python3 bison flex libboost-dev libboost-filesystem-dev libboost-system-dev --yes
    if [ "$1" = "clang" ]; then
        echo "Installing clang compiler."
        DEBIAN_FRONTEND=noninteractive sudo apt-get install clang
    else
        echo "Installing default gnu compiler."
        DEBIAN_FRONTEND=noninteractive sudo apt-get install gcc g++
    fi
    sudo pip3 install -U pytest pyyaml
    sudo apt-get autoremove --yes
    ;;
Darwin*)
    echo "Mac os detected."
    brew update
    echo "Installing dependencies."
    brew install python3 bison flex -v -f 2>&1 && brew upgrade boost || true
    curl "https://bootstrap.pypa.io/pip/2.7/get-pip.py" | sudo python3
    sudo pip install tornado
    sudo pip install --user nose
    sudo pip install pytest --upgrade --ignore-installed six
    ;;
*)
    echo "Unknown or currently unsupported os: ${unameOut}"
    ;;
esac
