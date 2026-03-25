# CODEBUDDY.md This file provides guidance to CodeBuddy when working with code in this repository.

## 构建命令

### 编译 32 位版本
适用于通达信 32 位版本：
```cmd
mkdir build
cd build
cmake -A Win32 ..
cmake --build . --config Release
```

### 编译 64 位版本
适用于通达信 64 位版本：
```cmd
mkdir build
cd build
cmake -A x64 ..
cmake --build . --config Release
```

编译后的 DLL 文件位于 `build/Release/ChanlunX.dll`，需根据通达信软件位数选择对应的 DLL 版本（32 位通达信使用 32 位 DLL，64 位通达信使用 64 位 DLL）。

## 架构概述

ChanlunX 是一个通达信 DLL 插件，实现了缠论技术分析的核心算法，包括 K 线包含处理、笔、段和中枢的计算。

### 核心层次结构

项目采用三层架构设计，从原始 K 线数据到高级分析指标的逐层处理：

1. **K 线处理层**（KxianChuLi）
   - 处理 K 线包含关系，将包含的 K 线合并
   - 输出标准化后的 K 线序列，每根 K 线包含方向、高低价、起止位置

2. **笔识别层**（BiChuLi）
   - 基于 KxianChuLi 的输出，识别走势的转折点（顶底分型）
   - 支持两种画法：简笔（Bi1）和标准笔（Bi2）
   - 输出笔的方向（上升/下降）、起止点和高低价

3. **段和中枢层**（Duan, ZhongShu）
   - 段：基于笔序列识别更大级别的走势结构，支持标准画法和 1+1 终结画法
   - 中枢：识别笔的重叠区间，包含方向、高低点、起止位置和序列号

### 数据流向

```
原始 K 线（High/Low）
    ↓
KxianChuLi（包含处理）
    ↓
BiChuLi（笔识别）→ Bi1/Bi2（简笔/标准笔）
    ↓
Duan（段识别）→ Duan1（标准）/Duan2（1+1终结）
    ↓
ZhongShu（中枢计算）→ 中枢高/低/起止/方向/序列
    ↓
BeiChi（MACD背驰）→ 顶背驰/底背驰信号
```

### DLL 导出函数

插件注册了 10 个导出函数（通过 RegisterTdxFunc），编号 1-10：

- **Func1**: 简笔顶底端点
- **Func2**: 标准笔顶底端点
- **Func3**: 段的端点（标准画法）
- **Func4**: 段的端点（1+1 终结画法）
- **Func5**: 中枢高点数据
- **Func6**: 中枢低点数据
- **Func7**: 中枢起点、终点信号（起点=1，终点=2）
- **Func8**: 中枢方向数据
- **Func9**: 同方向的第几个中枢（序列编号）
- **Func10**: MACD 背驰信号（1=顶背驰，-1=底背驰，MACD 参数 10/20/7）

所有函数遵循统一签名：`(int nCount, float *pOut, float *a, float *b, float *c)`，其中 nCount 为 K 线数量，pOut 为输出数组。

### 文件组织

- `Main.cpp/h`: DLL 入口和导出函数注册
- `KxianChuLi.cpp/h`: K 线包含处理逻辑
- `BiChuLi.cpp/h`: 笔识别逻辑
- `Bi.cpp/h`: 笔计算接口实现
- `Duan.cpp/h`: 段计算接口实现
- `ZhongShu.cpp/h`: 中枢计算接口实现
- `BeiChi.cpp/h`: MACD 背驰检测（MACD 参数 SHORT=10, LONG=20, M=7）
- `ChanlunXg.h`: 通达信选股接口定义（兼容性）
- `ChanlunZb.h`: 大智慧/通达信指标接口定义

### 通达信公式使用

将编译好的 DLL 复制到通达信的 `T0002\dlls` 目录，在通达信公式编辑器中调用。示例（绑定为 2 号函数）：

```text
FRAC:=TDXDLL2(2,H,L,0);{标准笔}
NOTEXT画上升笔:DRAWLINE(FRAC=-1,L,FRAC=+1,H,0), DOTLINE,COLORYELLOW;

DUAN1:=TDXDLL2(3,FRAC,H,L);{段的端点标准画法}
NOTEXT画上升段:DRAWLINE(DUAN1=-1,L,DUAN1=+1,H,0), COLORFF8000;

BIZG:=TDXDLL2(5,FRAC,H,L);{中枢高}
BIZD:=TDXDLL2(6,FRAC,H,L);{中枢低}
BISE:=TDXDLL2(7,FRAC,H,L);{中枢起止}

MACDBC:=TDXDLL2(10,FRAC,C,H);{MACD背驰信号, 1=顶背驰 -1=底背驰}
DRAWTEXT(MACDBC=1,H*1.005,'顶背驰'),COLORGREEN;
DRAWTEXT(MACDBC=-1,L*0.995,'底背驰'),COLORRED;
```
