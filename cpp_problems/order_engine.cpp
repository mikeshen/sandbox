#include <algorithm>
#include <cassert>
#include <chrono>
#include <functional>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <mutex>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include "test_runner.h"

using namespace std;

class Order
{
   public:
    int    orderId;
    bool   isBuy;
    double price; // Price for limit orders, ignored for market orders
    int    quantity;
    bool   fillOrKill; // True for fill-or-kill orders
    std::chrono::time_point<std::chrono::steady_clock> timestamp;

    // Default constructor
    Order()
        : orderId(0),
          isBuy(false),
          price(0.0),
          quantity(0),
          fillOrKill(false),
          timestamp(std::chrono::steady_clock::now())
    {
    }

    // Parameterized constructor
    Order(int id, bool buy, double p, int q, bool fok)
        : orderId(id),
          isBuy(buy),
          price(p),
          quantity(q),
          fillOrKill(fok),
          timestamp(std::chrono::steady_clock::now())
    {
    }

    // Copy constructor
    Order(const Order& other)
        : orderId(other.orderId),
          isBuy(other.isBuy),
          price(other.price),
          quantity(other.quantity),
          fillOrKill(other.fillOrKill),
          timestamp(other.timestamp)
    {
    }

    // Move constructor
    Order(Order&& other) noexcept
        : orderId(other.orderId),
          isBuy(other.isBuy),
          price(other.price),
          quantity(other.quantity),
          fillOrKill(other.fillOrKill),
          timestamp(std::move(other.timestamp))
    {
        other.orderId    = 0;
        other.isBuy      = false;
        other.price      = 0.0;
        other.quantity   = 0;
        other.fillOrKill = false;
    }

    // Copy assignment operator
    Order& operator=(const Order& other)
    {
        if (this != &other) {
            orderId    = other.orderId;
            isBuy      = other.isBuy;
            price      = other.price;
            quantity   = other.quantity;
            fillOrKill = other.fillOrKill;
            timestamp  = other.timestamp;
        }
        return *this;
    }

    // Move assignment operator
    Order& operator=(Order&& other) noexcept
    {
        if (this != &other) {
            orderId    = other.orderId;
            isBuy      = other.isBuy;
            price      = other.price;
            quantity   = other.quantity;
            fillOrKill = other.fillOrKill;
            timestamp  = std::move(other.timestamp);

            other.orderId    = 0;
            other.isBuy      = false;
            other.price      = 0.0;
            other.quantity   = 0;
            other.fillOrKill = false;
        }
        return *this;
    }
};

class OrderBook
{
    struct OrderNode {
        Order                          order;
        std::list<OrderNode>::iterator it;
        std::list<OrderNode>*          orderList; // Pointer to the list

        OrderNode(const Order& o) : order(o), orderList(nullptr) {}
        OrderNode() : order(0, false, 0.0, 0, false), orderList(nullptr) {} // Default constructor
    };

   public:
    // Price -> List of orders at that price
    std::map<double, std::list<OrderNode>> buyOrders;  // Descending price
    std::map<double, std::list<OrderNode>> sellOrders; // Ascending price
    std::unordered_map<int, OrderNode>     orderLookup;

    std::mutex bookMutex; // For thread safety (if required)

   private:
    // Helper function to match orders
    template <typename Iterator, typename Comparator>
    void matchOrdersHelper(Order& incomingOrder, Iterator begin, Iterator end, Comparator comp)
    {
        for (auto it = begin; it != end; ++it) {
            auto& orderList = it->second;
            for (auto listIt = orderList.begin();
                 listIt != orderList.end() && incomingOrder.quantity > 0;) {
                auto& existingOrder = listIt->order;
                if (comp(incomingOrder.price, existingOrder.price))
                    return;

                int tradeQuantity = std::min(incomingOrder.quantity, existingOrder.quantity);
                incomingOrder.quantity -= tradeQuantity;
                existingOrder.quantity -= tradeQuantity;

                if (existingOrder.quantity == 0) {
                    orderLookup.erase(existingOrder.orderId);
                    listIt = orderList.erase(listIt);
                } else {
                    ++listIt;
                }
            }
        }
    }

    // Match orders using price-time priority
    void matchOrder(Order& incomingOrder)
    {
        auto& matchingOrders = incomingOrder.isBuy ? sellOrders : buyOrders;
        auto& sameSideOrders = incomingOrder.isBuy ? buyOrders : sellOrders;

        if (incomingOrder.isBuy) {
            // Start from the lowest price for sell orders
            matchOrdersHelper(incomingOrder,
                              matchingOrders.begin(),
                              matchingOrders.end(),
                              std::less_equal<double>());

            // Delete empty levels starting from begin for sellOrders
            for (auto it = matchingOrders.begin(); it != matchingOrders.end();) {
                if (it->second.empty()) {
                    it = matchingOrders.erase(it);
                } else {
                    ++it;
                }
            }
        } else {
            // Start from the highest price for buy orders
            matchOrdersHelper(incomingOrder,
                              matchingOrders.rbegin(),
                              matchingOrders.rend(),
                              std::greater_equal<double>());

            // Delete empty levels starting from rbegin for buyOrders
            for (auto it = matchingOrders.rbegin(); it != matchingOrders.rend();) {
                if (it->second.empty()) {
                    it = decltype(it)(matchingOrders.erase(std::next(it).base()));
                } else {
                    ++it;
                }
            }
        }

        // Handle remaining quantities for fill-or-kill orders
        if (incomingOrder.fillOrKill && incomingOrder.quantity > 0) {
            incomingOrder.quantity = 0; // Cancel the order
            std::cout << "Fill-or-kill order ID " << incomingOrder.orderId << " was cancelled.\n";
        }

        // Add remaining quantities to the same side order book
        if (incomingOrder.quantity > 0) {
            OrderNode newNode(incomingOrder);
            auto      it = sameSideOrders[incomingOrder.price].insert(
                sameSideOrders[incomingOrder.price].end(), newNode);
            newNode.it                         = it;
            newNode.orderList                  = &sameSideOrders[incomingOrder.price];
            orderLookup[incomingOrder.orderId] = newNode;
        }
    }

   public:
    void processOrder(Order& order)
    {
        std::lock_guard<std::mutex> lock(bookMutex);

        if (order.quantity <= 0)
            return;

        if (orderLookup.find(order.orderId) != orderLookup.end()) {
            std::cerr << "Order ID " << order.orderId << " already exists.\n";
            return;
        }

        // Match incoming order
        matchOrder(order);
    }

    void cancelOrder(int orderId)
    {
        std::lock_guard<std::mutex> lock(bookMutex);

        auto it = orderLookup.find(orderId);
        if (it != orderLookup.end()) {
            auto& orderNode = it->second;
            auto& orderList = *orderNode.orderList;
            orderList.erase(orderNode.it);

            // Remove the price level if the list is empty
            if (orderList.empty()) {
                if (orderNode.order.isBuy) {
                    buyOrders.erase(orderNode.order.price);
                } else {
                    sellOrders.erase(orderNode.order.price);
                }
            }

            orderLookup.erase(it);
            std::cout << "Order ID " << orderId << " was cancelled.\n";
        } else {
            std::cerr << "Order ID " << orderId << " not found.\n";
        }
    }
};

void testAddBuyOrder()
{
    OrderBook orderBook;
    Order     order(1, true, 100.0, 10, false);
    orderBook.processOrder(order);

    customAssert(orderBook.buyOrders.size() == 1);
    customAssert(orderBook.buyOrders.begin()->second.size() == 1);
    customAssert(orderBook.orderLookup.size() == 1);
}

void testAddSellOrder()
{
    OrderBook orderBook;
    Order     order(2, false, 50.0, 5, false);
    orderBook.processOrder(order);

    customAssert(orderBook.sellOrders.size() == 1);
    customAssert(orderBook.sellOrders.begin()->second.size() == 1);
    customAssert(orderBook.orderLookup.size() == 1);
}

void testMatchOrders()
{
    OrderBook orderBook;
    Order     buyOrder(1, true, 100.0, 10, false);
    Order     sellOrder(2, false, 90.0, 5, false);

    orderBook.processOrder(buyOrder);
    orderBook.processOrder(sellOrder);

    customAssert(orderBook.buyOrders.size() == 1);
    customAssert(orderBook.buyOrders.begin()->second.begin()->order.quantity == 5);
    customAssert(orderBook.sellOrders.size() == 0);
    customAssert(orderBook.orderLookup.size() == 1);
}

void testFillOrKillOrder()
{
    OrderBook orderBook;
    Order     buyOrder(1, true, 100.0, 10, false);
    Order     sellOrder(2, false, 90.0, 5, false);
    Order     fillOrKillOrder(3, true, 95.0, 10, true);

    orderBook.processOrder(buyOrder);
    orderBook.processOrder(sellOrder);
    orderBook.processOrder(fillOrKillOrder);

    customAssert(orderBook.buyOrders.size() == 1);
    customAssert(orderBook.buyOrders.begin()->second.begin()->order.quantity == 5);
    customAssert(orderBook.sellOrders.size() == 0);
    customAssert(orderBook.orderLookup.size() == 1);
}

void testCancelOrder()
{
    OrderBook orderBook;
    Order     order(1, true, 100.0, 10, false);
    orderBook.processOrder(order);
    orderBook.cancelOrder(1);

    customAssert(orderBook.buyOrders.size() == 0);
    customAssert(orderBook.orderLookup.size() == 0);
}

void runTests()
{
    vector<string> testResults;

    testResults.push_back(runTest("testAddBuyOrder", testAddBuyOrder));
    testResults.push_back(runTest("testAddSellOrder", testAddSellOrder));
    testResults.push_back(runTest("testMatchOrders", testMatchOrders));
    testResults.push_back(runTest("testFillOrKillOrder", testFillOrKillOrder));
    testResults.push_back(runTest("testCancelOrder", testCancelOrder));

    // Print test results
    for (const auto& result : testResults) {
        cout << result << endl;
    }
}

int main()
{
    runTests();
    return 0;
}