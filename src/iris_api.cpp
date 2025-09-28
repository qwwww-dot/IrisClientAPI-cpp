#include "iris/iris_api.hpp"
#include <sstream>
#include <stdexcept>
#include <regex>
#include <iostream>

namespace iris {

IrisApi::IrisApi(long botId, const std::string& irisToken, const std::string& baseUrl)
    : botId_(botId)
    , irisToken_(irisToken)
    , curl_(nullptr) {
    
    if (baseUrl.empty()) {
        std::stringstream ss;
        ss << "https://iris-tg.ru/api/" << botId_ << "_" << irisToken_ 
           << "/v" << IRIS_API_VERSION;
        baseUrl_ = ss.str();
    } else {
        baseUrl_ = baseUrl;
    }

    curl_ = curl_easy_init();
    if (!curl_) {
        throw std::runtime_error("Failed to initialize CURL");
    }
}

IrisApi::~IrisApi() {
    if (curl_) {
        curl_easy_cleanup(curl_);
    }
}

size_t IrisApi::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string IrisApi::makeRequest(const std::string& method, 
                               const std::vector<std::pair<std::string, std::string>>& params,
                               bool isPost) {
    if (!curl_) {
        throw std::runtime_error("CURL not initialized");
    }

    std::string url = buildUrl(method);
    std::string postData;
    
    if (!params.empty()) {
        url += "?";
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) url += "&";
            char* escaped_key = curl_easy_escape(curl_, params[i].first.c_str(), 0);
            char* escaped_value = curl_easy_escape(curl_, params[i].second.c_str(), 0);
            url += std::string(escaped_key) + "=" + std::string(escaped_value);
            curl_free(escaped_key);
            curl_free(escaped_value);
        }
    }



    curl_easy_reset(curl_);
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT, 30L);
    
    std::string response;
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);

    if (isPost) {
        curl_easy_setopt(curl_, CURLOPT_POST, 1L);
        curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, ""); 
    }

    char errbuf[CURL_ERROR_SIZE];
    curl_easy_setopt(curl_, CURLOPT_ERRORBUFFER, errbuf);
    errbuf[0] = 0;

    struct curl_slist* headers = nullptr;
    if (isPost) {
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);
    }

    CURLcode res = curl_easy_perform(curl_);
    
    if (headers) {
        curl_slist_free_all(headers);
    }

    if (res != CURLE_OK) {
        std::string error = errbuf[0] ? errbuf : curl_easy_strerror(res);
        throw NetworkException("CURL error (" + std::to_string(res) + "): " + error);
    }

    long http_code = 0;
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &http_code);
    


    if (http_code != 200) {
        std::string error_msg = "HTTP error " + std::to_string(http_code);
        if (!response.empty()) {
            try {
                auto json = nlohmann::json::parse(response);
                if (json.contains("error")) {
                    error_msg += ": " + json["error"].get<std::string>();
                } else if (json.contains("description")) {
                    error_msg += ": " + json["description"].get<std::string>();
                }
            } catch (...) {
                error_msg += ": " + response;
            }
        }
        throw ApiResponseException(error_msg);
    }

    return response;
}

std::string IrisApi::buildUrl(const std::string& method) const {
    return baseUrl_ + "/" + method;
}


std::optional<Response> IrisApi::giveSweets(int count, long userId, 
                                          const std::string& comment,
                                          bool withoutDonateScore) {
    if (count <= 0) {
        throw std::invalid_argument("Count must be positive");
    }
    if (!comment.empty() && comment.length() > 128) {
        throw std::invalid_argument("Comment must not exceed 128 characters");
    }

    std::vector<std::pair<std::string, std::string>> params;
    params.emplace_back("sweets", std::to_string(count));
    params.emplace_back("user_id", std::to_string(userId));
    params.emplace_back("without_donate_score", withoutDonateScore ? "true" : "false");
    if (!comment.empty()) {
        params.emplace_back("comment", comment);
    }

    try {
        auto response = makeRequest("pocket/sweets/give", params, true);

        auto json = nlohmann::json::parse(response);
        Response result;
        
        if (json.contains("error")) {
            result.error = APIError{
                json["error"].contains("code") ? json["error"]["code"].get<int>() : 0,
                json["error"].contains("description") ? 
                    json["error"]["description"].get<std::string>() : "Unknown error"
            };
            result.result = 0;
        } else {
            result.result = json.contains("result") ? json["result"].get<int>() : 0;
        }
        
        return result;
    } catch (const nlohmann::json::parse_error& e) {
#ifdef DEBUG_OUTPUT
        std::cerr << "JSON parse error: " << e.what() << std::endl;
#endif
        return std::nullopt;
    } catch (const std::exception& e) {

        return std::nullopt;
    }
}

std::optional<BalanceData> IrisApi::getBalance() {
    try {
        auto response = makeRequest("pocket/balance");
        return nlohmann::json::parse(response).get<BalanceData>();
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<Response> IrisApi::giveGold(int count, long userId,
                                        const std::string& comment,
                                        bool withoutDonateScore) {
    if (count <= 0) {
        throw std::invalid_argument("Count must be positive");
    }
    if (!comment.empty() && comment.length() > 128) {
        throw std::invalid_argument("Comment must not exceed 128 characters");
    }

    std::vector<std::pair<std::string, std::string>> params = {
        {"gold", std::to_string(count)},
        {"user_id", std::to_string(userId)},
        {"without_donate_score", withoutDonateScore ? "true" : "false"}
    };
    if (!comment.empty()) {
        params.emplace_back("comment", comment);
    }

    try {
        auto response = makeRequest("pocket/gold/give", params, true);
        return nlohmann::json::parse(response).get<Response>();
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<Response> IrisApi::giveDonateScore(int count, long userId,
                                               const std::string& comment) {
    if (count <= 0) {
        throw std::invalid_argument("Count must be positive");
    }
    if (!comment.empty() && comment.length() > 128) {
        throw std::invalid_argument("Comment must not exceed 128 characters");
    }

    std::vector<std::pair<std::string, std::string>> params = {
        {"amount", std::to_string(count)},
        {"user_id", std::to_string(userId)}
    };
    if (!comment.empty()) {
        params.emplace_back("comment", comment);
    }

    try {
        auto response = makeRequest("pocket/donate_score/give", params, true);
        return nlohmann::json::parse(response).get<Response>();
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::vector<HistoryData> IrisApi::getSweetsHistory(int offset) {
    try {
        std::vector<std::pair<std::string, std::string>> params;
        if (offset > 0) {
            params.emplace_back("offset", std::to_string(offset));
        }
        auto response = makeRequest("pocket/sweets/history", params);
        return nlohmann::json::parse(response).get<std::vector<HistoryData>>();
    } catch (const std::exception&) {
        return {};
    }
}

std::vector<HistoryData> IrisApi::getGoldHistory(int offset) {
    try {
        std::vector<std::pair<std::string, std::string>> params;
        if (offset > 0) {
            params.emplace_back("offset", std::to_string(offset));
        }
        auto response = makeRequest("pocket/gold/history", params);
        return nlohmann::json::parse(response).get<std::vector<HistoryData>>();
    } catch (const std::exception&) {
        return {};
    }
}

std::vector<HistoryData> IrisApi::getDonateScoreHistory(int offset) {
    try {
        std::vector<std::pair<std::string, std::string>> params;
        if (offset > 0) {
            params.emplace_back("offset", std::to_string(offset));
        }
        auto response = makeRequest("pocket/donate_score/history", params);
        return nlohmann::json::parse(response).get<std::vector<HistoryData>>();
    } catch (const std::exception&) {
        return {};
    }
}

std::optional<Response> IrisApi::enablePocket(bool enable) {
    try {
        auto response = makeRequest(enable ? "pocket/enable" : "pocket/disable", {}, true);
        return nlohmann::json::parse(response).get<Response>();
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<Response> IrisApi::enableAllPocket(bool enable) {
    try {
        auto response = makeRequest(enable ? "pocket/allow_all" : "pocket/deny_all", {}, true);
        return nlohmann::json::parse(response).get<Response>();
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<Response> IrisApi::allowUserPocket(long userId, bool enable) {
    try {
        std::vector<std::pair<std::string, std::string>> params = {
            {"user_id", std::to_string(userId)}
        };
        auto response = makeRequest(enable ? "pocket/allow_user" : "pocket/deny_user", params, true);
        return nlohmann::json::parse(response).get<Response>();
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::vector<UpdatesLog> IrisApi::getUpdates(int offset, int limit) {
    try {
        std::vector<std::pair<std::string, std::string>> params;
        if (offset > 0) params.emplace_back("offset", std::to_string(offset));
        if (limit > 0) params.emplace_back("limit", std::to_string(limit));
        
        auto response = makeRequest("getUpdates", params, true);
        return nlohmann::json::parse(response).get<std::vector<UpdatesLog>>();
    } catch (const std::exception&) {
        return {};
    }
}

std::vector<long> IrisApi::getIrisAgents() {
    try {
        auto response = makeRequest("iris_agents");
        return nlohmann::json::parse(response).get<std::vector<long>>();
    } catch (const std::exception&) {
        return {};
    }
}

std::optional<UserRegInfo> IrisApi::checkUserReg(long userId) {
    try {
        std::vector<std::pair<std::string, std::string>> params = {
            {"user_id", std::to_string(userId)}
        };
        auto response = makeRequest("user_info/reg", params, true);
        return nlohmann::json::parse(response).get<UserRegInfo>();
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<UserSpamInfo> IrisApi::checkUserSpam(long userId) {
    try {
        std::vector<std::pair<std::string, std::string>> params = {
            {"user_id", std::to_string(userId)}
        };
        auto response = makeRequest("user_info/spam", params, true);
        return nlohmann::json::parse(response).get<UserSpamInfo>();
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<UserActivityInfo> IrisApi::checkUserActivity(long userId) {
    try {
        std::vector<std::pair<std::string, std::string>> params = {
            {"user_id", std::to_string(userId)}
        };
        auto response = makeRequest("user_info/activity", params, true);
        return nlohmann::json::parse(response).get<UserActivityInfo>();
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<UserStarsInfo> IrisApi::checkUserStars(long userId) {
    try {
        std::vector<std::pair<std::string, std::string>> params = {
            {"user_id", std::to_string(userId)}
        };
        auto response = makeRequest("user_info/stars", params, true);
        return nlohmann::json::parse(response).get<UserStarsInfo>();
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<UserPocketInfo> IrisApi::checkUserPocket(long userId) {
    try {
        std::vector<std::pair<std::string, std::string>> params = {
            {"user_id", std::to_string(userId)}
        };
        auto response = makeRequest("user_info/pocket", params, true);
        return nlohmann::json::parse(response).get<UserPocketInfo>();
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<BuyTradesResponse> IrisApi::buyTrade(double price, int volume) {
    if (price < 0.01 || price > 1000000.0) {
        throw std::invalid_argument("Price must be between 0.01 and 1,000,000");
    }

    try {
        std::vector<std::pair<std::string, std::string>> params = {
            {"price", std::to_string(price)},
            {"volume", std::to_string(volume)}
        };
        auto response = makeRequest("trade/buy", params);
        return nlohmann::json::parse(response).get<BuyTradesResponse>();
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<SellTradesResponse> IrisApi::sellTrade(double price, int volume) {
    if (price < 0.01 || price > 1000000.0) {
        throw std::invalid_argument("Price must be between 0.01 and 1,000,000");
    }

    try {
        std::vector<std::pair<std::string, std::string>> params = {
            {"price", std::to_string(price)},
            {"volume", std::to_string(volume)}
        };
        auto response = makeRequest("trade/sell", params);
        return nlohmann::json::parse(response).get<SellTradesResponse>();
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<OrdersResponse> IrisApi::getOrdersTrade() {
    try {
        auto response = makeRequest("trade/my_orders");
        return nlohmann::json::parse(response).get<OrdersResponse>();
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<CancelTradesResponse> IrisApi::cancelPriceTrade(double price) {
    if (price < 0.01 || price > 1000000.0) {
        throw std::invalid_argument("Price must be between 0.01 and 1,000,000");
    }

    try {
        std::vector<std::pair<std::string, std::string>> params = {
            {"price", std::to_string(price)}
        };
        auto response = makeRequest("trade/cancel_price", params);
        return nlohmann::json::parse(response).get<CancelTradesResponse>();
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<CancelTradesResponse> IrisApi::cancelAllTrade() {
    try {
        auto response = makeRequest("trade/cancel_all");
        return nlohmann::json::parse(response).get<CancelTradesResponse>();
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<CancelTradesResponse> IrisApi::cancelPartTrade(int id, int volume) {
    try {
        std::vector<std::pair<std::string, std::string>> params = {
            {"id", std::to_string(id)},
            {"volume", std::to_string(volume)}
        };
        auto response = makeRequest("trade/cancel_part", params);
        return nlohmann::json::parse(response).get<CancelTradesResponse>();
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::string IrisApi::generateDeepLink(Currency currency, int count,
                                    const std::string& comment) {
    if (count <= 0) {
        throw std::invalid_argument("Count must be positive");
    }

    if (!comment.empty()) {
        if (comment.length() > 128) {
            throw std::invalid_argument("Comment must not exceed 128 characters");
        }
        
        static const std::regex pattern("^[a-zA-Z0-9_]+$");
        if (!std::regex_match(comment, pattern)) {
            throw std::invalid_argument("Comment can only contain letters, numbers and underscore");
        }
    }

    std::stringstream url;
    url << "https://t.me/iris_black_bot?start=";
    
    switch (currency) {
        case Currency::GOLD:
            url << "givegold_bot";
            break;
        case Currency::SWEETS:
            url << "give_bot";
            break;
        case Currency::DONATE_SCORE:
            url << "givedonate_score_bot";
            break;
    }

    url << botId_ << "_" << count;
    
    if (!comment.empty()) {
        url << "_" << comment;
    }

    return url.str();
}

std::string IrisApi::generateBotPermissionsDeepLink(
    const std::vector<BotPermission>& permissions) {
    std::stringstream url;
    url << "https://t.me/iris_black_bot?start=request_rights_" << botId_;

    for (const auto& permission : permissions) {
        url << "_";
        switch (permission) {
            case BotPermission::REG:
                url << "reg";
                break;
            case BotPermission::ACTIVITY:
                url << "activity";
                break;
            case BotPermission::SPAM:
                url << "spam";
                break;
            case BotPermission::STARS:
                url << "stars";
                break;
            case BotPermission::POCKET:
                url << "pocket";
                break;
        }
    }

    return url.str();
}

} // namespace iris