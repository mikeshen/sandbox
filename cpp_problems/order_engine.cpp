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

    // Price -> List of orders at that price
    std::map<double, std::list<OrderNode>> buyOrders;  // Descending price
    std::map<double, std::list<OrderNode>> sellOrders; // Ascending price
    std::unordered_map<int, OrderNode>     orderLookup;

    std::mutex bookMutex; // For thread safety (if required)

    // Helper function to convert reverse iterator to normal iterator
    template <typename Iterator>
    auto convertIterator(Iterator it) -> decltype(it)
    {
        return it;
    }

    template <typename Iterator>
    auto convertIterator(std::reverse_iterator<Iterator> it) -> Iterator
    {
        return std::next(it).base();
    }

    // Helper function to match orders
    template <typename Iterator, typename Comparator>
    void matchOrdersHelper(Order& incomingOrder, Iterator begin, Iterator end, Comparator comp)
    {
        for (auto it = begin; it != end; ++it) {
            auto& orderList = it->second;
            for (auto listIt = orderList.begin();
                 listIt != orderList.end() && incomingOrder.quantity > 0;) {
                auto& existingOrder = listIt->order;
                if (!comp(incomingOrder.price, existingOrder.price))
                    break;

                int tradeQuantity = std::min(incomingOrder.quantity, existingOrder.quantity);
                incomingOrder.quantity -= tradeQuantity;
                existingOrder.quantity -= tradeQuantity;

                if (existingOrder.quantity == 0) {
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
            orderNode.orderList->erase(orderNode.it);
            orderLookup.erase(it);
            std::cout << "Order ID " << orderId << " was cancelled.\n";
        } else {
            std::cerr << "Order ID " << orderId << " not found.\n";
        }
    }
};

// Test case
void testOrderMatching()
{
    OrderBook orderBook;

    // Add some sell orders
    Order sellOrder1(4, false, 102.0, 15, false);
    orderBook.processOrder(sellOrder1);
    Order sellOrder2(5, false, 100.0, 10, false);
    orderBook.processOrder(sellOrder2);
    Order sellOrder3(6, false, 98.0, 25, false);
    orderBook.processOrder(sellOrder3);

    // Add an incoming buy order that should match with the lowest sell order
    Order incomingBuyOrder(7, true, 101.0, 15, false);
    orderBook.processOrder(incomingBuyOrder);
    assert(incomingBuyOrder.quantity == 0); // Should be fully matched

    // Add some buy orders
    Order buyOrder1(1, true, 100.0, 10, false);
    orderBook.processOrder(buyOrder1);
    Order buyOrder2(2, true, 101.0, 5, false);
    orderBook.processOrder(buyOrder2);
    Order buyOrder3(3, true, 99.0, 20, false);
    orderBook.processOrder(buyOrder3);

    // Add an incoming sell order that should match with the highest buy order
    Order incomingSellOrder(8, false, 99.0, 10, false);
    orderBook.processOrder(incomingSellOrder);
    assert(incomingSellOrder.quantity == 0); // Should be fully matched

    std::cout << "All test cases passed.\n";
}

int main()
{
    testOrderMatching();
    return 0;
}
