#include "DXFeed.h"
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <iostream>
#include <thread>

#define LS(s) LS2(s)
#define LS2(s) L##s

#include <string>

#ifdef _MSC_FULL_VER
#pragma warning(push)
#pragma warning(disable : 4244)
#endif

#include <codecvt>

namespace dxf {
    struct StringConverter {
        static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> wstringConvert;

        static std::wstring utf8ToWString(const std::string &utf8) noexcept {
            try {
                return wstringConvert.from_bytes(utf8);
            } catch (...) { return {}; }
        }

        static std::wstring utf8ToWString(const char *utf8) noexcept {
            if (utf8 == nullptr) { return {}; }

            try {
                return wstringConvert.from_bytes(utf8);
            } catch (...) { return {}; }
        }

        static wchar_t utf8ToWChar(char c) noexcept {
            if (c == '\0') { return L'\0'; }

            return utf8ToWString(std::string(1, c))[0];
        }

        static std::string wStringToUtf8(const std::wstring &utf16) noexcept {
            try {
                return wstringConvert.to_bytes(utf16);
            } catch (...) { return {}; }
        }

        static std::string wStringToUtf8(const wchar_t *utf16) noexcept {
            if (utf16 == nullptr) { return {}; }

            try {
                return wstringConvert.to_bytes(utf16);
            } catch (...) { return {}; }
        }

        static char wCharToUtf8(wchar_t c) noexcept {
            if (c == L'\0') { return '\0'; }

            return wStringToUtf8(std::wstring(1, c))[0];
        }
    };

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> StringConverter::wstringConvert{};

}// namespace dxf

#ifdef _MSC_FULL_VER
#pragma warning(pop)
#endif

inline std::string formatLocalTime(long long timestamp, const std::string &format = "%Y-%m-%d %H:%M:%S") {
    return fmt::format(fmt::format("{{:{}}}", format), fmt::localtime(static_cast<std::time_t>(timestamp)));
}

inline std::string formatLocalTimestampWithMillis(long long timestamp,
                                                  const std::string &format = "%Y-%m-%d %H:%M:%S") {
    long long ms = timestamp % 1000;

    return fmt::format("{}.{:0>3}", formatLocalTime(timestamp / 1000, format), ms);
}

int main() {
    dxf_connection_t con = nullptr;
    auto r = dxf_create_connection("demo.dxfeed.com:7300", nullptr, nullptr, nullptr, nullptr, nullptr, &con);

    if (r == DXF_FAILURE) { return 1; }

    dxf_subscription_t sub = nullptr;

    r = dxf_create_subscription_timed(con, DXF_ET_TIME_AND_SALE, 0, &sub);

    if (r == DXF_FAILURE) { return 2; }

    dxf_attach_event_listener(
            sub,
            [](int eventType, dxf_const_string_t symbol, const dxf_event_data_t *data, int /* always = 1 */, void *) {
                auto out = fmt::memory_buffer();

                fmt::format_to(std::back_inserter(out), "{}{{symbol={}, ",
                               dxf::StringConverter::wStringToUtf8(dx_event_type_to_string(eventType)),
                               dxf::StringConverter::wStringToUtf8(symbol));
                if (eventType == DXF_ET_TIME_AND_SALE) {
                    // TODO: std::bit_cast
                    auto tns = reinterpret_cast<const dxf_time_and_sale_t *>(data);

                    //enum -> char[] helpers
                    static const char *orderSide[] = {"Undefined", "Buy", "Sell"};
                    static const char *tnsType[] = {"New", "Correction", "Cancel"};
                    static const char *orderScope[] = {"Composite", "Regional", "Aggregate", "Order"};

                    fmt::format_to(std::back_inserter(out),
                                   "index={}, time={}, exchange={}, price={:.6f}, size={:.6f}, bid price={:.6f}, "
                                   "ask price={:.6f}, exchange sale conditions={}, buyer={}, seller={}, side={}, "
                                   "type={}, is valid tick={}, is ETH trade={}, Trade Through Exempt={}, is spread "
                                   "leg={}, scope={}, event flags={:#X}, raw_flags={:#X}",
                                   tns->index, formatLocalTimestampWithMillis(tns->time),
                                   dxf::StringConverter::wCharToUtf8(tns->exchange_code), tns->price, tns->size,
                                   tns->bid_price, tns->ask_price,
                                   dxf::StringConverter::wStringToUtf8(tns->exchange_sale_conditions),
                                   dxf::StringConverter::wStringToUtf8(tns->buyer),
                                   dxf::StringConverter::wStringToUtf8(tns->seller), orderSide[tns->side],
                                   tnsType[tns->type], tns->is_valid_tick ? "True" : "False",
                                   tns->is_eth_trade ? "True" : "False",
                                   dxf::StringConverter::wCharToUtf8(tns->trade_through_exempt),
                                   tns->is_spread_leg ? "True" : "False", orderScope[tns->scope], tns->event_flags,
                                   tns->raw_flags);
                }

                fmt::format_to(std::back_inserter(out), "}}");
                std::cout << to_string(out) << std::endl;
            },
            nullptr);

    dxf_add_symbol(sub, L"ETH/USD:GDAX");

    std::cout << "Press Enter to stop" << std::endl;
    std::cin.get();

    return 0;
}
