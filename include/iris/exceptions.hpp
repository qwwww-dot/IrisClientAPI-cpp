#pragma once

#include <string>
#include <stdexcept>

namespace iris {

class IrisApiException : public std::runtime_error {
public:
    explicit IrisApiException(const std::string& message) : std::runtime_error(message) {}
};

class InvalidApiKeyException : public IrisApiException {
public:
    explicit InvalidApiKeyException() : IrisApiException("Invalid API key provided") {}
};

class NetworkException : public IrisApiException {
public:
    explicit NetworkException(const std::string& message) : IrisApiException("Network error: " + message) {}
};

class ApiResponseException : public IrisApiException {
public:
    explicit ApiResponseException(const std::string& message) : IrisApiException("API response error: " + message) {}
};

} // namespace iris