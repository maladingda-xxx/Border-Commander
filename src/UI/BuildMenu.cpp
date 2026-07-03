#include "UI/BuildMenu.h"
#include "Entity/Building.h"
#include "Resource/ResourceManager.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>

namespace {

constexpr float PANEL_X = 820.0f;
constexpr float PANEL_Y = 20.0f;
constexpr float PANEL_W = 190.0f;
constexpr float BTN_W = 170.0f;
constexpr float BTN_H = 50.0f;
constexpr float BTN_GAP = 4.0f;
constexpr float BTN_START_Y = 48.0f;
constexpr float TITLE_Y = 24.0f;

sf::Color fromHex(uint32_t hex) {
    return sf::Color(
        static_cast<uint8_t>((hex >> 16) & 0xFF),
        static_cast<uint8_t>((hex >> 8) & 0xFF),
        static_cast<uint8_t>(hex & 0xFF),
        static_cast<uint8_t>((hex >> 24) & 0xFF)
    );
}

} // anonymous namespace

BuildMenu::BuildMenu(const sf::Font& font, ResourceManager* resources)
    : m_font(font)
    , m_resources(resources) {
    buildLayout();
}

void BuildMenu::buildLayout() {
    m_buttons.clear();

    const BuildingType types[] = {
        BuildingType::Headquarters, BuildingType::Barracks,
        BuildingType::Farm, BuildingType::GoldMine
    };
    const char* hotkeys[] = {"1", "2", "3", "4"};

    for (int i = 0; i < 4; ++i) {
        float by = BTN_START_Y + i * (BTN_H + BTN_GAP);
        Building temp(types[i]);
        auto cost = temp.getCost();

        std::string costStr;
        if (cost.get(ResourceType::Gold) > 0) {
            costStr = std::to_string(cost.get(ResourceType::Gold)) + "G";
        }
        if (cost.get(ResourceType::Food) > 0) {
            if (!costStr.empty()) costStr += " ";
            costStr += std::to_string(cost.get(ResourceType::Food)) + "F";
        }
        if (costStr.empty()) {
            costStr = "Free";
        }

        Button btn;
        btn.bounds = sf::FloatRect({PANEL_X + (PANEL_W - BTN_W) / 2.0f, by}, {BTN_W, BTN_H});
        btn.label = std::string("[") + hotkeys[i] + "] " + temp.getName();
        btn.costText = costStr;
        btn.buildingType = types[i];
        m_buttons.push_back(btn);
    }
}

void BuildMenu::update() {
    refreshAffordability();
}

void BuildMenu::refreshAffordability() {
    for (auto& btn : m_buttons) {
        Building temp(btn.buildingType);
        auto cost = temp.getCost();
        btn.enabled = m_resources->canAfford(cost);
    }
}

void BuildMenu::handleMouseMove(const sf::Vector2i& pos) {
    m_hoveredIdx = -1;
    sf::Vector2f fpos(static_cast<float>(pos.x), static_cast<float>(pos.y));
    for (int i = 0; i < static_cast<int>(m_buttons.size()); ++i) {
        if (m_buttons[i].bounds.contains(fpos)) {
            m_hoveredIdx = i;
            break;
        }
    }
}

bool BuildMenu::handleClick(const sf::Vector2i& pos) {
    sf::Vector2f fpos(static_cast<float>(pos.x), static_cast<float>(pos.y));
    for (int i = 0; i < static_cast<int>(m_buttons.size()); ++i) {
        if (m_buttons[i].bounds.contains(fpos) && m_buttons[i].enabled) {
            m_selectedIdx = i;
            m_placementMode = true;
            m_selectedType = m_buttons[i].buildingType;
            return true;
        }
    }
    return false;
}

void BuildMenu::selectByKey(BuildingType type) {
    for (int i = 0; i < static_cast<int>(m_buttons.size()); ++i) {
        if (m_buttons[i].buildingType == type) {
            m_selectedIdx = i;
            m_placementMode = true;
            m_selectedType = type;
            return;
        }
    }
}

void BuildMenu::cancelPlacement() {
    m_placementMode = false;
    m_selectedIdx = -1;
}

void BuildMenu::render(sf::RenderWindow& window) {
    // Panel background
    sf::RectangleShape panel({PANEL_W, PANEL_Y + BTN_START_Y + 4 * (BTN_H + BTN_GAP) + 8.0f});
    panel.setPosition({PANEL_X, PANEL_Y});
    panel.setFillColor(fromHex(0xC0000000));
    panel.setOutlineColor(fromHex(0x60444444));
    panel.setOutlineThickness(1.0f);
    window.draw(panel);

    // Title
    sf::Text titleText(m_font, "BUILD", 14);
    titleText.setFillColor(sf::Color::White);
    titleText.setPosition({PANEL_X + 8.0f, PANEL_Y + 4.0f});
    window.draw(titleText);

    // Buttons
    for (int i = 0; i < static_cast<int>(m_buttons.size()); ++i) {
        const auto& btn = m_buttons[i];
        bool isSelected = (m_selectedIdx == i && m_placementMode);

        // Button background
        sf::RectangleShape btnRect({btn.bounds.size.x, btn.bounds.size.y});
        btnRect.setPosition({btn.bounds.position.x, btn.bounds.position.y});

        if (isSelected) {
            btnRect.setFillColor(fromHex(0x603366CC));
            btnRect.setOutlineColor(sf::Color(100, 160, 255));
            btnRect.setOutlineThickness(2.0f);
        } else if (m_hoveredIdx == i && btn.enabled) {
            btnRect.setFillColor(fromHex(0x60333333));
            btnRect.setOutlineColor(fromHex(0x40555555));
            btnRect.setOutlineThickness(1.0f);
        } else {
            btnRect.setFillColor(fromHex(0x40222222));
            btnRect.setOutlineColor(fromHex(0x40333333));
            btnRect.setOutlineThickness(1.0f);
        }
        window.draw(btnRect);

        // Button label
        sf::Color labelColor = btn.enabled ? sf::Color::White : sf::Color(100, 100, 100);
        sf::Text labelText(m_font, btn.label, 13);
        labelText.setFillColor(labelColor);
        labelText.setPosition({btn.bounds.position.x + 6.0f, btn.bounds.position.y + 4.0f});
        window.draw(labelText);

        // Cost text
        sf::Color costColor;
        if (!btn.enabled) {
            costColor = sf::Color(255, 80, 80);
        } else if (btn.costText == "Free") {
            costColor = sf::Color(100, 220, 100);
        } else {
            costColor = sf::Color(200, 200, 200);
        }
        sf::Text costText(m_font, btn.costText, 12);
        costText.setFillColor(costColor);
        costText.setPosition({btn.bounds.position.x + 6.0f, btn.bounds.position.y + 26.0f});
        window.draw(costText);
    }

    // Cancel hint
    sf::Text cancelText(m_font, "ESC / Right-click: Cancel", 11);
    cancelText.setFillColor(sf::Color(140, 140, 140));
    float cancelY = BTN_START_Y + 4 * (BTN_H + BTN_GAP) + 8.0f;
    cancelText.setPosition({PANEL_X + 8.0f, cancelY});
    window.draw(cancelText);
}
