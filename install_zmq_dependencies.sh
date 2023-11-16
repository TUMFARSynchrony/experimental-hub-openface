#!/bin/sh
#==============================================================================
# Title: install_zmq_dependencies.sh
# Description: Install ZeroMQ dependencies for OpenFace in the experimental-hub. 
# Will install libzmq and cppzmq by following the first 2 instructions at 
# https://github.com/zeromq/cppzmq#build-instructions, 
# only use if you do not have the dependencies already installed for ZeroMQ.
# Author: Aykut Aykut <aykutaaykut.ai@gmail.com>
# Date: 20231116
# Usage: sh install_zmq_dependencies.sh
#==============================================================================

# Exit script if any command fails
set -e 
set -o pipefail

ZMQ_DIR="./zmq_dependencies"

mkdir "$ZMQ_DIR"
cd "$ZMQ_DIR"

#==============================================================================
# Install libzmq
#==============================================================================
LIBZMQ_DIR="./libzmq-master"

if ! [ -d "$LIBZMQ_DIR" ]
then
  wget https://github.com/zeromq/libzmq/archive/refs/heads/master.zip
  if [ $? -ne 0 ]
  then
    echo "Can't find libzmq. Please follow these steps:"
    echo " - Download the zip of libzmq from https://github.com/zeromq/libzmq inside ${ZMQ_DIR}"
    echo " - Unzip the zip file downloaded and delete the zip file"
    echo " - Re-run this script"
    exit 1
  fi

  unzip master.zip
  rm master.zip
fi

cd "$LIBZMQ_DIR"
if [ $? -ne 0 ]
then
  echo "Can't navigate to ${LIBZMQ_DIR}. Please check if ${LIBZMQ_DIR} is available in the directory."
  exit 1
fi

if [ -d "build" ]
then
  rm -rf build
fi

mkdir build
cd build

cmake ..
if [ $? -ne 0 ]
then
  echo "CMAKE error in libzmq build. Please search for the error message."
  exit 1
fi

sudo make -j4 install
if [ $? -ne 0 ]
then
  echo "MAKE error in libzmq install. Please search for the error message."
  exit 1
fi

cd ../../

#==============================================================================
# Install cppzmq
#==============================================================================
CPPZMQ_DIR="./cppzmq-master"

if ! [ -d "$CPPZMQ_DIR" ]
then
  wget https://github.com/zeromq/cppzmq/archive/refs/heads/master.zip
  if [ $? -ne 0 ]
  then
    echo "Can't find cppzmq. Please follow these steps:"
    echo " - Download the zip of cppzmq from https://github.com/zeromq/cppzmq inside ${ZMQ_DIR}"
    echo " - Unzip the zip file downloaded and delete the zip file"
    echo " - Re-run this script"
    exit 1
  fi

  unzip master.zip
  rm master.zip
fi

cd cppzmq-master
if [ $? -ne 0 ]
then
  echo "Can't navigate to ${CPPZMQ_DIR}. Please check if ${CPPZMQ_DIR} is available in the directory."
  exit 1
fi

if [ -d "build" ]
then
  rm -rf build
fi

mkdir build
cd build

cmake ..
if [ $? -ne 0 ]
then
  echo "CMAKE error in cppzmq build. Please search for the error message."
  exit 1
fi

sudo make -j4 install
if [ $? -ne 0 ]
then
  echo "MAKE error in cppzmq install. Please search for the error message."
  exit 1
fi

cd ../../../
