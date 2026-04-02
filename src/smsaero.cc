#include "smsaero.hh"
#include <cstring>
#include <ctime>
#include <iomanip>
#include <memory>
#include <sstream>
#include <curl/curl.h>

namespace smsaero {

    static constexpr long DEFAULT_TIMEOUT = 30L;
    static constexpr long DEFAULT_CONNECT_TIMEOUT = 10L;
    static constexpr curl_off_t MAX_RESPONSE_SIZE = 10 * 1024 * 1024;

    std::atomic<int> SmsAero::curl_ref_count_{0};

    const std::vector<std::string> SmsAero::GATE_URLS = {
        "gate.smsaero.ru/v2/",
        "gate.smsaero.org/v2/",
        "gate.smsaero.net/v2/",
    };

    SmsAeroError::SmsAeroError(std::string message) : msg_(std::move(message)) {
    }

    const char *SmsAeroError::what() const noexcept {
        return msg_.c_str();
    }

    SmsAeroConnectionError::SmsAeroConnectionError(const std::string &message) : SmsAeroError(message) {
    }

    SmsAero::SmsAero(
        std::string email,
        std::string api_key,
        std::string url_gate,
        std::string signature
    ): email_(std::move(email)), api_key_(std::move(api_key)), url_gate_(std::move(url_gate)),
       signature_(std::move(signature)) {
        if (curl_ref_count_ == 0) {
            curl_global_init(CURL_GLOBAL_DEFAULT);
        }
        curl_ref_count_++;
    }

    static void secure_wipe(std::string &s) {
        if (!s.empty()) {
            volatile char *p = &s[0];
            for (size_t i = 0; i < s.size(); ++i) {
                p[i] = 0;
            }
        }
        s.clear();
    }

    SmsAero::~SmsAero() {
        secure_wipe(email_);
        secure_wipe(api_key_);
        secure_wipe(signature_);

        if (--curl_ref_count_ <= 0) {
            curl_global_cleanup();
            curl_ref_count_ = 0;
        }
    }

    json SmsAero::send_sms(
        const std::string &number,
        const std::string &text,
        const std::string &date_send,
        const std::string &callback_url
    ) {
        json data = {
            {"sign", signature_},
            {"text", text},
            {"callbackUrl", callback_url}
        };

        get_num(number, data);

        if (!date_send.empty()) {
            std::tm tm = {};
            std::istringstream ss(date_send);
            ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
            if (ss.fail()) {
                throw SmsAeroError("param `date` is not in correct format");
            }
            std::time_t const time = std::mktime(&tm);
            data["dateSend"] = static_cast<int64_t>(time);
        }
        return request("sms/send", data);
    }

    json SmsAero::sms_status(const unsigned int sms_id) {
        return request("sms/status", {{"id", sms_id}});
    }

    json SmsAero::sms_list(
        const std::string &number,
        const std::string &text,
        const unsigned int page
    ) {
        json data;
        if (!number.empty()) {
            data["number"] = number;
        }
        if (!text.empty()) {
            data["text"] = text;
        }
        return request("sms/list", data, page);
    }

    json SmsAero::balance() {
        return request("balance");
    }

    json SmsAero::auth() {
        return request("auth");
    }

    json SmsAero::cards() {
        return request("cards");
    }

    json SmsAero::tariffs() {
        return request("tariffs");
    }

    json SmsAero::sign_list(const unsigned int page) {
        return request("sign/list", {}, page);
    }

    json SmsAero::group_add(const std::string &name) {
        return request("group/add", {{"name", name}});
    }

    json SmsAero::group_delete(const unsigned int group_id) {
        return request("group/delete", {{"id", group_id}});
    }

    json SmsAero::group_list(const unsigned int page) {
        return request("group/list", {}, page);
    }

    json SmsAero::blacklist_add(const std::string &number) {
        json data;
        get_num(number, data);
        return request("blacklist/add", data);
    }

    json SmsAero::blacklist_list(
        const std::string &number,
        const unsigned int page
    ) {
        json data;
        if (!number.empty()) {
            data["number"] = number;
        }
        return request("blacklist/list", data, page);
    }

    json SmsAero::blacklist_delete(const unsigned int blacklist_id) {
        return request("blacklist/delete", {{"id", blacklist_id}});
    }

    json SmsAero::hlr_check(const std::string &number) {
        json data;
        get_num(number, data);
        return request("hlr/check", data);
    }

    json SmsAero::hlr_status(const unsigned int hlr_id) {
        return request("hlr/status", {{"id", hlr_id}});
    }

    json SmsAero::contact_add(
        const std::string &number,
        const unsigned int group_id,
        const std::string &birthday,
        const std::string &sex,
        const std::string &lname,
        const std::string &fname,
        const std::string &sname,
        const std::string &param1,
        const std::string &param2,
        const std::string &param3
    ) {
        return request("contact/add", {
                           {"number", number},
                           {"groupId", group_id},
                           {"birthday", birthday},
                           {"sex", sex},
                           {"lname", lname},
                           {"fname", fname},
                           {"sname", sname},
                           {"param1", param1},
                           {"param2", param2},
                           {"param3", param3}
                       });
    }

    json SmsAero::contact_delete(const unsigned int contact_id) {
        return request("contact/delete", {{"id", contact_id}});
    }

    json SmsAero::contact_list(
        const std::string &number,
        const unsigned int group_id,
        const std::string &birthday,
        const std::string &sex,
        const std::string &operator_str,
        const std::string &lname,
        const std::string &fname,
        const std::string &sname,
        const unsigned int page
    ) {
        return request("contact/list", {
                           {"number", number},
                           {"groupId", group_id},
                           {"birthday", birthday},
                           {"sex", sex},
                           {"operator", operator_str},
                           {"lname", lname},
                           {"fname", fname},
                           {"sname", sname}
                       }, page);
    }

    json SmsAero::viber_send(
        const std::string &sign,
        const std::string &channel,
        const std::string &text,
        const std::string &number,
        const unsigned int group_id,
        const std::string &image_source,
        const std::string &text_button,
        const std::string &link_button,
        const std::string &date_send,
        const std::string &sign_sms,
        const std::string &channel_sms,
        const std::string &text_sms,
        const std::string &price_sms
    ) {
        json data = {
            {"groupId", group_id},
            {"sign", sign},
            {"channel", channel},
            {"text", text},
            {"imageSource", image_source},
            {"textButton", text_button},
            {"linkButton", link_button},
            {"dateSend", date_send},
            {"signSms", sign_sms},
            {"channelSms", channel_sms},
            {"textSms", text_sms},
            {"priceSms", price_sms}
        };
        get_num(number, data);
        return request("viber/send", data);
    }

    json SmsAero::viber_sign_list() {
        return request("viber/sign/list");
    }

    json SmsAero::viber_list(const unsigned int page) {
        return request("viber/list", {}, page);
    }

    json SmsAero::send_telegram(
        const std::string &number,
        const int code,
        const std::string &sign,
        const std::string &text
    ) {
        json data = {
            {"code", code}
        };

        get_num(number, data);

        if (!sign.empty()) {
            data["sign"] = sign;
        }
        if (!text.empty()) {
            data["text"] = text;
        }

        return request("telegram/send", data);
    }

    json SmsAero::telegram_status(const unsigned int telegram_id) {
        return request("telegram/status", {{"id", telegram_id}});
    }

    json SmsAero::send_mobile_id(
        const std::string &number,
        const std::string &sign,
        const std::string &callback_url
    ) {
        return request("mobile-id/send", {
            {"number", number},
            {"sign", sign},
            {"callbackUrl", callback_url}
        });
    }

    json SmsAero::mobile_id_status(const unsigned int req_id) {
        return request("mobile-id/status", {{"id", req_id}});
    }

    json SmsAero::verify_mobile_id(
        const unsigned int req_id,
        const std::string &code,
        const std::string &sign
    ) {
        return request("mobile-id/verify", {
            {"id", req_id},
            {"code", code},
            {"sign", sign}
        });
    }

    std::vector<std::string> SmsAero::get_gate_urls() {
        if (!url_gate_.empty()) {
            return {url_gate_};
        }
        return GATE_URLS;
    }

    size_t SmsAero::WriteCallback(void *contents, const size_t size, const size_t nmemb, void *userp) {
        if (nmemb > 0 && size > SIZE_MAX / nmemb) return 0;
        const size_t chunk = size * nmemb;
        auto *buf = static_cast<std::string *>(userp);
        if (buf->size() + chunk > static_cast<size_t>(MAX_RESPONSE_SIZE)) return 0;
        buf->append(static_cast<char *>(contents), chunk);
        return chunk;
    }

    json SmsAero::request(const std::string &selector, const json &data, const unsigned int page) {
        for (const auto &gate : get_gate_urls()) {
            try {
                CURL *raw_curl = curl_easy_init();
                if (!raw_curl) {
                    throw SmsAeroConnectionError("Failed to initialize CURL");
                }

                auto curl_deleter = [](CURL *c) { curl_easy_cleanup(c); };
                std::unique_ptr<CURL, decltype(curl_deleter)> curl(raw_curl, curl_deleter);

                std::string url = "https://";
                url += gate;
                url += selector;

                if (page != 0) {
                    url += "?page=" + std::to_string(page);
                }

                std::string auth = email_ + ":" + api_key_;

                std::string readBuffer;
                curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
                curl_easy_setopt(curl.get(), CURLOPT_USERPWD, auth.c_str());
                curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, WriteCallback);
                curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &readBuffer);
                curl_easy_setopt(curl.get(), CURLOPT_POST, 1L);
                curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT, DEFAULT_TIMEOUT);
                curl_easy_setopt(curl.get(), CURLOPT_CONNECTTIMEOUT, DEFAULT_CONNECT_TIMEOUT);
                curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, 1L);
                curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, 2L);
                curl_easy_setopt(curl.get(), CURLOPT_MAXFILESIZE_LARGE, MAX_RESPONSE_SIZE);

                std::string jsonData = data.dump();
                curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, jsonData.c_str());

                struct curl_slist *raw_headers = nullptr;
                raw_headers = curl_slist_append(raw_headers, "Content-Type: application/json");
                raw_headers = curl_slist_append(raw_headers, "User-Agent: SACppClient/1.0");

                auto slist_deleter = [](curl_slist *s) { curl_slist_free_all(s); };
                std::unique_ptr<curl_slist, decltype(slist_deleter)> headers(raw_headers, slist_deleter);

                curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, headers.get());

                CURLcode const res = curl_easy_perform(curl.get());
                if (res != CURLE_OK) {
                    throw SmsAeroConnectionError(curl_easy_strerror(res));
                }

                json result;
                try {
                    result = json::parse(readBuffer);
                } catch (const json::parse_error &) {
                    throw SmsAeroConnectionError("Invalid JSON response from server");
                }

                check_response(result);
                return result.value("data", json(nullptr));
            } catch (const SmsAeroConnectionError &) {
                continue;
            }
        }
        throw SmsAeroConnectionError("All connection attempts failed");
    }

    void SmsAero::check_response(const json &response) {
        if (!response.contains("success") || !response["success"].is_boolean()) {
            throw SmsAeroError("Invalid API response: missing 'success' field");
        }
        if (!response["success"].get<bool>()) {
            std::string msg = response.value("message", "Unknown API error");
            throw SmsAeroError(msg);
        }
    }

    void SmsAero::get_num(const std::string &number, json &data) {
        if (number.find(',') != std::string::npos) {
            std::vector<std::string> numbers;
            std::istringstream iss(number);
            std::string temp;

            while (std::getline(iss, temp, ',')) {
                numbers.push_back(temp);
            }

            data["numbers"] = numbers;
        } else {
            data["number"] = number;
        }
    }
} // end namespace smsaero
