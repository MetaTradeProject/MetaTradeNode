# MetaTrade Node
`MetaTrade Gateway`网关服务的本地客户端实现
# 目录

## 安全验证
## Stomp服务
与网关之间的通信交互由`Stomp`协议实现，该项目依赖[WebStomp++](https://github.com/Freesia810/WebStomppp)，在此库的基础上进行了封装，整体上由`MetaTradeClient`提供服务
### MetaTrade Client
- `MetaTradeClient(address)`：构造函数，`address`标识客户端的钱包地址
- `RegisterService(service)`：注册相应的`Blockchain`服务实例
- `RunSync()`：同步启动客户端服务
- `RunAsync()`：异步启动客户端服务

同时，`MetaTradeClient`启动时标识了`Status`的标志，`Status`具有以下情况：
- `BORN`
- `SEND_INIT`
- `SERVICE_INIT`
- `SUB_ALL`
- `FINISHED`

只有`MetaTradeClient`处于`FINISHED`状态时，客户端服务才能正常工作，否则请进行`Status`标志的排查
## Blockchain实现
### Blockchain服务接口
`MetaTrade Node`为所有的区块链不同实现提供了统一的客户端注册接口，任何自定义的区块链实现都需要继承`BlockChainService`抽象类：

- `RegisterClient(client)`：注册对应的客户端实例
- `virtual Init(msg)`：客户端收到网关的`Init`消息时，会回调该接口，在该接口中，应该实现区块链数据结构的初始化等工作
注：在`Service`中该函数已经实现，请子类`override`时务必在最后调用抽象类的该函数.

- `virtual onTrade(msg)`：客户端收到网关的`Trade`消息时，会进行回调
- `virtual onSpawn(msg)`：客户端收到网关的`Spawn`消息时，会进行回调
- `virtual onJudge(msg)`：客户端收到网关的`Judge`消息时，会进行回调
- `virtual onSemiSync(msg)`：客户端收到网关的`SemiSync`消息时，会进行回调
- `virtual onSync(msg)`：客户端收到网关的`Sync`消息时，会进行回调

具体的消息广播机制和发送机制请参阅[MetaTrade Gateway](https://github.com/Freesia810/MetaTradeGateway)的文档部分