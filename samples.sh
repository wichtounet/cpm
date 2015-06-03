#!/bin/bash

mkdir -p results

cp examples/full.cpp examples/full.cpp.bak

export CXX=clang++
export LD=clang++

sed -i '14s/.*/constexpr const double factor = 1.1;/' examples/full.cpp
make -B examples
./debug/bin/full --tag=1 --configuration="-O2"

sed -i '14s/.*/constexpr const double factor = 1.0;/' examples/full.cpp
make -B examples
./debug/bin/full --tag=2 --configuration="-O2"

sed -i '14s/.*/constexpr const double factor = 0.9;/' examples/full.cpp
make -B examples
./debug/bin/full --tag=1 --configuration="-O3"

sed -i '14s/.*/constexpr const double factor = 0.7;/' examples/full.cpp
make -B examples
./debug/bin/full --tag=2 --configuration="-O3"

export CXX=g++
export LD=g++

sed -i '14s/.*/constexpr const double factor = 1.0;/' examples/full.cpp
make -B examples
./debug/bin/full --tag=1 --configuration="-O2"

sed -i '14s/.*/constexpr const double factor = 0.8;/' examples/full.cpp
make -B examples
./debug/bin/full --tag=2 --configuration="-O2"

sed -i '14s/.*/constexpr const double factor = 0.8;/' examples/full.cpp
make -B examples
./debug/bin/full --tag=1 --configuration="-O3"

sed -i '14s/.*/constexpr const double factor = 0.65;/' examples/full.cpp
make -B examples
./debug/bin/full --tag=2 --configuration="-O3"

mv -f examples/full.cpp.bak examples/full.cpp
