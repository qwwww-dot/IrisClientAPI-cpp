# IrisCPP Client API

C++ клиент для работы с API сервиса Iris, который позволяет управлять ирисками, Ирис голд и очками доната в Telegram боте @iris_black_bot.

## Начало работы

1. Получите токен от @iris_black_bot, отправив команду `+ирис коннект`
2. Запомните ID вашего бота (можно получить у @BotFather)
3. Используйте эти данные при инициализации API клиента

## Возможности

- 🍬 Управление ирисками (отправка, получение баланса, история)
- 🏆 Управление Ирис голд (отправка, получение баланса, история)
- 💰 Управление очками доната (отправка, получение баланса, история)
- 📊 Получение информации о пользователях
- 💹 Торговые операции (покупка/продажа, управление ордерами)

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

## Примеры использования

### Инициализация

```cpp
#include <iris/iris_api.hpp>

int main() {
    // Создание экземпляра API клиента
    // bot_id - ID вашего бота
    // token - токен от @iris_black_bot
    iris::IrisApi api(bot_id, "your-iris-token");
}
```

### Получение информации о балансе

```cpp
try {
    auto userInfo = api.getUserInfo();
    if (userInfo.balance) {
        std::cout << "Баланс ирисок: " << userInfo.balance->sweets << std::endl;
        std::cout << "Баланс Ирис голд: " << userInfo.balance->gold << std::endl;
    }
} catch (const iris::IrisApiException& e) {
    std::cerr << "Ошибка: " << e.what() << std::endl;
}
```

### Работа с ирисками

```cpp
// Отправка ирисок
try {
    auto response = api.giveSweets(100, recipient_id, "За помощь");
    if (response && response->success) {
        std::cout << "Ириски успешно отправлены!" << std::endl;
    }
} catch (const iris::IrisApiException& e) {
    std::cerr << "Ошибка при отправке ирисок: " << e.what() << std::endl;
}

// История операций с ирисками
try {
    auto history = api.getSweetsHistory(0); // 0 - начальная страница
    for (const auto& transaction : history) {
        std::cout << "ID пользователя: " << transaction.user_id << std::endl;
        std::cout << "Количество: " << transaction.amount << std::endl;
        std::cout << "Время: " << transaction.timestamp << std::endl;
        if (transaction.comment) {
            std::cout << "Комментарий: " << *transaction.comment << std::endl;
        }
    }
} catch (const iris::IrisApiException& e) {
    std::cerr << "Ошибка при получении истории: " << e.what() << std::endl;
}
```

### Торговые операции

```cpp
// Покупка Ирис голд
try {
    auto buyResponse = api.buyTrade(0.5, 100); // Цена, количество
    if (buyResponse && buyResponse->done_volume > 0) {
        std::cout << "Куплено Ирис голд: " << buyResponse->done_volume << std::endl;
        std::cout << "Потрачено ирисок: " << buyResponse->sweets_spent << std::endl;
    }
} catch (const iris::IrisApiException& e) {
    std::cerr << "Ошибка при покупке: " << e.what() << std::endl;
}

// Получение списка ордеров
try {
    auto orders = api.getOrdersTrade();
    if (orders) {
        // Ордера на покупку
        for (const auto& order : orders->buy) {
            if (order.price && order.volume) {
                std::cout << "Покупка: " << *order.volume 
                         << " по цене " << *order.price << std::endl;
            }
        }
        // Ордера на продажу
        for (const auto& order : orders->sell) {
            if (order.price && order.volume) {
                std::cout << "Продажа: " << *order.volume 
                         << " по цене " << *order.price << std::endl;
            }
        }
    }
} catch (const iris::IrisApiException& e) {
    std::cerr << "Ошибка при получении ордеров: " << e.what() << std::endl;
}
```

### Отмена ордеров

```cpp
try {
    auto response = api.cancelTrade();
    if (response && response->success) {
        std::cout << "Все ордера успешно отменены" << std::endl;
    }
} catch (const iris::IrisApiException& e) {
    std::cerr << "Ошибка при отмене ордеров: " << e.what() << std::endl;
}
```

### Получение обновлений

```cpp
try {
    auto updates = api.getUpdates();
    if (updates) {
        if (updates->balance) {
            std::cout << "Новый баланс ирисок: " << updates->balance->sweets << std::endl;
        }
        if (!updates->trades.empty()) {
            std::cout << "Количество новых торговых операций: " << updates->trades.size() << std::endl;
        }
    }
} catch (const iris::IrisApiException& e) {
    std::cerr << "Ошибка при получении обновлений: " << e.what() << std::endl;
}
```

## Обработка ошибок

Библиотека использует исключения для обработки ошибок:

```cpp
try {
    auto response = api.giveSweets(100, recipient_id);
} catch (const iris::NetworkException& e) {
    // Ошибки сети (проблемы с подключением, таймауты и т.д.)
    std::cerr << "Ошибка сети: " << e.what() << std::endl;
} catch (const iris::ApiResponseException& e) {
    // Ошибки API (неверный токен, недостаточно средств и т.д.)
    std::cerr << "Ошибка API: " << e.what() << std::endl;
} catch (const iris::IrisApiException& e) {
    // Общие ошибки API
    std::cerr << "Ошибка Iris API: " << e.what() << std::endl;
} catch (const std::exception& e) {
    // Другие ошибки
    std::cerr << "Неизвестная ошибка: " << e.what() << std::endl;
}
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
