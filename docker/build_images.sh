#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"

echo "Building docker images..."
echo "Using root context: ${DIR}/.."

echo "Building cascade-builder..."
docker build --rm -f "${DIR}/dockerfiles/builder/Dockerfile" -t cascade-builder:latest ${DIR}/..

echo "Building cascade..."
docker build --rm -f "${DIR}/dockerfiles/cascade/Dockerfile" -t cascade:latest ${DIR}/..