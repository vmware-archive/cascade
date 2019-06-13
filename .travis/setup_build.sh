#!/bin/bash
mkdir -p ~/.conan && cp .travis/conan_settings.yml ~/.conan/settings.yml

if [[ "$OSTYPE" == "darwin"* ]]; then
./setup --pre-ci --coverage=$COVERAGE
else
sudo chroot --userspec travis:travis $HOME/$ARCH /bin/bash -c "cd /cascade && ./setup --pre-ci --coverage=$COVERAGE"
fi
