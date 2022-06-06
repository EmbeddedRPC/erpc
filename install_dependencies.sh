#!/bin/bash

unameOut="$(uname -s)"
case "${unameOut}" in
Linux*)
    echo "Linux os detected. Installing dependencies."
    sudo apt-get update -qq
    sudo apt-get install python3 bison flex libboost-dev libboost-filesystem-dev libboost-system-dev
    if [ "$1"="clang" ]; then
        echo "Installing clang compiler."
        sudo apt-get install clang
    else
        echo "Installing default gnu compiler."
        sudo apt-get install gcc g++
    fi
    sudo pip3 install pytest pyyaml
    ;;
Darwin*)
    echo "Mac os detected. Installing dependencies."
    brew update
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
