# SmsAero C++ Api client

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

Library for sending SMS messages using the SmsAero API. Written in C++.

## Usage example:

Get credentials from account settings page: https://smsaero.ru/cabinet/settings/apikey/

```cpp
#include <iostream>
#include "smsaero.hh"

const std::string SMSAERO_EMAIL = "your email";
const std::string SMSAERO_API_KEY = "your api key";

int main() {
    try {
        smsaero::SmsAero smsAero(SMSAERO_EMAIL, SMSAERO_API_KEY);
        const smsaero::json result = smsAero.send("70000000000", "Hello, world!");
        std::cout << result << std::endl;
    } catch (const smsaero::SmsAeroError& e) {
        std::cerr << "SmsAero error: " << e.what() << std::endl;
    }
    return 0;
}
```

## Install required libraries:

* libcurl
* nlohmann/json

```bash
sudo apt-get install -y libcurl4-openssl-dev nlohmann-json3-dev libgtest-dev
```


## Build example:

```bash
git clone https://github.com/smsaero/smsaero_cpp.git
cd smsaero_cpp
make
```

## Run example:

```bash
SMSAERO_EMAIL="your email"
SMSAERO_API_KEY="your api key"

./bin/demo -u "$SMSAERO_EMAIL" -t "$SMSAERO_API_KEY" -n 70000000000,70000000001 -m 'Hello, World!' | jq .
```

## Run on Docker:

```bash
docker pull 'smsaero/smsaero_cpp:latest'
docker run -it --rm 'smsaero/smsaero_cpp:latest' smsaero_send -e "your email" -t "your api key" -n 79038805678 -m 'Hello, World!'
```


## License

```
MIT License
```
