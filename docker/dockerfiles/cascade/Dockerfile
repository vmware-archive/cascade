FROM cascade-builder:latest AS cascade-builder

WORKDIR /cascade
COPY . /cascade

# The type of build
ARG BUILD_TYPE=RELWITHDEBINFO

RUN mkdir build && \
     cd build && \
      cmake -DCMAKE_C_COMPILER=/usr/bin/clang-8 -DCMAKE_CXX_COMPILER=/usr/bin/clang++-8 -DCMAKE_BUILD_TYPE=${BUILD_TYPE} .. && \ 
      CTEST_OUTPUT_ON_FAILURE=true make -j$(nproc) all test && \
      make install

CMD [ "cascade" ]