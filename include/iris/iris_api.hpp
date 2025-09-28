#pragma once

#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include "models.hpp"
#include "exceptions.hpp"

namespace iris {


enum class Currency {
    GOLD,
    SWEETS,
    DONATE_SCORE
};

enum class BotPermission {
    REG,
    ACTIVITY,
    SPAM,
    STARS,
    POCKET
};

class IrisApi {
public:
    IrisApi(long botId, const std::string& irisToken, 
            const std::string& baseUrl = "");
    ~IrisApi();

    std::optional<Response> giveSweets(int count, long userId, 
                                     const std::string& comment = "", 
                                     bool withoutDonateScore = true);
    
    std::optional<Response> giveGold(int count, long userId,
                                   const std::string& comment = "",
                                   bool withoutDonateScore = true);
    
    std::optional<Response> giveDonateScore(int count, long userId,
                                          const std::string& comment = "");

    std::optional<BalanceData> getBalance();
    std::vector<HistoryData> getSweetsHistory(int offset = 0);
    std::vector<HistoryData> getGoldHistory(int offset = 0);
    std::vector<HistoryData> getDonateScoreHistory(int offset = 0);

    std::optional<Response> enablePocket(bool enable = true);
    std::optional<Response> enableAllPocket(bool enable = true);
    std::optional<Response> allowUserPocket(long userId, bool enable);

    std::vector<UpdatesLog> getUpdates(int offset = 0, int limit = 0);
    std::vector<long> getIrisAgents();

    std::string generateDeepLink(Currency currency, int count, 
                               const std::string& comment = "");
    std::string generateBotPermissionsDeepLink(
        const std::vector<BotPermission>& permissions);

    std::optional<UserRegInfo> checkUserReg(long userId);
    std::optional<UserSpamInfo> checkUserSpam(long userId);
    std::optional<UserActivityInfo> checkUserActivity(long userId);
    std::optional<UserStarsInfo> checkUserStars(long userId);
    std::optional<UserPocketInfo> checkUserPocket(long userId);

    std::optional<BuyTradesResponse> buyTrade(double price, int volume);
    std::optional<SellTradesResponse> sellTrade(double price, int volume);
    std::optional<OrdersResponse> getOrdersTrade();
    std::optional<CancelTradesResponse> cancelPriceTrade(double price);
    std::optional<CancelTradesResponse> cancelAllTrade();
    std::optional<CancelTradesResponse> cancelPartTrade(int id, int volume);

private:
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    std::string makeRequest(const std::string& method, 
                          const std::vector<std::pair<std::string, std::string>>& params = {}, 
                          bool isPost = false);
    std::string buildUrl(const std::string& method) const;
    
    long botId_;
    std::string irisToken_;
    std::string baseUrl_;
    CURL* curl_;
    static constexpr const char* IRIS_API_VERSION = "0.3";
};

} // namespace iris