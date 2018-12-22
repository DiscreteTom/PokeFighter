# 第二阶段程序说明

在第一阶段的基础上，实现以下改进：
- 增加用户类
- 使用数据库存储信息
- 实现socket网络通信
- 完成服务端接口

## 设计

第一版本的设计是从小精灵类的实现入手的。第二版本不同，将从服务端接口的角度开始设计，并同时考虑数据库的引入

服务器在启动时将处于一个阻塞持续监听的状态，使用多线程技术实现按下任意键停止监听。所以在Hub里面设置了两个线程函数listenFunc和terminateFunc，前者用于实现正常的阻塞监听，后者实现按键停止服务器。

针对多用户登录的问题，服务器设置一个统一的登录端口A0，这个端口将写死到客户端程序中。客户端向服务端的端口A0请求登录时，服务端程序将分配给客户用户一个目前未被使用的endpoint端口Ax，客户端将连接此Ax端口来实现持续的连接。

用户名格式要求:
- 不包含空格、回车、制表等空白字符
- 长度6-30
密码格式要求:
- 仅包含字母、数字、下划线`_`
- 长度6-30

数据库schema:

```sql
User(
	id integer primary key,
	name text unique not null,
	password text not null,
	win int not null,
	total int not null
);
Pokemon(
	id integer primary key,
	userid integer not null,
	name text not null,
	race int not null,
	atk int not null,
	def int not null,
	maxHp int not null,
	speed int not null,
	lv int not null,
	exp int not null,
);
```

其中设置User的id为integer类型的primary key的话，可以由sqlite3自动生成id

Endpoint有一个timer，当用户超过timer没有给服务器发数据时服务器会检测socket是否还维持着链接

多Endpoint管理：
- 使用vector保存Endpoint的指针，使所有Endpoints可以通过下标访问，并方便处理内存。
- Endpoint结束条件：当timer超时且发现socket链接已断开，则告诉Hub关闭此Endpoint。具体方案如下：
	- public Endpoint::start()函数启动服务并返回端口号，如果返回0则表示启动失败
	- public Endpoint::process()函数里面写socket的accept函数，并在process函数中处理各种请求，返回时代表endpoint结束（超时或用户退出）
	- Hub::mornitor(Endpoint *)是线程函数，每当新建Endpoint的时候新建一个线程，调用此函数，参数为新的Endpoint的指针
	- Hub首先使用Endpoint::start()获取端口号，然后新建线程把mornitor函数detach。
	- Hub::mornitor函数中调用Endpoint::process()开始recv，当process函数返回时表示此endpoint结束运行，在Hub的Endpoint指针容器中清除此对象并delete之
- 因为Hub的mornitor线程和listenFunc线程函数都需要访问Endpoint指针容器，所以设置锁来防止多线程出现故障

通信方式：客户端发送一条请求，服务器回复一条信息，服务端不需持续监听。客户端发送注销请求时服务器不回复

Endpoint断线重连方案：
- Endpoint使用长连接，设置SOCKET为keep alive自动发送心跳包
- 客户端断开时，recv函数返回SOCKET_ERROR，设置online为false并重新阻塞在accept，detach一个timer线程
- Endpoint在accept成功后设置online为true
- timer超时后如果online仍为false则判定玩家长时间未登陆而退出
- 如果timer超时前玩家重新登陆则终止timer
	- 使用condition_variable实现带有条件的sleep

Hub提供的接口：
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
getPokemon <pokemonID>
pokemonChangeName <pokemonID> <newName>
battle <type: LV_UP | DUEL> <pokemonID> <enemyRaceID> <enemyLV>
useSkill <skillID>
getDuelStatistic
chooseBet
discard <pokemonID>
```

编码相关：
服务端使用GB2312编码，客户端识别时使用`QString QString::fromLocal8Bit(const char *str, int size = -1)`来实现字符转换，发送时如果信息包含中文也需要使用`QString.toLocal8Bit`来实现转换，以此实现中文信息传递。

目前能够出现中文的地方有：
- 用户名
- 精灵名
- 种族名

需要安装LAV filters才能播放音频。音频文件保存在exe目录下的media目录下

使用QSS实现样式与内容分离。注意QSS文件一定**不要**使用UTF-8编码，建议使用ASCII编码。此处使用了GB2312

发送到客户端的战斗信息格式：

```shell
<playerRound: 0 | 1> <skillName> <dodge: 0 | 1> <defenderHP> <defenderAtk: 0 | 1 | 2> <defenderDef: 0 | 1 | 2> <defenderSpeed: 0 | 1 | 2> <defenderPP1> <defenderPP2> <defenderPP3> <attackerHP> <attackerAtk: 0 | 1 | 2> <attackerDef: 0 | 1 | 2> <attackerSpeed: 0 | 1 | 2> <attackerPP1> <attackerPP2> <attackerPP3>
```

其中playerRound为1表示玩家的回合，为0表示对手的回合。属性为0表示下降，为1表示不变，为2表示上升。HP显示精灵剩余血量。闪避为0表示闪避失败

客户端需要监视精灵血量，当血量为0时战斗终止。

自动战斗由客户端实现，随机使用技能。