#!/bin/bash
mkdir -p ~/.conan && cp .travis/conan_settings.yml ~/.conan/settings.yml

if [[ "$OSTYPE" == "darwin"* ]]; then
./setup --silent --ci --coverage=$COVERAGE
else

if [[ "$COVERAGE" == 1 ]]; then
sudo chroot $HOME/$ARCH /bin/bash -c "apt-get install -y lcov"
fi

sudo chroot --userspec travis:travis $HOME/$ARCH /bin/bash -c "cd /cascade && ./setup --silent --ci --coverage=$COVERAGE"

if [[ "$COVERAGE" == 1 ]]; then
curl -L https://codecov.io/bash -o codecov.sh
bash codecov.sh -x gcov 2>&1 | grep -v 'has arcs to entry block' | grep -v 'has arcs from exit block' 

# Move on if we can't submit coverage
cd .. && sonar-scanner -Dsonar.projectKey=cascade -Dsonar.organization=cascade -Dsonar.sources=lib,src,tools -Dsonar.tests=test -Dsonar.cfamily.build-wrapper-output=build/bw-output -Dsonar.cfamily.gcov.reportsPath=build -Dsonar.host.url=https://sonarcloud.io -Dsonar.login=$SONAR_TOKEN -Dsonar.exclusions=cascade_coverage/*,**/cascade_coverage/*,**/verilog_parser.cc,**/verilog_parser.hh,**/verilog_lexer.cc -Dsonar.cfamily.threads=$(getconf _NPROCESSORS_ONLN) || true
fi
fi