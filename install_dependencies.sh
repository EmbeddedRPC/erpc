#!/bin/bash

# exit when any command fails
set -e

unameOut="$(uname -s)"
case "${unameOut}" in
Linux*)
    echo "Linux os detected. Installing dependencies."
    sudo apt-get update -qq
    sudo apt-get install python3 bison flex libboost-dev libboost-filesystem-dev libboost-system-dev
    if [ "$1" = "clang" ]; then
        echo "Installing clang compiler."
        sudo apt-get install clang
    else
        echo "Installing default gnu compiler."
        sudo apt-get install gcc g++
    fi
    sudo pip3 install -U pytest pyyaml
    ;;
Darwin*)
    echo "Mac os detected. Installing dependencies."
    brew update
    brew install python3 boost bison flex -v -f 2>&1
    sudo pip3 install tornado
    sudo pip3 install --user nose
    sudo pip3 install pytest --upgrade --ignore-installed six
    sudo pip3 install pyyaml
    ;;
*)
    echo "Unknown or currently unsupported os: ${unameOut}"
    ;;
esac
