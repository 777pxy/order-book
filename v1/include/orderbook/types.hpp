#include <cstdint>
enum class Side {
    Buy,
    Sell
};

enum class OrderType {
    Limit,
    Market
};

struct Order {
    uint64_t id;
    Side side;
    OrderType type;
    int64_t price;
    uint32_t qty;
    uint64_t seq;
    /*
    seq = sequence number
    instrument = one arbitrary asset e.g. stock, bond, future...
    order = a request to buy some volume of an instrument
    price-time priority: Used as the dealbreaker when multiple orders have the same price.
    seq will increment per order. by order i mean order on the respective side not an order filled against one on the other
    */ 
};
