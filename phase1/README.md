# 第一阶段程序说明

## 文件清单

- pokemon.h
- pokemon.cpp
- battlecontroller.h
- battlecontroller.cpp
- tester.cpp

## 使用说明

### 种族初始化与精灵生成

在tester.cpp中如下代码初始化了四个种族

```c++
Race<0> race1;
Race<1> race2;
Race<2> race3;
Race<3> race4;
```

四个种族分别为小火龙（高攻击）、妙蛙种子（高血量）、杰尼龟（高防御）和波波（高速度）

初始化所有种族之后即可使用Pokemon类的构造函数来生成一只精灵。构造函数的第一个参数是精灵的种族，第二个参数为精灵的名字。如果没有给出精灵的名字，则默认使用种族名。示例：

```c++
Pokemon pokemon1(race1, "pokemon1");
Pokemon pokemon2(race2);
```

生成精灵后会看到如下输出：

```
Init pokemon1 from Charmander
Type: High Attack
Atk: 13
Def: 7
MaxHp: 30
Speed: 11
LV: 1
Exp: 0
Skills:
        Name: kick
        Description: simple attack
        PP: infinity
        Name: spark
        Description: ignore opponent's half defence
        PP: 10
        Name: rage
        Description: increase attack
        PP: 3
        Name: fireball
        Description: cause huge damage
        PP: 5
```

### 升级

此版本仅提供增加宠物经验值的函数（即升级函数），并未给出经验值结算方案。计划将在以后版本设计。

函数Pokemon::gainExp(int count)可以给精灵增加经验值。经验值达到一定值（升级经验曲线）后即可升级。调用一次此函数可以造成多次升级。参数count应为正整数，不符合条件的count会被默认替换为1

示例：使用代码`pokemon1.gainExp(10)`，可以看到如下输出

```
pokemon1 gains 10 exp!
Now pokemon1 has 10 exp

Level Up!
pokemon1's now LV2!

Atk: 10->18!
Def: 3->5!
MaxHP: 24->34!
Speed: 8->13!
```

### 战斗

如果要进行精灵之间的战斗，则需要构造一个战斗控制器BattleController，其构造函数为`BattleController(Pokemon &pokemon1, Pokemon &pokemon2, bool autoFight = true);`，即需要传递两个已经存在的精灵（可以是同一只）。第三个参数为自动战斗标识，默认自动战斗，设置为false的时候也可以手动战斗

构造BattleController之后，使用BattleController::start()即可开启战斗

示例代码：

```c++
	//  BattleController battle(pokemon1, pokemon2);//auto fight
	BattleController battle(pokemon1, pokemon3, false); //manual fight
	battle.start();
```

自动战斗模式下，系统会随机在精灵能够使用的技能中挑选一个释放。自动战斗产生的输出如下：

```
pokemon1 VS pokemon2!
Battle Start!

pokemon1 uses kick!
pokemon2 takes 13 damage!
pokemon2's HP becomes 24

pokemon2 uses kick!
pokemon1 takes 10 damage!
pokemon1's HP becomes 28

pokemon1 uses kick!
pokemon2 takes 12 damage!
pokemon2's HP becomes 12

pokemon1 uses kick!
pokemon2 takes 10 damage!
pokemon2's HP becomes 2

pokemon2 uses kick!
pokemon1 takes 10 damage!
pokemon1's HP becomes 18

pokemon1 uses kick!
pokemon2 takes 12 damage!
pokemon2 is down!

pokemon1 won!
```

手动战斗模式下，玩家可以选择每一步释放哪个技能。关于技能的获取将在下文中提到。手动战斗可以获得如下输出：

```
pokemon1 VS pokemon2!
Battle Start!

pokemon1, your turn!
Choose a skill to attack!
        1: kick               simple attack
        2: spark(10/10)       ignore opponent's half defence
        3: rage( 3/ 3)        increase attack
        4: fireball( 5/ 5)    cause huge damage
Please input a number: 3

pokemon1 uses rage!
pokemon1's Attack +13
pokemon1's Attack becomes 121

pokemon2, your turn!
Choose a skill to attack!
        1: kick                     simple attack
        2: photosynthesis( 5/ 5)    restore HP
        3: life drain(10/10)        cause damage and restore HP, ignore defence
        4: razor leaf( 5/ 5)        cause huge damage
Please input a number: 4

pokemon2 uses razor leaf!
pokemon1 takes 82 damage!
pokemon1's HP becomes 48

pokemon1, your turn!
Choose a skill to attack!
        1: kick               simple attack
        2: spark(10/10)       ignore opponent's half defence
        3: rage( 2/ 3)        increase attack
        4: fireball( 5/ 5)    cause huge damage
Please input a number: 4

pokemon1 uses fireball!
pokemon2 takes 157 damage!
pokemon2's HP becomes 67

pokemon2, your turn!
Choose a skill to attack!
        1: kick                     simple attack
        2: photosynthesis( 5/ 5)    restore HP
        3: life drain(10/10)        cause damage and restore HP, ignore defence
        4: razor leaf( 4/ 5)        cause huge damage
Please input a number: 3

pokemon2 uses life drain!
pokemon2 restores 46HP!
pokemon2's HP becomes 113

pokemon1 takes 46 damage!
pokemon1's HP becomes 2

pokemon1, your turn!
Choose a skill to attack!
        1: kick               simple attack
        2: spark(10/10)       ignore opponent's half defence
        3: rage( 2/ 3)        increase attack
        4: fireball( 4/ 5)    cause huge damage
Please input a number: 4

pokemon1 uses fireball!
pokemon2 takes 152 damage!
pokemon2 is down!

pokemon1 won!

请按任意键继续. . .
```

可以看到，精灵的技能有释放次数（PP值），每次释放都会减少此技能的PP值。精灵的第一个技能kick为普通攻击，可以无限次释放。技能后面跟着技能描述，玩家可以根据技能描述来选择释放技能。

玩家需要输入一个数字来释放技能。如果出现错误输入则默认释放普通攻击。当一个精灵的血量HP为0时，另一个精灵获得胜利

## 设计描述

### pokemon.h/pokemon.cpp

根据题目要求，设置枚举类型“精灵类型（PokemonType）”，提供四个枚举值ATK/HP/DEF/SPE来表示此类型的精灵的那个属性比较强

在设计的时候使用“速度”代替了“攻击间隔”这一个量

设计基类PokemonBase，用来派生出不同种族的精灵。基类拥有种族名、精灵类型、类型基础攻击、类型基础防御、类型基础血量、类型基础速度、类型经验曲线、种族技能名称、种族技能描述和种族技能PP值。这些属性的访问权限全部设计为protected以便被派生类访问和被基类指针访问

PokemonBase的构造函数需要传入一个参数type，即精灵类型。PokemonBase会根据精灵类型生成类型基础属性，即同类型的精灵的初始属性是相同的

PokemonBase提供了纯虚函数attack使精灵可以发动攻击。dodge函数用来判断闪避

模板类Race为PokemonBase的基类，意为不同的精灵种族。如`Race<0>`是小火龙，`Race<1>`是妙蛙种子。此模板类只需要定义两个函数，一个是构造函数，另一个是来自基类的攻击函数attack

Race类的构造函数会指定此种族的精灵属于哪些类型，比如小火龙种族属于高攻击型，则使用枚举量ATK作为基类PokemonBase的参数。构造函数还会把PokemonBase中没有初始化的属性全部初始化，包括种族名、经验曲线、技能名称、技能描述、技能PP值。

Race类的attack函数则为此种族的每个技能提供了计算公式。

Pokemon类是具体的小精灵类，它包括种族、小精灵名、当前攻击力、当前防御力、当前血量上限、当前速度、当前等级和当前经验值这些属性。因为在战斗中有些属性会临时发生变化，所以还有战斗中血量、战斗中攻击、战斗中防御、战斗中速度和战斗中PP点这五个属性。战斗时的临时属性拥有getter和setter函数，但是非战斗时属性只有getter函数，其值只能通过构造函数或升级函数更改。

Pokemon的构造函数需要一个种族的const引用作为参数，表示这个精灵属于此种族。因为精灵不可能更改种族的属性，所以种族在内部保存为一个const引用。精灵还可以被重命名，如果没有重命名，则默认使用种族名。精灵在生成时会获得一些基础属性，如基础攻击、基础防御等。这些属性还会被附加一些随机因素，使同一种族生成的精灵有不同的属性

关于攻击函数attack，小精灵Pokemon类中的attack用来实现自动战斗和手动战斗的控制相关操作，确定发动哪个技能后调用种族Race的attack函数。模板类Race中的攻击函数定义了此种族的精灵的攻击计算方式与攻击效果。

小精灵的技能会随着等级的增长而获得，初始只有普通攻击kick，5级时获得第二个技能，10级获得第三个技能，15级获得第四个技能。

### battlecontroller.h/battlecontroller.cpp

当需要实现两只小精灵的战斗时需要构造战斗控制器BattleController。其构造函数的参数为两个小精灵的引用和一个自动战斗标识。构造结束后调用start函数即可开启战斗

战斗时会有两个计时器timer。计时器会从0开始上升，当精灵A的计时器不小于精灵A的速度时，精灵B发动攻击，以此实现速度高的精灵攻击快的效果。

如果两只小精灵的timer同时达到标准，则速度高的小精灵先发动攻击

当存在一只小精灵的血量为0时另一只小精灵获得战斗胜利