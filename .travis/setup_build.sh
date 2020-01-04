#!/bin/sh

if [[ "$OSTYPE" == "darwin"* ]]; then
./setup --pre-ci --coverage=$COVERAGE
else
sudo chroot --userspec travis:travis $HOME/$ARCH /bin/sh -c "cd /cascade && ./setup --pre-ci --coverage=$COVERAGE"
fi
