#include <iostream>
#include <map>
#include <list>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <mutex>

struct Order {
    int orderId;
    bool isBuy;
    double price; // Price for limit orders, ignored for market orders
    int quantity;
    bool fillOrKill; // True for fill-or-kill orders
    std::chrono::time_point<std::chrono::steady_clock> timestamp;
};

class OrderBook {
    struct OrderNode {
        Order order;
        std::list<OrderNode>::iterator it;
        std::list<OrderNode>* orderList; // Pointer to the list
    };

    // Price -> List of orders at that price
    std::map<double, std::list<OrderNode>> buyOrders; // Descending price
    std::map<double, std::list<OrderNode>> sellOrders; // Ascending price
    std::unordered_map<int, OrderNode> orderLookup;

    std::mutex bookMutex; // For thread safety (if required)

    // Match orders using price-time priority
    void matchOrder(Order& incomingOrder) {
        auto& matchingOrders = incomingOrder.isBuy ? sellOrders : buyOrders;
        auto& sameSideOrders = incomingOrder.isBuy ? buyOrders : sellOrders;

        for (auto it = matchingOrders.begin(); it != matchingOrders.end() && incomingOrder.quantity > 0;) {
            auto& orderList = it->second;
            for (auto listIt = orderList.begin(); listIt != orderList.end() && incomingOrder.quantity > 0;) {
                auto& existingOrder = listIt->order;
                if (incomingOrder.isBuy && incomingOrder.price < existingOrder.price) break;
                if (!incomingOrder.isBuy && incomingOrder.price > existingOrder.price) break;

                int tradeQuantity = std::min(incomingOrder.quantity, existingOrder.quantity);
                incomingOrder.quantity -= tradeQuantity;
                existingOrder.quantity -= tradeQuantity;

                if (existingOrder.quantity == 0) {
                    listIt = orderList.erase(listIt);
                } else {
                    ++listIt;
                }
            }

            if (orderList.empty()) {
                it = matchingOrders.erase(it);
            } else {
                ++it;
            }
        }

        // Handle remaining quantities for fill-or-kill orders
        if (incomingOrder.fillOrKill && incomingOrder.quantity > 0) {
            incomingOrder.quantity = 0; // Cancel the order
            std::cout << "Fill-or-kill order ID " << incomingOrder.orderId << " was cancelled.\n";
        }

        // Add remaining quantities to the same side order book
        if (incomingOrder.quantity > 0) {
            addOrderToBook(incomingOrder, sameSideOrders);
        }
    }

    // Add a new order to the appropriate order book
    void addOrderToBook(Order& order, std::map<double, std::list<OrderNode>>& orderBook) {
        OrderNode newNode{order};
        auto it = orderBook.find(order.price);

        if (it == orderBook.end()) {
            orderBook[order.price] = std::list<OrderNode>();
        }

        auto& orderList = orderBook[order.price];
        newNode.it = orderList.insert(orderList.end(), newNode);
        newNode.orderList = &orderList; // Store the pointer to the list
        orderLookup[order.orderId] = newNode;
    }

public:
    void processOrder(Order& order) {
        std::lock_guard<std::mutex> lock(bookMutex);

        if (order.quantity <= 0) return;

        if (orderLookup.find(order.orderId) != orderLookup.end()) {
            std::cerr << "Order ID " << order.orderId << " already exists.\n";
            return;
        }

        // Match incoming order
        matchOrder(order);
    }

    void cancelOrder(int orderId) {
        std::lock_guard<std::mutex> lock(bookMutex);

        auto it = orderLookup.find(orderId);
        if (it == orderLookup.end()) {
            std::cerr << "Order ID " << orderId << " not found.\n";
            return;
        }

        auto& node = it->second;
        auto& orderList = *node.orderList;
        double price = node.order.price;
        orderList.erase(node.it);

        if (orderList.empty()) {
            auto& orderBook = node.order.isBuy ? buyOrders : sellOrders;
            orderBook.erase(price);
        }

        orderLookup.erase(it);
        std::cout << "Cancelled Order ID " << orderId << "\n";
    }
};

// Example usage
int main() {
    OrderBook orderBook;
    Order order1{1, true, 100.0, 10, false, std::chrono::steady_clock::now()};
    Order order2{2, false, 100.0, 5, false, std::chrono::steady_clock::now()};

    orderBook.processOrder(order1);
    orderBook.processOrder(order2);

    orderBook.cancelOrder(1);

    return 0;
}