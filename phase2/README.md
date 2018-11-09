# 第二阶段程序说明

在第一阶段的基础上，实现以下改进：
- 增加用户类
- 使用数据库存储信息
- 实现socket网络通信
- 完成服务端接口

## 设计

第一版本的设计是从小精灵类的实现入手的。第二版本不同，将从服务端接口的角度开始设计，并同时考虑数据库的引入

服务器在启动时将处于一个阻塞持续监听的状态，使用多线程技术实现按下任意键停止监听。所以在Server里面设置了两个线程函数listenFunc和terminateFunc，前者用于实现正常的阻塞监听，后者实现按键停止服务器。

针对多用户登录的问题，服务器设置一个统一的登录端口A0，这个端口将写死到客户端程序中。客户端向服务端的端口A0请求登录时，服务端程序将分配给客户用户一个目前未被使用的endpoint端口Ax，客户端将连接此Ax端口来实现持续的连接。

用户名和密码格式要求:
- 硬性要求
	- 没有换行符`\n`
	- 没有空字符`\0`
	- 长度大于6位
- 字符限制
	- 字母
	- 数字
	- 下划线`_`

数据库schema:

```sql
User(
	id integer primary key,
	name text unique not null,
	password text not null
);
```

其中设置User的id为integer类型的primary key的话，可以由sqlite3自动生成id

Endpoint有一个timer，当用户超过timer没有给服务器发数据时服务器会检测socket是否还维持着链接

多Endpoint管理：
- 使用vector保存Endpoint的指针，使所有Endpoints可以通过下标访问，并方便处理内存。
- Endpoint结束条件：当timer超时且发现socket链接已断开，则告诉Server关闭此Endpoint。具体方案如下：
	- public Endpoint::start()函数启动服务并返回端口号，如果返回0则表示启动失败
	- public Endpoint::process()函数里面写socket的accept函数，并在process函数中处理各种请求，返回时代表endpoint结束（超时或用户退出）
	- Server::mornitor(Endpoint *)是线程函数，每当新建Endpoint的时候新建一个线程，调用此函数，参数为新的Endpoint的指针
	- Server首先使用Endpoint::start()获取端口号，然后新建线程把mornitor函数detach。
	- Server::mornitor函数中调用Endpoint::process()开始recv，当process函数返回时表示此endpoint结束运行，在Server的Endpoint指针容器中清除此对象并delete之
- 因为Server的mornitor线程和listenFunc线程函数都需要访问Endpoint指针容器，所以设置锁来防止多线程出现故障

通信方式：客户端发送一条请求，服务器回复一条信息，服务端不需持续监听。客户端发送注销请求时服务器不回复

Endpoint断线重连方案：
- Endpoint使用长连接，设置SOCKET为keep alive自动发送心跳包
- 客户端断开时，recv函数返回SOCKET_ERROR，设置online为false并重新阻塞在accept，detach一个timer线程
- Endpoint在accept成功后设置online为true
- timer超时后如果online仍为false则判定玩家长时间未登陆而退出
- 如果timer超时前玩家重新登陆则终止timer
	- 使用condition_variable实现带有条件的sleep

Server提供的接口：
```shell
login <username> <password>
logon <username> <password>
```

Endpoint提供的接口：
```shell
logout
resetPassword <oldPassword> <newPassword>
getPlayerList
getPokemonList [playerID]
getPokemonByID <pokemonID>
battle <pokemonID> [-auto]
useSkill <skillID>
```