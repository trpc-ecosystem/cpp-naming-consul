#!/bin/bash
bazel build //examples/...

./bazel-bin/examples/server/helloworld_server --config=examples/server/trpc_cpp_fiber.yaml &
sleep 3
./bazel-bin/examples/client/client --config=examples/client/trpc_cpp_fiber.yaml
if [ $? -ne 0 ]; then
    echo "run client error"
    exit -1
fi

sleep 5

set -x
set +o posix
server_pid=`pidof helloworld_server`
kill '-SIGUSR2' "${server_pid}"
if [ $? -ne 0 ]; then
  echo "helloworld_server exit error"
  exit -1
fi
set -o posix
