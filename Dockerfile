FROM ghcr.io/aristotelis2002/uni-cool

USER root
WORKDIR /workspace/cool-compiler
COPY . .
RUN bash tools/build.sh

USER student
ENTRYPOINT ["./build/parser"]
