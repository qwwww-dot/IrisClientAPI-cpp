# IrisCPP Client API

A C++ client library for interacting with the Iris API. This is a C++ port of the original Kotlin IrisClientAPI.

## Requirements

- C++17 or higher
- CMake 3.14 or higher
- libcurl
- nlohmann_json

## Building

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Usage Example

```cpp
#include <iris/iris_api.hpp>

int main() {
    iris::IrisApi api("your-api-key");
    
    try {
        auto userInfo = api.getUserInfo();
        std::cout << "User balance: " << userInfo.balance << std::endl;
    } catch (const iris::IrisApiException& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    return 0;
}
```

## License

This project is licensed under the same terms as the original IrisClientAPI.