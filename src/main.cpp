#include <Geode/Geode.hpp>
#include <Geode/modify/LevelSearchLayer.hpp>
#include <Geode/binding/ProfilePage.hpp>
#include <Geode/utils/string.hpp>

using namespace geode::prelude;

static bool getNumericIntent(std::string const& query, int& out, bool& parseFail) {
    auto trimmed = string::trim(query);

    if (trimmed.empty())
        return false;

    if (!std::ranges::all_of(trimmed, [](unsigned char c){
        return std::isdigit(c);
    }))
        return false;

    auto res = numFromString<int>(trimmed);
    if (!res) {
        parseFail = true;
        return true;
    }

    int val = res.unwrap();
    if (val < 0) {
        parseFail = true;
        return true;
    }

    out = val;
    return true;
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

        int parsedID = 0;
        bool parseFail = false;

        if (getNumericIntent(query, parsedID, parseFail)) {

            if (parseFail) {
                FLAlertLayer::create(
                    this,
                    "Invalid ID",
                    "Number is too large",
                    "OK",
                    nullptr
                )->show();
                return;
            }

            if (Mod::get()->getSettingValue<bool>("force-account-search")) {
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

        int parsedID = 0;
        bool parseFail = false;
        
        // Additional validation in case the search input changes while the popup is open
        // (normally not possible in vanilla UI, but kept for safety and mod compatibility)
        if (!getNumericIntent(query, parsedID, parseFail) || parseFail) {
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
