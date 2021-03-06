#!/bin/bash

set -e # exit on error

# "DEPENDECIES ========================================================================"

export INSTALL_PREFIX="/usr/local/"
export PATH=$INSTALL_PREFIX/bin:$PATH

cd $ROOT_PATH

# travis bug: https://github.com/travis-ci/travis-ci/issues/6307
# rvm get head || true

brew update || echo "suppress failures in order to ignore warnings"		
brew tap homebrew/versions || echo "suppress failures in order to ignore warnings"

# eigen3 needs gfortran
brew install gcc || echo "suppress failures in order to ignore warnings"

# Cmake
brew install cmake || echo "suppress failures in order to ignore warnings"

echo "Path set to ${PATH}"
echo "CXX set to ${CXX}"
echo "CC set to ${CC}"

${CXX} --version
cmake --version

chmod +x $CHECKOUT_PATH/travis/install_dep.sh
. $CHECKOUT_PATH/travis/install_dep.sh

# "DEPENDECIES COMPLETE ================================================================="

set +e