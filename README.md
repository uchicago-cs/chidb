# chidb - A didactic RDBMS

> The chidb documentation is available at <http://chi.cs.uchicago.edu/>
> 参考实现：<https://github.com/iamslash/chidb>、<https://github.com/youngsamwei/chidb>

## 构建和安装

### 准备的包

在linux环境下，我这里尝试过没问题的是在Debian系统下。这些包需要自行安装，但不限于以下包，如果在生成configure中发生错误，可能是还缺其他包

* automake

* autoconf

* libtool

* check

* pkg-config

* libedit-dev

* flex/lex

* bison/yacc

### 生成configure

如果在此步骤发生错误，可以在后面加```--verbose```以显示详细信息。

```bash
./autogen.sh
```

### 生成Makefile

```bash
./configure
```

### 编译

```bash
make
```

### 运行

```bash
./chidb
```
