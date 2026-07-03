#pragma once

#include "Entity/BuildingTypes.h"

#include <SFML/Graphics/Rect.hpp>

#include <string>
#include <vector>

namespace sf {
    class Font;
    class RenderWindow;
}
class ResourceManager;

class BuildMenu {
public:
    BuildMenu(const sf::Font& font, ResourceManager* resources);

    void update();
    void handleMouseMove(const sf::Vector2i& pos);
    bool handleClick(const sf::Vector2i& pos);
    void selectByKey(BuildingType type);
    void cancelPlacement();
    void render(sf::RenderWindow& window);

    bool isPlacementMode() const { return m_placementMode; }
    BuildingType getSelectedType() const { return m_selectedType; }

private:
    struct Button {
        sf::FloatRect bounds;
        std::string label;
        std::string costText;
        bool enabled = true;
        bool hovered = false;
        BuildingType buildingType;
    };

    const sf::Font& m_font;
    ResourceManager* m_resources;
    std::vector<Button> m_buttons;
    int m_hoveredIdx = -1;
    int m_selectedIdx = -1;
    bool m_placementMode = false;
    BuildingType m_selectedType = BuildingType::Headquarters;

    void buildLayout();
    void refreshAffordability();
};
