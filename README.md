# Myllfcchat

`Myllfcchat` 是一个基于 `Qt 6` 的桌面即时通讯项目，包含一个 Windows 客户端和一组拆分后的后端服务。后端采用 `C++ + Boost.Asio + gRPC` 为主，配合一个 `Node.js` 验证码服务，围绕登录鉴权、聊天连接分配、好友关系、消息同步、图片/头像资源传输等场景实现了一套完整链路。

从代码现状看，这个仓库更接近一个可运行的课程/练手型 IM 系统，而不是已经产品化的聊天软件：核心的注册、登录、私聊、加好友、图片消息和头像上传已经串起来了，但群聊、通用文件消息、完整个人资料编辑等能力仍有预留代码或半成品痕迹。

## 功能概览

当前代码已经实现或基本实现的能力：

- 邮箱验证码注册。
- 邮箱验证码找回/重置密码。
- 用户登录，并通过 `GateServer` 获取聊天服务与资源服务地址。
- 登录后拉取个人资料、好友列表、好友申请列表、聊天会话列表与历史消息。
- 按用户 ID 或用户名搜索用户。
- 发起好友申请、接收好友申请、通过好友申请。
- 创建私聊会话。
- 文本消息收发。
- 图片消息收发。
- 图片资源上传、下载、进度更新、暂停/继续。
- 用户头像裁剪、本地保存、上传，以及头像懒加载下载。
- 心跳保活。
- 同账号异地登录踢下线。
- 多个聊天节点之间通过 gRPC 转发跨服通知。

代码里已经预留但目前不建议在 README 中宣称“完全可用”的能力：

- 群聊：协议和部分模型有预留，但客户端加载聊天线程时仍主动跳过 `group` 类型。
- 通用文件消息：枚举、资源传输链路和部分代码存在，但聊天页发送普通文件的 UI 流程尚未完成。
- 完整个人资料编辑：设置页已有 `nick/name/desc` 输入框，但当前真正打通的是头像上传。

## 系统架构

### 整体链路

```text
Qt Client
  |
  | HTTP
  v
GateServer
  |-- gRPC --> VarifyServer     (邮箱验证码)
  |-- gRPC --> StatusServer     (分配 ChatServer 节点 + 生成 token)
  |
  | 返回 chat/res 地址与 token
  v
Qt Client
  |-- TCP --> ChatServer / ChatServer2   (登录、好友、文本消息、会话、历史消息)
  |-- TCP --> ResourceServer             (头像、图片、资源上传下载)

ChatServer / ResourceServer / GateServer / StatusServer
  |-- MySQL   (用户、好友、会话、消息)
  |-- Redis   (验证码、token、登录状态、传输状态、缓存)
```

### 模块职责

#### `llfcchat/`

Qt 6 桌面客户端。

- 认证界面：登录、注册、重置密码。
- 主聊天界面：聊天列表、联系人列表、搜索、好友申请、资料页。
- TCP 聊天连接由 `TcpMgr` 管理。
- 资源连接由 `FileTcpMgr` 管理，负责头像/图片上传下载、断点续传、进度回调。

#### `workspace/GateServer/`

HTTP 网关和认证入口。

- 对外暴露：`/get_varifycode`、`/user_register`、`/reset_pwd`、`/user_login`。
- 调用 `VarifyServer` 发送邮箱验证码。
- 校验注册/重置密码所需验证码。
- 校验登录账号密码。
- 通过 `StatusServer` 为用户分配合适的聊天节点，并返回聊天节点地址、资源服务地址和登录 token。

#### `workspace/VarifyServer/`

Node.js gRPC 验证码服务。

- 生成 4 位验证码。
- 将验证码写入 Redis 并设置过期时间。
- 通过 `nodemailer` 发送邮件。

#### `workspace/StatusServer/`

聊天节点调度与 token 分发服务。

- 从配置中读取多个 `ChatServer` 节点。
- 依据 Redis 中维护的登录计数，选择负载更低的节点。
- 为登录用户生成 token 并写入 Redis。

#### `workspace/ChatServer/` 与 `workspace/ChatServer2/`

聊天核心服务，两个目录对应两个聊天节点实例。

- 校验客户端 token，建立登录态。
- 拉取基础资料、好友列表、好友申请列表。
- 搜索用户。
- 发起好友申请、通过好友申请。
- 创建私聊线程。
- 加载聊天线程列表与历史消息。
- 处理文本消息发送。
- 处理图片消息元信息写库。
- 通过 gRPC 把跨节点通知转发给对方所在聊天节点。
- 处理重复登录踢人。

#### `workspace/ResourceServer/`

资源传输服务。

- 头像上传。
- 聊天图片上传。
- 图片下载。
- 基于 Redis 维护资源传输进度，实现续传。
- 资源上传完成后通过 gRPC 通知对应的 `ChatServer`，再由聊天服务通知接收方客户端。

## 目录结构

```text
.
├─ llfcchat/                 Qt 6 客户端
│  ├─ res/                   图标、头像、GIF 等资源
│  ├─ style/                 QSS 样式
│  ├─ static/                客户端静态资源
│  ├─ *.cpp / *.h / *.ui     客户端源码与界面
│  └─ llfcchat.pro           Qt 工程文件
├─ workspace/
│  ├─ GateServer/            HTTP 网关
│  ├─ StatusServer/          聊天节点调度服务
│  ├─ ChatServer/            聊天节点 1
│  ├─ ChatServer2/           聊天节点 2
│  ├─ ResourceServer/        资源传输服务
│  └─ VarifyServer/          Node.js 验证码服务
├─ AGENTS.md
└─ LICENSE
```

## 技术栈

- 客户端：`Qt 6`、`QWidget`、`QNetworkAccessManager`、自定义 TCP 协议。
- 后端 C++：`C++17`、`Boost.Asio`、`gRPC`、`protobuf`、`jsoncpp`。
- 后端 Node.js：`@grpc/grpc-js`、`nodemailer`、`ioredis`。
- 存储：`MySQL`、`Redis`。

## 运行环境

项目明显以 Windows 本地开发为主，默认假设如下：

- Windows。
- Qt 6 + MinGW 构建客户端。
- Visual Studio x64 构建 C++ 服务。
- Node.js 运行 `VarifyServer`。
- 本地可用的 MySQL 与 Redis。

另外需要注意，C++ 服务工程中的 `PropertySheet.props` 写死了本机库路径，例如：

- `Boost`
- `gRPC / protobuf`
- `jsoncpp`
- `MySQL Connector/C++`
- `hiredis`

如果你不是原作者的机器环境，需要先修改这些 `.props` 里的 `IncludePath`、`LibraryPath` 和附加库目录，否则服务端工程通常无法直接编译通过。

## 外部依赖

### MySQL

多个服务的 `config.ini` 默认都指向：

- Host: `127.0.0.1`
- Port: `3306`
- Schema: `llfc`

代码依赖 MySQL 存储用户、好友、聊天线程、消息、头像信息等。

注意：当前仓库里没有看到数据库初始化 SQL 脚本，因此你需要自行准备 `llfc` 数据库结构和初始数据，或从原项目来源补齐建表脚本。

### Redis

默认配置：

- Host: `127.0.0.1`
- Port: `6380`

Redis 用于：

- 邮箱验证码缓存。
- 登录 token。
- 用户在线所在节点。
- 聊天节点登录计数。
- 资源上传/下载进度与续传状态。
- 用户基础信息缓存。

### 邮件服务

`workspace/VarifyServer/config.json` 中包含邮箱账号配置，验证码邮件通过 `nodemailer` 发送。实际部署前应替换为你自己的发信账号，并避免把真实凭据提交到仓库。

## 构建

### 1. 构建客户端

在 Qt MinGW Shell 中进入 `llfcchat/`：

```powershell
qmake llfcchat.pro
mingw32-make
```

构建产物默认输出到：

- `llfcchat/bin/`

客户端会在运行目录下读取 `config.ini`，该文件在 `qmake` 的后处理步骤中会复制到输出目录。

### 2. 构建 C++ 服务

在对应服务目录执行，例如：

```powershell
msbuild GateServer.sln /p:Configuration=Debug /p:Platform=x64
msbuild StatusServer.sln /p:Configuration=Debug /p:Platform=x64
msbuild ChatServer.sln /p:Configuration=Debug /p:Platform=x64
msbuild ChatServer.sln /p:Configuration=Debug /p:Platform=x64
msbuild ResourceServer.sln /p:Configuration=Debug /p:Platform=x64
```

说明：

- `ChatServer/ChatServer.sln` 对应聊天节点 1。
- `ChatServer2/ChatServer.sln` 对应聊天节点 2。
- 两个聊天节点工程名都叫 `ChatServer.sln`，只是目录不同。

如果修改了 `message.proto`，先在服务目录执行：

```powershell
./start.bat
```

用于重新生成 `message.pb.*` 和 `message.grpc.pb.*`。

### 3. 构建并启动验证码服务

进入 `workspace/VarifyServer/`：

```powershell
npm install
npm run serve
```

## 推荐启动顺序

先保证配置文件中的地址、端口、数据库密码、Redis 密码已经对齐。

推荐按下面顺序启动：

1. 启动 `Redis`。
2. 启动 `MySQL`，并确认 `llfc` 库结构已准备好。
3. 启动 `workspace/VarifyServer/`。
4. 启动 `workspace/StatusServer/`。
5. 启动 `workspace/ChatServer/`。
6. 启动 `workspace/ChatServer2/`。
7. 启动 `workspace/ResourceServer/`。
8. 启动 `workspace/GateServer/`。
9. 启动 `llfcchat/bin/llfcchat.exe`。

## 关键配置文件

- 客户端：`llfcchat/config.ini`
  - 指向 `GateServer` 的 HTTP 地址。
- 网关：`workspace/GateServer/config.ini`
  - 配置验证码服务、状态服务、资源服务、MySQL、Redis。
- 聊天节点：`workspace/ChatServer/config.ini`、`workspace/ChatServer2/config.ini`
  - 配置本节点端口、RPC 端口、对端聊天节点、MySQL、Redis。
- 状态服务：`workspace/StatusServer/config.ini`
  - 配置聊天节点列表。
- 资源服务：`workspace/ResourceServer/config.ini`
  - 配置资源输出目录、聊天节点 gRPC 地址、MySQL、Redis。
- 验证码服务：`workspace/VarifyServer/config.json`
  - 配置发件邮箱、MySQL、Redis。

## 客户端交互流程简述

### 登录流程

1. 客户端向 `GateServer` 发送 `/user_login`。
2. `GateServer` 校验账号密码。
3. `GateServer` 向 `StatusServer` 请求一个聊天节点和 token。
4. `GateServer` 把 `ChatServer` 地址、`ResourceServer` 地址、token 返回给客户端。
5. 客户端分别连接聊天服务和资源服务。
6. 客户端携带 `uid + token` 发送聊天登录请求。

### 文本消息流程

1. 客户端向 `ChatServer` 发送文本消息。
2. `ChatServer` 写入 MySQL。
3. 若对方在线且在同节点，直接 TCP 推送。
4. 若对方在其他聊天节点，通过 gRPC 转发，再由目标节点推送。

### 图片消息流程

1. 客户端先向 `ChatServer` 发送图片消息元信息，创建聊天记录。
2. 客户端再向 `ResourceServer` 分片上传图片内容。
3. 资源服务保存文件，并更新数据库里的上传状态。
4. 上传完成后，资源服务通过 gRPC 通知接收方所在聊天节点。
5. 接收方客户端收到图片消息通知后，按需从 `ResourceServer` 下载图片。

## 手工验证建议

仓库没有现成的自动化测试，至少建议做这些手工验证：

- 注册一个新用户，确认能收到验证码邮件。
- 用新用户登录，确认能进入聊天主界面。
- 用两个账号互相搜索并加好友。
- 通过好友申请后确认私聊线程被创建。
- 发送文本消息，确认双方都能收到。
- 发送图片，确认上传进度、对端通知和图片下载可用。
- 修改头像并重新登录，确认头像持久化和拉取正常。
- 使用同一账号双端登录，确认旧会话被踢下线。

## 已知边界与注意事项

- 当前仓库没有数据库初始化脚本，首次搭建的主要门槛在这里。
- 配置文件中含有本地密码和邮箱凭据，分享代码前应先替换为占位值。
- `workspace/ResourceServer/bin/static/` 下已有运行期资源文件，这些更像本地测试数据，不应视为源码的一部分。
- C++ 服务对本机第三方库目录依赖很重，换机器通常需要手动修工程配置。
- 客户端代码中群聊与普通文件消息存在预留接口，但不能当作完整功能使用。
- 设置页的资料编辑 UI 尚未完整接到后端更新流程。

## 适合的用途

这个项目比较适合作为以下用途：

- 学习 Qt 桌面 IM 客户端的基础组织方式。
- 学习网关、状态服务、聊天节点、资源服务拆分后的后端结构。
- 学习 `Boost.Asio + gRPC + Redis + MySQL` 组合在即时通讯场景中的基本用法。
- 作为二次开发基础，继续补全群聊、文件消息、消息已读、离线消息、资料编辑等能力。

