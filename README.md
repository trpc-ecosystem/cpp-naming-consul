[中文](./README.zh_CN.md)

[TOC]

# Overview
[Consul](https://developer.hashicorp.com/consul) is a service networking solution that enables teams to manage secure network connectivity between services and across multi-cloud environments and runtimes. Consul offers service discovery, identity-based authorization, L7 traffic management, and service-to-service encryption. To facilitate users in integrating with Consul, we provide the Consul Name Service plugin.

# Usage
For detailed usage examples, please refer to [Consul examples](./examples).

## Dependency
### Bazel
In the WORKSPACE file of the project, import the cpp-naming-consul repository and its dependencies:
```
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

git_repository(
    name = "trpc_cpp",
    remote = "https://github.com/trpc-group/trpc-cpp.git",
    branch = "main",
)

load("@trpc_cpp//trpc:workspace.bzl", "trpc_workspace")
trpc_workspace()

git_repository(
    name = "cpp-naming-consul",
    remote = "https://github.com/trpc-ecosystem/cpp-naming-consul.git",
    branch = "main",
)

load("@cpp-naming-consul//trpc:workspace.bzl", "naming_consul_workspace")
naming_consul_workspace()
```

Additionally, since this plugin relies on the curl library, please ensure that curl is already installed on the system. The library path is '/usr/lib64/libcurl.so', and the header files are located in '/usr/include'.
### cmake
Not supported yet.

## Plugin registration
1. For the server-side scenario, you need to register in the TrpcApp::RegisterPlugins function during service startup. Taking HelloworldServer as example:

```
#include "trpc/naming/consul/consul_registry_api.h"
#include "trpc/naming/consul/consul_selector_api.h"

class HelloworldServer : public ::trpc::TrpcApp {
 public:
  ...
  int RegisterPlugins() override {
    // register consul selector plugin
    ::trpc::consul::selector::Init();
    // register consul registry plugin
    ::trpc::consul::registry::Init();

    return 0;
  }
};
```
2. For the pure client-side scenario, you need to register after the framework configuration initialization and before the start of other framework modules:
```
int main(int argc, char* argv[]) {
  ParseClientConfig(argc, argv);

  // register consul selector plugin
  ::trpc::consul::selector::Init();

  return ::trpc::RunInTrpcRuntime([]() { return Run(); });
}
```

Note: Users can configure and register the selector plugin and registry plugin according to their usage scenarios. If the service registration functionality is not required, there is no need to register the registry plugin. Similarly, if the routing selection functionality is not needed, there is no need to register the selector plugin.

## Plugin Configuration

When using the Consul plugin, it is necessary to add the corresponding plugin configuration in the framework configuration file.

```yaml
plugins:
  registry: #registry plugin
    consul:
      address: 127.0.0.1:8500  #address of consul service
  selector:  #selector plugin
    consul:
      address: 127.0.0.1:8500  #address of consul service
```

## Using the Consul selector plugin for service routing

After correctly configuring and registering the plugins, you can specify the `selector_name: consul` configuration option in the serviceproxy. And the framework will automatically use the Consul selector plugin for routing.

## Support features

About consul registry plugin, service registration is supported, heartbeat reporting is not yet supported.

About consul selector plugin, routing policies of SelectorPolicy::ONE random route, SelectorPolicy::MULTIPLE, SelectorPolicy::ALL are supported. Other routing policies and circuit breaking are not yet supported.

## Precautions
Before using the cpp-naming-consul plugin, make sure you have installed and configured Consul correctly. For detailed instructions, please refer to [https://developer.hashicorp.com/consul](https://developer.hashicorp.com/consul).

# About Unit Testing Environment 
Before running the unit tests in this repository, it is necessary to set up a consul environment:

1. Install consul

  Here we use binary installation method:

  ```
  wget https://releases.hashicorp.com/consul/1.19.0/consul_1.19.0_linux_amd64.zip -O /tmp/consul_1.19.0_linux_amd64.zip
  unzip /tmp/consul_1.19.0_linux_amd64.zip -d /tmp
  rm /tmp/LICENSE.txt
  mv /tmp/consul /usr/bin
  ```

2. Config and start consul

  Here we use dev model:

  ```
  consul agent -dev  -config-dir=./trpc/naming/consul/testing/consul.d/ &
  ``` 
