#include <Siv3D.hpp>
#include <vector>

// Types of fruits
static constexpr int FruitCount = 10;

// For texture and radius of fruit
struct FruitDef {
    Texture texture;
    double radius;
};

struct Fruit {
    int type;
    Vec2 position;
    Vec2 velocity;
    double radius;
};

// Basic physics constants
constexpr double Gravity = 500;
constexpr double Friction = 0.999;
constexpr double StepTime = 1.0 / 60.0;


// Release all fruits when game over
static void ReleaseFruits(std::vector<Fruit>& fruits) {
    fruits.clear();
}


void Main() {
    Window::Resize(800, 1000);
    Scene::SetBackground(ColorF(0.95));

    // Fruit definitions
    const std::vector<FruitDef> fruitDefs = {
        { Texture{ U"🍒"_emoji }, 18 }, { Texture{ U"🍇"_emoji }, 22 }, { Texture{ U"🍓"_emoji }, 26 },
        { Texture{ U"🍊"_emoji }, 32 }, { Texture{ U"🍎"_emoji }, 38 }, { Texture{ U"🍐"_emoji }, 46 },
        { Texture{ U"🥝"_emoji }, 56 }, { Texture{ U"🍍"_emoji }, 70 }, { Texture{ U"🍈"_emoji }, 90 },
        { Texture{ U"🍉"_emoji }, 110 }
    };

    constexpr double cupW = 450;
    constexpr double cupH = 700;

    // Static ground positions
    const Vec2 floorPos = Vec2{ 400, 910 };
    const Vec2 leftWallPos = Vec2{ 400 - cupW / 2 - 10, 200};
    const Vec2 rightWallPos = Vec2{ 400 + cupW / 2 + 10, 200};

    std::vector<Fruit> fruits;
    bool gameOver = false;

    int previewType = Random(FruitCount - 1);

    while (System::Update()) {
        if (gameOver) {
            if (SimpleGUI::Button(U"Restart", Vec2{ 350, 450 })) {
                ReleaseFruits(fruits);
                gameOver = false;
                previewType = Random(FruitCount - 1);
            }
            continue;
        }

        // Drop fruit on click
        if (MouseL.down()) {
            double r = fruitDefs[previewType].radius;
            fruits.push_back({ previewType, Vec2{ Cursor::Pos().x, 80 }, Vec2{ 0, 0 }, r });
            previewType = Random(FruitCount - 1);
        }

        // Physics update
        for (auto& f : fruits) {

            // Gravity
            f.velocity.y += Gravity * StepTime;

            // Apply velocity
            f.position += f.velocity * StepTime;

            // Air friction
            f.velocity *= Friction;

            // Floor collision
            if (f.position.y + f.radius > floorPos.y) {
                f.position.y = floorPos.y - f.radius;
                f.velocity.y *= -0.6;
            }

            // Walls collision
            if (f.position.x - f.radius < leftWallPos.x) {
                f.position.x = leftWallPos.x + f.radius;
                f.velocity.x *= -0.6;
            }
            if (f.position.x + f.radius > rightWallPos.x) {
                f.position.x = rightWallPos.x - f.radius;
                f.velocity.x *= -0.6;
            }
        }

        // Fruit–fruit collision
        for (size_t i = 0; i < fruits.size(); ++i) {
            for (size_t j = i + 1; j < fruits.size(); ++j) {

                Fruit& A = fruits[i];
                Fruit& B = fruits[j];

                Vec2 diff = B.position - A.position;
                double dist = diff.length();
                double minDist = A.radius + B.radius;

                if (dist < minDist && dist > 0.0001) {

                    Vec2 normal = diff / dist;
                    double penetration = minDist - dist;

                    // Push fruits apart
                    A.position -= normal * (penetration * 0.5);
                    B.position += normal * (penetration * 0.5);

                    // Simple bounce response
                    double va = Dot(A.velocity, normal);
                    double vb = Dot(B.velocity, normal);
                    double bounce = 0.8;

                    double newVA = vb * bounce;
                    double newVB = va * bounce;

                    A.velocity += normal * (newVA - va);
                    B.velocity += normal * (newVB - vb);
                }
            }
        }


        // Merging logic
        bool didMerge = false;

        for (size_t i = 0; i < fruits.size() && !didMerge; ++i) {
            for (size_t j = i + 1; j < fruits.size() && !didMerge; ++j) {

                Fruit& A = fruits[i];
                Fruit& B = fruits[j];

                if (A.type != B.type) continue;

                double dist = A.position.distanceFrom(B.position);
                double minDist = A.radius + B.radius + 100;

                // Merge threshold (overlap more than 40%)
                if (dist < minDist) {
                    int newType = A.type + 1;
                    Vec2 mergePos = (A.position + B.position) * 0.5;

                    fruits.erase(fruits.begin() + j);
                    fruits.erase(fruits.begin() + i);

                    if (newType < FruitCount) {
                        double r = fruitDefs[newType].radius;
                        fruits.push_back({ newType, mergePos, Vec2{ 0, 0 }, r });
                    }

                    didMerge = true;
                }
            }
        }


        // Game over if a fruit falls outside
        for (auto& f : fruits) {
            if (f.position.y > 1100) {
                gameOver = true;
            }
        }

        // Draw fruits
        for (auto& f : fruits) {
            const auto& def = fruitDefs[f.type];
            def.texture.resized(f.radius * 2).drawAt(f.position);
        }

        // Preview fruit
        {
            const auto& def = fruitDefs[previewType];
            def.texture.resized(def.radius * 2).drawAt(Vec2{ Cursor::Pos().x, 80 });
        }

        // Draw cup
        Rect(floorPos.x - cupW / 2, floorPos.y - 10, cupW, 20).draw(Palette::Gray);
        Rect(leftWallPos.x - 10, leftWallPos.y, 20, cupH).draw(Palette::Gray);
        Rect(rightWallPos.x - 10, rightWallPos.y, 20, cupH).draw(Palette::Gray);
    }
}
