# MetaTrade Node
`MetaTrade Gateway`网关服务的本地客户端实现
# 目录
## 项目依赖
- [libWebStomp++](https://github.com/Freesia810/WebStomppp)：提供客户端的`Stomp`服务
- [rmd160](https://homes.esat.kuleuven.be/~bosselae/ripemd160/)：实现`RIPEMD-160`哈希算法加密
- [picosha2](https://github.com/okdshin/PicoSHA2)：提供SHA-256哈希算法
- [cJSON](https://github.com/DaveGamble/cJSON)：提供轻量的`json`序列化与反序列化服务
- [secp256k1](https://github.com/bitcoin-core/secp256k1)：提供区块链公钥和地址生成的`ECC`算法和公钥压缩算法

程序运行时依赖这些`dll`，请将它们放在`exe`文件的同一目录下，否则程序无法正常运行

## 安全验证
### 生成钱包地址
钱包地址生成算法参考[Bitcoin](https://developer.bitcoin.org/index.html)的生成算法：

- 地址的生成首先需要私钥（32 bytes/64位16进制串）
- 将私钥进行`ECC`算法，获得压缩后的公钥（33 bytes/66位16进制串）
- 将获得的公钥进行`RIPEMD-160`加密，获得新的摘要（20 bytes/40位16进制串）
- 将摘要值数据前面加上一个byte的版本号数据(0x00)，获得新的公钥，将其进行两次`SHA-256`加密，获得摘要（32 bytes/64位16进制串）
- 获得摘要值的前四个字节即为公钥的校验值，将这四个字节放在新的公钥后面，获得地址的原始数据（25 bytes/50位16进制串）
- 将地址的原始数据进行`Base-58`编码，获得以`1`开始的34位字符，即为统一标识身份的钱包地址

### 验证钱包地址
客户端会验证其他节点提供的地址是否正确，这主要是生成算法的逆向操作，这包括：

- 将地址进行`Base-58`解码，从而获得原始地址数据（25 bytes/50位16进制串）
- 取原始地址数据的后四个字节，这即为待验证的校验值（4bytes/8位16进制串）
- 将去掉校验值后的原始地址数据进行两次`SHA-256`加密，获得摘要
- 比较获得摘要的前四个字节和校验和是否相同，即可验证钱包地址的真实性

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