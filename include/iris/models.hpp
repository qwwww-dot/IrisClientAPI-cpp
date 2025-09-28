#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <optional>
#include <regex>

namespace iris {

struct APIError {
    int code;
    std::string description;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(APIError, code, description)
};

struct Response {
    int result;
    std::optional<APIError> error;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Response, result, error)
};

struct BalanceData {
    int gold;
    double sweets;
    int donate_score;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(BalanceData, gold, sweets, donate_score)
};

struct HistoryData {
    long user_id;
    std::string type;
    int amount;
    std::optional<std::string> comment;
    long timestamp;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(HistoryData, user_id, type, amount, comment, timestamp)
};

struct UpdatesLog {
    long update_id;
    std::string type;
    long user_id;
    int amount;
    std::optional<std::string> comment;
    long timestamp;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(UpdatesLog, update_id, type, user_id, amount, comment, timestamp)
};

struct UserRegInfo {
    long timestamp;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(UserRegInfo, timestamp)
};

struct UserSpamInfo {
    bool spam;
    bool ignore;
    bool scam;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(UserSpamInfo, spam, ignore, scam)
};

struct UserActivityInfo {
    int messages;
    int characters;
    int forwarded;
    int replies;
    int mentions;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(UserActivityInfo, messages, characters, forwarded, replies, mentions)
};

struct UserStarsInfo {
    int stars;
    std::string rank;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(UserStarsInfo, stars, rank)
};

struct UserPocketInfo {
    int gold;
    double sweets;
    int donate_score;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(UserPocketInfo, gold, sweets, donate_score)
};

struct OrderBuyTradesResponse {
    std::optional<int> id;
    std::optional<int> volume;
    std::optional<double> price;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(OrderBuyTradesResponse, id, volume, price)
};

struct BuyTradesResponse {
    int done_volume;
    double sweets_spent;
    std::optional<OrderBuyTradesResponse> new_order;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(BuyTradesResponse, done_volume, sweets_spent, new_order)
};

struct OrderSellTradesResponse {
    std::optional<int> id;
    std::optional<int> volume;
    std::optional<double> price;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(OrderSellTradesResponse, id, volume, price)
};

struct SellTradesResponse {
    int done_volume;
    double sweets_earned;
    std::optional<OrderSellTradesResponse> new_order;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(SellTradesResponse, done_volume, sweets_earned, new_order)
};

struct OrdersResponse {
    std::vector<OrderBuyTradesResponse> buy;
    std::vector<OrderSellTradesResponse> sell;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(OrdersResponse, buy, sell)
};

struct CancelTradesResponse {
    std::vector<int> cancelled_orders;
    int cancelled_volume;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(CancelTradesResponse, cancelled_orders, cancelled_volume)
};

} // namespace iris