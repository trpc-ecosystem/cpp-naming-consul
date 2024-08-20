# Consul conducted services Demo

This example demonstrates how to use consul as Service Registry and Discovery Center. The example includes a simple client and server program.

## Prerequisites
1. Consul in local host, use default port 8500. See https://developer.hashicorp.com/consul/docs/install.
2. Bazel

## File Structure

```shell
$ tree examples
examples
├── client
│   ├── BUILD
│   ├── client.cc
│   └── trpc_cpp_fiber.yaml
├── README.md
└── server
    ├── BUILD
    ├── greeter_service.cc
    ├── greeter_service.h
    ├── helloworld.proto
    ├── helloworld_server.cc
    └── trpc_cpp_fiber.yaml
```

## Service Config

* Server

  Config Consul address in plugins.registry section, like this:

  ```shell
  plugins:
      registry:
          consul:
            address: 127.0.0.1:8500
  ```

* Client

  Config Consul address in plugins.selector section, like this:

  ```shell
  plugins:
      selector:
          consul:
            address: 127.0.0.1:8500
  ```

## Running the Example

* Build and run server and client program

  ```shell
  cd ../
  ./run_example.sh
  ```

* Compilation

  We can run the following command to compile the demo.

  ```shell
  bazel build //examples/...
  ```

*  Run the server program.

  We can run the following command to start the server program.

  ```shell
  ./bazel-bin/examples/server/helloworld_server --config=examples/server/trpc_cpp_fiber.yaml
  ```

* Run the client program

  We can run the following command to start the client program.

  ```shell
  ./bazel-bin/examples/client/client --config=examples/client/trpc_cpp_fiber.yaml
  ```
 
