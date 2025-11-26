# include <Siv3D.hpp>

// types of fruits
static constexpr int FruitCount = 10;

// for texture and radius of fruit
struct FruitDef {
    Texture texture;
    double radius;
};

struct Fruit {
    int type;
    P2Body body;
};

// release all fruits when game over
static void ReleaseFruits(Array<Fruit>& fruits)
{
    for (auto& f : fruits)
    {
        if (f.body)
        {
            f.body.release();
        }
    }
    fruits.clear();
}

void Main()
{
    Window::Resize(800, 1000);
    Scene::SetBackground(ColorF(0.95));

    // Fruit definitions
    const Array<FruitDef> fruitDefs = {
        { Texture{U"🍒"_emoji}, 18 }, { Texture{U"🍇"_emoji}, 22 }, { Texture{U"🍓"_emoji}, 26 },
        { Texture{U"🍊"_emoji}, 32 }, { Texture{U"🍎"_emoji}, 38 }, { Texture{U"🍐"_emoji}, 46 },
        { Texture{U"🥝"_emoji}, 56 }, { Texture{U"🍍"_emoji}, 70 }, { Texture{U"🍈"_emoji}, 90 },
        { Texture{U"🍉"_emoji}, 110 }
    };

    constexpr double StepTime = (1.0 / 60.0);
    double accumulatedTime = 0.0;

    P2World world;

    // Build static cup
    const double cupW = 450;
    const double cupH = 700;

	Array<P2Body> grounds;
    grounds << world.createRect(P2Static, Vec2{ 400, 910 }, Vec2{ cupW, 20 }); // floor
    grounds << world.createRect(P2Static, Vec2{ 400 - cupW / 2 - 10, 900 - cupH / 2 }, Vec2{ 20, cupH }); // left wall
    grounds << world.createRect(P2Static, Vec2{ 400 + cupW / 2 + 10, 900 - cupH / 2 }, Vec2{ 20, cupH }); // right wall

    Array<Fruit> fruits;
    bool gameOver = false;

    // Current preview fruit type
    int previewType = Random(FruitCount - 1);

    while (System::Update())
    {
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
            P2Body b = world.createCircle(P2Dynamic, Vec2{ Cursor::Pos().x, 80 }, r);
            fruits.push_back(Fruit{ previewType, b });
            previewType = Random(FruitCount - 1); // new preview fruit
        }

        // Physics update
        for (accumulatedTime += Scene::DeltaTime(); StepTime <= accumulatedTime; accumulatedTime -= StepTime) {
            world.update(StepTime);
        }

		// Remove fruits that went out-of-bounds (defeat condition)
		for (auto& f : fruits) {
			if (!f.body || f.body.getPos().y > 1100)
			{
				gameOver = true;
			}
		}

        // Merging logic for fruits
        bool didMerge = false;
        for (size_t i = 0; i < fruits.size() && !didMerge; ++i) {
            for (size_t j = i + 1; j < fruits.size() && !didMerge; ++j) {
                auto& A = fruits[i];
                auto& B = fruits[j];
                if (A.type == B.type) {
                    double dist = A.body.getPos().distanceFrom(B.body.getPos());
                    double minDist = fruitDefs[A.type].radius + fruitDefs[B.type].radius + 100;
                    if (dist < minDist * 0.8) {
                        int newType = A.type + 1;
                        Vec2 mergePos = (A.body.getPos() + B.body.getPos()) / 2;

						// release current bodies
                        A.body.release();
                        B.body.release();
                        fruits.erase(fruits.begin() + j);
                        fruits.erase(fruits.begin() + i);

						// become next fruit
                        if (newType < FruitCount) {
                            double r = fruitDefs[newType].radius;
                            P2Body nb = world.createCircle(P2Dynamic, mergePos, r);
                            fruits.push_back(Fruit{ newType, nb });
                        }
                        didMerge = true;
                    }
                }
            }
        }

        // Draw all ground
		for (const auto& ground : grounds)
		{
			ground.draw(Palette::Gray);
		}

        // Draw fruits
        for (auto& f : fruits) {
            if (!f.body) continue;
			//Circle(f.body.getPos(), fruitDefs[f.type].radius).drawFrame(2, ColorF(1, 0, 0));
            Vec2 pos = f.body.getPos();
            const auto& def = fruitDefs[f.type];
            def.texture.resized(def.radius * 2).drawAt(pos);
        }

        // Draw preview fruit following cursor (not physics body)
        const auto& def = fruitDefs[previewType];
        def.texture.resized(def.radius * 2).drawAt(Vec2{ Cursor::Pos().x, 80 });
    }
}
