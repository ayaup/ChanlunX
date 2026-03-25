#ifndef __BEICHI_H__
#define __BEICHI_H__

#include <vector>

// MACD背驰检测
// 参数: SHORT=10, LONG=20, M=7
// pIn: 笔端点(+1顶/-1底), pClose: 收盘价(用于MACD计算), pHigh: 最高价(用于顶背驰价格比较)
// 底背驰价格比较用pClose, 顶背驰价格比较用pHigh
// 返回: 1=顶背驰, -1=底背驰, 0=无信号
std::vector<float> BeiChi(int nCount, std::vector<float> pIn, std::vector<float> pClose, std::vector<float> pHigh);

#endif
