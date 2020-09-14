#!/usr/bin/env bash
g++ -O3 -Wall -shared -std=c++11 -undefined dynamic_lookup `python3 -m pybind11 --includes` affilbind.cpp map_aggregate.cpp -o affilbind`python3.7-config --extension-suffix`