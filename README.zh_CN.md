# 前言
[Consul](https://developer.hashicorp.com/consul) 是一种服务网络解决方案，使团队能够在服务之间以及跨多云环境和运行时管理安全的网络连接。Consul提供服务发现、基于身份的授权、L7流量管理和服务之间的加密。为了方便用户对接Consul，我们提供了Consul名字服务插件。

# 使用说明
详细的使用例子可以参考: [Consul examples](./examples)。

## 引入依赖
### Bazel
在项目的`WORKSPACE`文件中，引入`cpp-naming-consul`仓库及其依赖：
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

另外，由于本插件依赖curl库，需要确保系统已经安装过curl，库路径为/usr/lib64/libcurl.so，头文件位置为/usr/include
### cmake
暂不支持

## 注册插件
1. 对于服务端场景，用户需要重载`TrpcApp::RegisterPlugins`函数，并在其中进行注册，以HelloworldServer服务为例：

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
2. 对于纯客户端场景，需要在启动框架配置初始化后，框架其他模块启动前注册：
```
int main(int argc, char* argv[]) {
  ParseClientConfig(argc, argv);

  // register consul selector plugin
  ::trpc::consul::selector::Init();

  return ::trpc::RunInTrpcRuntime([]() { return Run(); });
}
```

Note: 用户可根据使用情况配置和注册selector插件和registry插件，如果不用服务注册功能的话则不需要注册registry插件，不用路由选择功能的话则不需要注册selector插件。

## 插件配置
使用Consul插件时，必须在框架配置文件中加上相应的插件配置：
```yaml
plugins:
  registry: #服务注册插件
    consul:
      address: 127.0.0.1:8500  #consul服务地址
  selector:  #路由选择插件
    consul:
      address: 127.0.0.1:8500  #consul服务地址
```

## 使用consul selector插件进行服务路由
在正确配置和注册插件后，就可以通过指定serviceproxy的`selector_name: consul`配置项，从而让框架自动使用consul selector插件进行路由。

## 功能支持情况

当前consul名字服务Registry情况，注册已支持，心跳上报暂未支持。

当前consul名字服务Selector情况，支持SelectorPolicy::ONE随机路由、SelectorPolicy::MULTIPLE以及SelectorPolicy::ALL, 其他路由策略及熔断等暂未支持。

## 注意事项
在使用 cpp-naming-consul 插件之前，你需要确保已正确安装并配置了 consul。具体说明详见[https://developer.hashicorp.com/consul](https://developer.hashicorp.com/consul)

# 关于单元测试环境
运行本仓库下的单元测试前，需要先搭建consul环境：

1. 安装consul

  以二进制安装方式为例：

  ```
  wget https://releases.hashicorp.com/consul/1.19.0/consul_1.19.0_linux_amd64.zip -O /tmp/consul_1.19.0_linux_amd64.zip
  unzip /tmp/consul_1.19.0_linux_amd64.zip -d /tmp
  rm /tmp/LICENSE.txt
  mv /tmp/consul /usr/bin
  ```

2. 配置及启动consul

  以开发模式为例，启动consul agent

  ```
  consul agent -dev  -config-dir=./testing/consul.d/ &
  ``` 
