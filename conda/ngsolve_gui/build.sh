#!/bin/bash
mkdir build
cd build

cmake -G "Unix Makefiles" \
  -DCMAKE_BUILD_TYPE=RELEASE \
  -DCMAKE_PREFIX_PATH=${PREFIX} \
  -DCMAKE_SYSTEM_PREFIX_PATH=${SYS_PREFIX} \
  ${SRC_DIR}

make -j$CPU_COUNT
make install

