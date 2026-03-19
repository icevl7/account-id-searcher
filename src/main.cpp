#include <Geode/Geode.hpp>
#include <Geode/modify/LevelSearchLayer.hpp>
#include <Geode/binding/ProfilePage.hpp>
#include <cctype>
#include <climits>
#include <string>
#include <string_view>
#include <algorithm>

using namespace geode::prelude;

bool isNumericOnly(const std::string& s) {
    if (s.empty()) return false;

    for (char c : s) {
        if (!std::isdigit(static_cast<unsigned char>(c))) {
            
            std::string_view sv = s;

            size_t first = sv.find_first_not_of(' ');
            
            if (first == std::string_view::npos) return false;

            size_t last = sv.find_last_not_of(' ');
            std::string_view trimmed = sv.substr(first, last - first + 1);

            if (std::ranges::all_of(trimmed, [](unsigned char ch) { 
                return std::isdigit(ch); 
            })) {
                return true;
            }

            return false;
        }
    }

    return true;
}

bool parseSafeInt32(const std::string& s, int& out) {
    try {
        long long val = std::stoll(s);

        if (val < 0 || val > INT_MAX)
            return false;

        out = (int)val;
        return true;
    }
    catch (...) {
        return false;
    }
}

class $modify(MyLevelSearchLayer, LevelSearchLayer) {

    struct Fields {
        bool waitingChoice = false;
        bool skipNextCheck = false;
        CCObject* lastSender = nullptr;
    };

    void onSearchUser(CCObject* sender) {

        if (m_fields->skipNextCheck) {
            m_fields->skipNextCheck = false;
            LevelSearchLayer::onSearchUser(sender);
            return;
        }

        std::string query = m_searchInput->getString();

        if (isNumericOnly(query)) {

            int parsedID;

            if (Mod::get()->getSettingValue<bool>("force-account-search")) {

                if (!parseSafeInt32(query, parsedID)) {
                    FLAlertLayer::create(
                        this,
                        "Invalid ID",
                        "Number is too large",
                        "OK",
                        nullptr
                    )->show();
                    return;
                }

                ProfilePage::create(parsedID, false)->show();
                return;
            }

            m_fields->waitingChoice = true;
            m_fields->lastSender = sender;

            FLAlertLayer::create(
                this,
                "Account Search",
                "Choose search type",
                "By Account ID",
                "By UserID"
            )->show();

            return;
        }

        LevelSearchLayer::onSearchUser(sender);
    }

    void FLAlert_Clicked(FLAlertLayer*, bool btn2) {

        if (!m_fields->waitingChoice) return;

        m_fields->waitingChoice = false;

        std::string query = m_searchInput->getString();
        int parsedID;

        if (!parseSafeInt32(query, parsedID)) {
            FLAlertLayer::create(
                this,
                "Invalid ID",
                "Number is too large",
                "OK",
                nullptr
            )->show();
            return;
        }

        if (btn2) {
            m_fields->skipNextCheck = true;
            LevelSearchLayer::onSearchUser(m_fields->lastSender);
        }
        else {
            ProfilePage::create(parsedID, false)->show();
        }
    }
};
