#ifndef SMSAERO_H
#define SMSAERO_H

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace smsaero {
    using json = nlohmann::json;

    class SmsAeroError : public std::exception {
    public:
        explicit SmsAeroError(std::string message);

        const char *what() const noexcept override;

    private:
        std::string msg_;
    };

    class SmsAeroHTTPError final : public SmsAeroError {
    public:
        explicit SmsAeroHTTPError(const std::string &message);
    };

    class SmsAeroConnectionError final : public SmsAeroError {
    public:
        explicit SmsAeroConnectionError(const std::string &message);
    };

    class SmsAero {
    public:
        SmsAero(
            std::string email,
            std::string api_key,
            std::string url_gate = "",
            std::string signature = "Sms Aero"
        );

        ~SmsAero();

        json send_sms(
            const std::string &number,
            const std::string &text,
            const std::string &date_send = "",
            const std::string &callback_url = "");

        json sms_status(unsigned int sms_id);

        json sms_list(
            const std::string &number = "",
            const std::string &text = "",
            unsigned int page = 0);

        json balance();

        json auth();

        json cards();

        json tariffs();

        json sign_list(unsigned int page = 0);

        json group_add(const std::string &name);

        json group_delete(unsigned int group_id);

        json group_list(unsigned int page = 0);

        json blacklist_add(const std::string &number);

        json blacklist_list(
            const std::string &number = "",
            unsigned int page = 0);

        json blacklist_delete(unsigned int blacklist_id);

        json hlr_check(const std::string &number);

        json hlr_status(unsigned int hlr_id);

        json contact_add(
            const std::string &number,
            unsigned int group_id = 0,
            const std::string &birthday = "",
            const std::string &sex = "",
            const std::string &lname = "",
            const std::string &fname = "",
            const std::string &sname = "",
            const std::string &param1 = "",
            const std::string &param2 = "",
            const std::string &param3 = "");

        json contact_delete(unsigned int contact_id);

        json contact_list(
            const std::string &number = "",
            unsigned int group_id = 0,
            const std::string &birthday = "",
            const std::string &sex = "",
            const std::string &operator_str = "",
            const std::string &lname = "",
            const std::string &fname = "",
            const std::string &sname = "",
            unsigned int page = 0);

        json viber_send(
            const std::string &sign,
            const std::string &channel,
            const std::string &text,
            const std::string &number = "",
            unsigned int group_id = 0,
            const std::string &image_source = "",
            const std::string &text_button = "",
            const std::string &link_button = "",
            const std::string &date_send = "",
            const std::string &sign_sms = "",
            const std::string &channel_sms = "",
            const std::string &text_sms = "",
            const std::string &price_sms = "");

        json viber_sign_list();

        json viber_list(unsigned int page = 0);

    private:
        std::string email_;
        std::string api_key_;
        std::string url_gate_;
        std::string signature_;

        std::vector<std::string> GATE_URLS = {
            "@gate.smsaero.ru/v2/",
            "@gate.smsaero.org/v2/",
            "@gate.smsaero.net/v2/",
        };

        std::vector<std::string> get_gate_urls();

        static size_t WriteCallback(
            void *contents,
            size_t size,
            size_t nmemb,
            void *userp);

        json request(
            const std::string &selector,
            const json &data = {},
            unsigned int page = 0,
            std::string proto = "https"
        );

        static void check_response(json response);

        static void get_num(const std::string &number, json &data);
    };
} // end namespace smsaero

#endif // SMSAERO_H
