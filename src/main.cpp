#include <algorithm>
#include <cstdint>
#include <iostream>
#include <memory>
#include <random>
#include <vector>

#include <numbers>
#include <cmath>
#include <optional>
#include <functional>

#include <SFML/Graphics.hpp>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;
const int FPS_LIMIT = 30;

const int ANIMATION_FRAMES = 180;

const float CIRCLE_RADIUS = 25.f;
const float LEFT_X = 75.f;
const float RIGHT_X = WINDOW_WIDTH - 75.f;
const float CIRCLE_Y = WINDOW_HEIGHT / 3.f;

const float GRAPH_LEFT = 80.f;
const float GRAPH_RIGHT = WINDOW_WIDTH - 80.f;
const float GRAPH_TOP = 500.f;
const float GRAPH_BOTTOM = 740.f;

// global tween function
std::function<float(float, float, float)> tween = [](float a, float b, float t) {
    return (1 - t) * a + t * b;
};

void handleInput(sf::Window& window, bool& shouldQuit) {
    while (const std::optional<sf::Event> event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
            shouldQuit = true;
        }

        if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            switch (key->code) {
            case sf::Keyboard::Key::Num1: // linear
                tween = [](float a, float b, float t) { return a + (b - a) * t; };
                break;
            case sf::Keyboard::Key::Num2: // ease-in quadratic
                tween = [](float a, float b, float t) { return a + (b - a) * t * t; };
                break;
            case sf::Keyboard::Key::Num3: // ease-out quadratic
                tween = [](float a, float b, float t) {
                    return a + (b - a) * (1.f - (1.f - t) * (1.f - t));
                };
                break;
            case sf::Keyboard::Key::Num4: // ease-in-out quadratic
                tween = [](float a, float b, float t) {
                    const float eased = t < .5f ? 2.f * t * t : 1.f - std::pow(-2.f * t + 2.f, 2.f) / 2.f;
                    return a + (b - a) * eased;
                };
                break;
            case sf::Keyboard::Key::Num5: // ease-in cubic
                tween = [](float a, float b, float t) { return a + (b - a) * t * t * t; };
                break;
            case sf::Keyboard::Key::Num6: // ease-out cubic
                tween = [](float a, float b, float t) {
                    return a + (b - a) * (1.f - std::pow(1.f - t, 3.f));
                };
                break;
            case sf::Keyboard::Key::Num7: // sine ease-in-out
                tween = [](float a, float b, float t) {
                    return a + (b - a) * (-(std::cos(std::numbers::pi_v<float> * t) - 1.f) / 2.f);
                };
                break;
            case sf::Keyboard::Key::Num8: // ease-out bounce
                tween = [](float a, float b, float t) {
                    constexpr float n = 7.5625f;
                    constexpr float d = 2.75f;
                    float eased;
                    if (t < 1.f / d) eased = n * t * t;
                    else if (t < 2.f / d) { t -= 1.5f / d; eased = n * t * t + .75f; }
                    else if (t < 2.5f / d) { t -= 2.25f / d; eased = n * t * t + .9375f; }
                    else { t -= 2.625f / d; eased = n * t * t + .984375f; }
                    return a + (b - a) * eased;
                };
                break;
            case sf::Keyboard::Key::Num9: // smoothstep
                tween = [](float a, float b, float t) {
                    const float eased = t * t * (3.f - 2.f * t);
                    return a + (b - a) * eased;
                };
                break;
            default:
                break;
            }
        }
    }
}

void render(sf::RenderWindow& window) {
    window.clear(sf::Color::Black);

    static int frame = 0;
    const float t = static_cast<float>(frame) / static_cast<float>(ANIMATION_FRAMES - 1);
    const float x = tween(LEFT_X, RIGHT_X, t);

    sf::CircleShape movingCircle(CIRCLE_RADIUS);
    movingCircle.setFillColor(sf::Color(80, 190, 255));
    movingCircle.setOrigin({CIRCLE_RADIUS, CIRCLE_RADIUS});
    movingCircle.setPosition({x, CIRCLE_Y});
    window.draw(movingCircle);

    sf::VertexArray axes(sf::PrimitiveType::Lines, 4);
    axes[0] = {{GRAPH_LEFT, GRAPH_BOTTOM}, sf::Color::White};
    axes[1] = {{GRAPH_RIGHT, GRAPH_BOTTOM}, sf::Color::White};
    axes[2] = {{GRAPH_LEFT, GRAPH_BOTTOM}, sf::Color::White};
    axes[3] = {{GRAPH_LEFT, GRAPH_TOP}, sf::Color::White};
    window.draw(axes);

    constexpr int SAMPLES = 200;
    sf::VertexArray curve(sf::PrimitiveType::LineStrip, SAMPLES + 1);
    for (int i = 0; i <= SAMPLES; ++i) {
        const float sampleT = static_cast<float>(i) / SAMPLES;
        const float value = std::clamp(tween(0.f, 1.f, sampleT), 0.f, 1.f);
        const float graphX = GRAPH_LEFT + sampleT * (GRAPH_RIGHT - GRAPH_LEFT);
        const float graphY = GRAPH_BOTTOM - value * (GRAPH_BOTTOM - GRAPH_TOP);
        curve[i] = {{graphX, graphY}, sf::Color(255, 210, 70)};
    }
    window.draw(curve);

    const float currentValue = std::clamp(tween(0.f, 1.f, t), 0.f, 1.f);
    const float markerX = GRAPH_LEFT + t * (GRAPH_RIGHT - GRAPH_LEFT);
    const float markerY = GRAPH_BOTTOM - currentValue * (GRAPH_BOTTOM - GRAPH_TOP);
    sf::CircleShape marker(7.f);
    marker.setFillColor(sf::Color::Red);
    marker.setOrigin({7.f, 7.f});
    marker.setPosition({markerX, markerY});
    window.draw(marker);

    window.display();
    frame = (frame + 1) % ANIMATION_FRAMES;
}

int main() {
    sf::RenderWindow window;

    try {
        // Initialize window
        window.create(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "Tween");
        window.setFramerateLimit(FPS_LIMIT);
        // Prevent key repeats.
        window.setKeyRepeatEnabled(false);

        bool shouldQuit = false;
        // Main game loop
        while (window.isOpen()) {
            handleInput(window, shouldQuit);
            if (shouldQuit) {
                break;
            }
            render(window);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}
