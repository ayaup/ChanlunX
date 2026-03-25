#include "BeiChi.h"
#include <cmath>
#include <algorithm>

// 计算EMA序列
static std::vector<float> CalcEMA(const std::vector<float>& data, int period)
{
    std::vector<float> ema(data.size(), 0.0f);
    if (data.empty() || period <= 0)
        return ema;

    float k = 2.0f / (period + 1);
    // SMA初始化: 用前period个数据均值作为第一个EMA
    int start = std::min(period - 1, (int)data.size() - 1);
    float sum = 0;
    for (int i = 0; i <= start; i++)
    {
        sum += data[i];
    }
    ema[start] = sum / (start + 1);

    for (int i = start + 1; i < (int)data.size(); i++)
    {
        ema[i] = data[i] * k + ema[i - 1] * (1 - k);
    }
    return ema;
}

std::vector<float> BeiChi(int nCount, std::vector<float> pIn, std::vector<float> pClose, std::vector<float> pHigh)
{
    std::vector<float> out(nCount, 0.0f);
    if (nCount < 25) // 数据不足，无法计算MACD(10,20,7)
        return out;

    // MACD参数
    const int SHORT = 10;
    const int LONG = 20;
    const int M = 7;

    // 计算MACD
    std::vector<float> emaS = CalcEMA(pClose, SHORT);
    std::vector<float> emaL = CalcEMA(pClose, LONG);

    // DIF = EMA(SHORT) - EMA(LONG)
    std::vector<float> dif(nCount, 0.0f);
    for (int i = 0; i < nCount; i++)
    {
        dif[i] = emaS[i] - emaL[i];
    }

    // DEA = EMA(DIF, M)
    std::vector<float> dea = CalcEMA(dif, M);

    // MACD柱 = 2 * (DIF - DEA)
    std::vector<float> macdBar(nCount, 0.0f);
    for (int i = 0; i < nCount; i++)
    {
        macdBar[i] = 2.0f * (dif[i] - dea[i]);
    }

    // 收集笔端点
    struct Pt
    {
        int index;
        float price;     // 极端价格(底用Close, 顶用High)
        float difVal;
        float macdBarVal;
    };

    std::vector<Pt> tops;  // 笔顶列表
    std::vector<Pt> bots;  // 笔底列表

    for (int i = 0; i < nCount; i++)
    {
        if (pIn[i] == 1)
        {
            Pt pt;
            pt.index = i;
            pt.price = pHigh[i];       // 顶背驰用High比较
            pt.difVal = dif[i];
            pt.macdBarVal = macdBar[i];
            tops.push_back(pt);
        }
        else if (pIn[i] == -1)
        {
            Pt pt;
            pt.index = i;
            pt.price = pClose[i];      // 底背驰用Close比较
            pt.difVal = dif[i];
            pt.macdBarVal = macdBar[i];
            bots.push_back(pt);
        }
    }

    // 顶背驰检测: 连续两个笔顶，价格创新高但DIF未创新高
    for (size_t i = 1; i < tops.size(); i++)
    {
        if (tops[i].price > tops[i - 1].price && tops[i].difVal < tops[i - 1].difVal)
        {
            out[tops[i].index] = 1;
        }
    }

    // 底背驰检测: 连续两个笔底，价格创新低但DIF未创新低
    for (size_t i = 1; i < bots.size(); i++)
    {
        if (bots[i].price < bots[i - 1].price && bots[i].difVal > bots[i - 1].difVal)
        {
            out[bots[i].index] = -1;
        }
    }

    return out;
}
