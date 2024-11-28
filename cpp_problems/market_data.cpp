#include <chrono>
#include <cmath>
#include <deque>
#include <iostream>
#include <mutex>
#include <unordered_map>
#include <vector>

/*
Follow-up Questions

Performance Optimization:
"How would you modify the implementation to reduce lock contention in high-throughput scenarios?"
Memory Management:
"Design a memory pool allocator for TickData to reduce heap allocations and fragmentation."
Distributed Systems:
"How would you extend this to work across multiple machines while maintaining consistency?"
Advanced Statistics:
"Add support for calculating real-time Sharpe ratio and beta coefficients."
Fault Tolerance:
"Implement a recovery mechanism to handle system crashes while maintaining statistical accuracy."
Zero-Copy Implementation:
"How would you modify the design to minimize data copying and memory allocations?"
Hardware Optimization:
"How would you optimize this for SIMD instructions or GPU processing?"
Time Complexity:
"Can you improve the cleanup_old_ticks() time complexity while maintaining thread safety?"
*/

struct TickData {
    double price;
    int    timestamp;
    double volume;
};

class MarketData
{
    struct SymbolStats {
        std::deque<TickData> tickWindow;
        std::deque<double>   squaredReturnsWindow;
        double               sumPrice{0.0};
        double               sumVolume{0.0};
        double               sumSquaredReturns{0.0};
        double               previousPrice{0.0};
        std::mutex           mtx;
    };

    std::unordered_map<std::string, SymbolStats> symbolData;
    const double ANOMALY_THRESHOLD = 0.1; // 10% price change threshold

   public:
    void process_tick(const std::string& symbol, double price, int timestamp, double volume = 1.0)
    {
        auto&                       stats = symbolData[symbol];
        std::lock_guard<std::mutex> lock(stats.mtx);

        // Anomaly detection
        if (stats.previousPrice > 0) {
            double priceChange = std::abs(price - stats.previousPrice) / stats.previousPrice;
            if (priceChange > ANOMALY_THRESHOLD) {
                // Handle anomaly - could log, reject, or adjust
                return;
            }
        }

        // Update rolling statistics
        stats.tickWindow.push_back({price, timestamp, volume});
        stats.sumPrice += price * volume;
        stats.sumVolume += volume;

        // Update volatility metrics
        if (stats.previousPrice > 0) {
            double return_       = std::log(price / stats.previousPrice);
            double squaredReturn = return_ * return_;
            stats.sumSquaredReturns += squaredReturn;
            stats.squaredReturnsWindow.push_back(squaredReturn);
        }
        stats.previousPrice = price;

        // Remove old ticks
        cleanup_old_ticks(stats, timestamp);
    }

    double get_vwap(const std::string& symbol)
    {
        auto&                       stats = symbolData[symbol];
        std::lock_guard<std::mutex> lock(stats.mtx);

        if (stats.sumVolume == 0)
            return 0.0;
        return stats.sumPrice / stats.sumVolume;
    }

    double get_price_volatility(const std::string& symbol, size_t window_ticks)
    {
        auto&                       stats = symbolData[symbol];
        std::lock_guard<std::mutex> lock(stats.mtx);

        if (stats.squaredReturnsWindow.size() < 2)
            return 0.0;

        // Adjust sumSquaredReturns to reflect only the last window_ticks returns
        while (stats.squaredReturnsWindow.size() > window_ticks) {
            stats.sumSquaredReturns -= stats.squaredReturnsWindow.front();
            stats.squaredReturnsWindow.pop_front();
        }

        return std::sqrt(stats.sumSquaredReturns / (stats.squaredReturnsWindow.size() - 1));
    }

   private:
    void cleanup_old_ticks(SymbolStats& stats, int currentTime)
    {
        while (!stats.tickWindow.empty() &&
               currentTime - stats.tickWindow.front().timestamp > 3600) {
            const auto& oldTick = stats.tickWindow.front();
            stats.sumPrice -= oldTick.price * oldTick.volume;
            stats.sumVolume -= oldTick.volume;
            stats.tickWindow.pop_front();
        }
    }
};

int main()
{
    return 0;
}