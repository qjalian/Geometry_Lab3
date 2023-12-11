#include <SFML/Graphics.hpp>
#include <functional>
#include <iostream>
#include <vector>
#include "imgui-SFML.h"
#include "imgui.h"

float RAnd(float w1, float w2) { return w1 + w2 + std::sqrt((w1 * w1 + w2 * w2) - 2 * w1 * w2); }

float ROr(float w1, float w2) { return w1 + w2 - std::sqrt((w1 * w1 + w2 * w2) - 2 * w1 * w2); }

sf::Color interpolateColors(const sf::Color &colorFirst, const sf::Color &colorSec, float k)
{
	sf::Color val;
	val.r = static_cast<sf::Uint8>(colorFirst.r + (colorSec.r - colorFirst.r) * k);
	val.g = static_cast<sf::Uint8>(colorFirst.g + (colorSec.g - colorFirst.g) * k);
	val.b = static_cast<sf::Uint8>(colorFirst.b + (colorSec.b - colorFirst.b) * k);
	val.a = static_cast<sf::Uint8>(colorFirst.a + (colorSec.a - colorFirst.a) * k);

	return val;
}

class RFuncSprites : public sf::Sprite
{
public:
	void create(const sf::Vector2u &size, const int getElem)
	{
		_image.create(size.x, size.y, sf::Color::Cyan);
		_texture.loadFromImage(_image);
		setTexture(_texture);

		_firstColor = sf::Color::Black;
		_secondColor = sf::Color::White;

		_getNorm = getElem;
	}

	void DrawRFunc(const std::function<float(const sf::Vector2f &)> &rfunc, const sf::FloatRect &subSpace)
	{
		sf::Vector2f spaceStep = {subSpace.width / static_cast<float>(_image.getSize().x),
								  subSpace.height / static_cast<float>(_image.getSize().y)};

		for (int x = 0; x < _image.getSize().x - 1; ++x)
		{
			for (int y = 0; y < _image.getSize().y - 1; ++y)
			{
				sf::Vector2f spacePoint1 = {subSpace.left + static_cast<float>(x) * spaceStep.x,
											subSpace.top + static_cast<float>(y) * spaceStep.y};

				const float z1 = rfunc(spacePoint1);

				sf::Vector2f spacePoint2 = {subSpace.left + static_cast<float>(x + 1) * spaceStep.x,
											subSpace.top + static_cast<float>(y) * spaceStep.y};

				const float z2 = rfunc(spacePoint2);

				sf::Vector2f spacePoint3 = {subSpace.left + static_cast<float>(x) * spaceStep.x,
											subSpace.top + static_cast<float>(y + 1) * spaceStep.y};

				const float z3 = rfunc(spacePoint3);

				const float A = calculateDeterminant3x3({
					{spacePoint1.y, z1, 1},
					{spacePoint2.y, z2, 1},
					{spacePoint3.y, z3, 1},
				});

				const float B = calculateDeterminant3x3({
					{spacePoint1.x, z1, 1},
					{spacePoint2.x, z2, 1},
					{spacePoint3.x, z3, 1},
				});

				const float C = calculateDeterminant3x3({
					{spacePoint1.x, spacePoint1.y, 1},
					{spacePoint2.x, spacePoint2.y, 1},
					{spacePoint3.x, spacePoint3.y, 1},
				});

				const float D = calculateDeterminant3x3({
					{spacePoint1.x, spacePoint1.y, z1},
					{spacePoint2.x, spacePoint2.y, z2},
					{spacePoint3.x, spacePoint3.y, z3},
				});

				const float det = std::sqrt(A * A + B * B + C * C + D * D);

				float nx = A / det;
				float ny = B / det;
				float nz = C / det;
				float nw = D / det;

				float getNorm = nx;

				switch (_getNorm)
				{
				case 0:
					break;
				case 1:
					getNorm = ny;
					break;
				case 2:
					getNorm = nz;
					break;
				case 3:
					getNorm = nw;
					break;
				}

				auto pixelColor = interpolateColors(_firstColor, _secondColor, (1.f + getNorm) / 2);
				_image.setPixel(x, y, pixelColor);
			}
		}

		_texture.loadFromImage(_image);
	}

	float calculateDeterminant3x3(const std::vector<std::vector<float>> &matrix)
	{
		if (matrix.size() != 3 || matrix[0].size() != 3 || matrix[1].size() != 3 || matrix[2].size() != 3)
		{
			throw std::runtime_error("Wrong");
		}

		return matrix[0][0] * (matrix[1][1] * matrix[2][2] - matrix[1][2] * matrix[2][1]) -
			matrix[0][1] * (matrix[1][0] * matrix[2][2] - matrix[1][2] * matrix[2][0]) +
			matrix[0][2] * (matrix[1][0] * matrix[2][1] - matrix[1][1] * matrix[2][0]);
	}

	void upd(const sf::Color &firstColor, const sf::Color &secondColor)
	{
		for (int x = 0; x < _image.getSize().x - 1; ++x)
		{
			for (int y = 0; y < _image.getSize().y - 1; ++y)
			{
				float t =
					(static_cast<float>(_image.getPixel(x, y).r) - _firstColor.r) / (_secondColor.r - _firstColor.r);
				auto pixelColor = interpolateColors(firstColor, secondColor, t);
				_image.setPixel(x, y, pixelColor);
			}
		}

		_firstColor = firstColor;
		_secondColor = secondColor;
		_texture.loadFromImage(_image);
	}

	void saveImg(const std::string &filename) { _image.saveToFile(filename); }
	void clearPath()
	{
		_image.create(_image.getSize().x, _image.getSize().y, sf::Color::Cyan);
		_texture.loadFromImage(_image);
	}

private:
	sf::Color _firstColor;
	sf::Color _secondColor;
	sf::Texture _texture;
	sf::Image _image;
	int _getNorm;
};

void HandleUserInput(sf::RenderWindow &window, const sf::Event &event, RFuncSprites &currentSprite);

int main()
{
	sf::RenderWindow window(sf::VideoMode(400, 400), "Lab3");
	window.setFramerateLimit(60);
	if (!ImGui::SFML::Init(window))
	{
		std::cout << "ImGui initialization failed\n";
		return -1;
	}

	auto spriteSize = sf::Vector2u{window.getSize().x, window.getSize().y};

	RFuncSprites RFuncSpritesNx;
	RFuncSpritesNx.create(spriteSize, 0);

	RFuncSprites RFuncSpritesNy;
	RFuncSpritesNy.create(spriteSize, 1);
	RFuncSpritesNy.setPosition(spriteSize.x, 0);

	RFuncSprites RFuncSpritesNz;
	RFuncSpritesNz.create(spriteSize, 2);
	RFuncSpritesNz.setPosition(0, spriteSize.y);

	RFuncSprites RFuncSpritesNw;
	RFuncSpritesNw.create(spriteSize, 3);
	RFuncSpritesNw.setPosition(spriteSize.x, spriteSize.y);

	std::function<float(const sf::Vector2f &)> rFunctions[6];

	rFunctions[0] = [](const sf::Vector2f &point) -> float { return std::sin(point.x) - std::cos(point.y); };
	rFunctions[1] = [](const sf::Vector2f &point) -> float { return point.x * point.y - 10; };
	rFunctions[2] = [](const sf::Vector2f &point) -> float { return std::sin(point.x) * std::cos(point.y); };
	rFunctions[3] = [](const sf::Vector2f &point) -> float { return std::cos(point.x + point.y); };
	rFunctions[4] = [](const sf::Vector2f &point) -> float { return point.x * point.x + point.y * point.y - 500; };
	rFunctions[5] = [](const sf::Vector2f &point) -> float { return std::sin(point.x) / std::cos(point.y) - 40; };

	std::function<float(const sf::Vector2f &)> complexFunction = [&rFunctions](const sf::Vector2f &point) -> float
	{
		return RAnd(RAnd(RAnd(ROr(RAnd(rFunctions[0](point), rFunctions[1](point)), rFunctions[2](point)),
							  rFunctions[3](point)),
						 rFunctions[4](point)),
					rFunctions[5](point));
	};

	sf::FloatRect subSpace(-10.f, -10.f, 20.f, 20.f);
	sf::RenderTexture pathTexture;
	pathTexture.create(window.getSize().x, window.getSize().y);
	pathTexture.clear(sf::Color::Transparent);

	int selectedMObrazIndex = 0;

	sf::Clock deltaClock;

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(event);

			HandleUserInput(window, event, RFuncSpritesNx);
			HandleUserInput(window, event, RFuncSpritesNy);
			HandleUserInput(window, event, RFuncSpritesNz);
			HandleUserInput(window, event, RFuncSpritesNw);

			if (event.type == sf::Event::Closed)
			{
				window.close();
			}
		}

		ImGui::SFML::Update(window, deltaClock.restart());

		ImGui::Begin("Menu");
		const char *mobrazNames[] = {"M-Obraz Nx", "M-Obraz Ny", "M-Obraz Nz", "M-Obraz Nw"};
		ImGui::Combo("Select M-Obraz", &selectedMObrazIndex, mobrazNames, IM_ARRAYSIZE(mobrazNames));

		ImVec4 firstColor = ImVec4(0, 0, 0, 1);
		ImVec4 secondColor = ImVec4(1, 1, 1, 1);

		auto sfFirstColor =
			sf::Color(static_cast<sf::Uint8>(firstColor.x * 255), static_cast<sf::Uint8>(firstColor.y * 255),
					  static_cast<sf::Uint8>(firstColor.z * 255), static_cast<sf::Uint8>(firstColor.w * 255));

		auto sfSecondColor =
            sf::Color(static_cast<sf::Uint8>(secondColor.x * 255), static_cast<sf::Uint8>(secondColor.y * 255),
					  static_cast<sf::Uint8>(secondColor.z * 255), static_cast<sf::Uint8>(secondColor.w * 255));

		switch (selectedMObrazIndex)
		{
		case 0:
			RFuncSpritesNx.DrawRFunc(complexFunction, subSpace);
			RFuncSpritesNx.upd(sfFirstColor, sfSecondColor);
			break;
		case 1:
			RFuncSpritesNy.DrawRFunc(complexFunction, subSpace);
			RFuncSpritesNy.upd(sfFirstColor, sfSecondColor);
			break;
		case 2:
			RFuncSpritesNz.DrawRFunc(complexFunction, subSpace);
			RFuncSpritesNz.upd(sfFirstColor, sfSecondColor);
			break;
		case 3:
			RFuncSpritesNw.DrawRFunc(complexFunction, subSpace);
			RFuncSpritesNw.upd(sfFirstColor, sfSecondColor);
			break;
		default:
			// Handle error or default case
			break;
		}

		if (ImGui::Button("Save Image"))
		{
			switch (selectedMObrazIndex)
			{
			case 0:
				RFuncSpritesNx.saveImg("nx.png");
				break;
			case 1:
				RFuncSpritesNy.saveImg("ny.png");
				break;
			case 2:
				RFuncSpritesNz.saveImg("nz.png");
				break;
			case 3:
				RFuncSpritesNw.saveImg("nw.png");
				break;
			default:
				// Handle error or default case
				break;
			}
		}
		if (ImGui::Button("Clear Paths"))
		{
			pathTexture.clear(sf::Color::Transparent);
			RFuncSpritesNx.clearPath();
			RFuncSpritesNy.clearPath();
			RFuncSpritesNz.clearPath();
			RFuncSpritesNw.clearPath();
			// Clear paths for other sprites if needed
		}
		ImGui::End();

		complexFunction = rFunctions[selectedMObrazIndex];

		// Draw the path on the render texture
		pathTexture.draw(RFuncSpritesNx);

		// Clear the window
		window.clear();

		// Draw the M-Obraz sprites
		RFuncSpritesNx.DrawRFunc(complexFunction, subSpace);
		RFuncSpritesNx.upd(sfFirstColor, sfSecondColor);

		// Draw the path render texture on top
		sf::Sprite pathSprite(pathTexture.getTexture());
		window.draw(pathSprite);

		ImGui::SFML::Render(window);

		window.display();
	}

	ImGui::SFML::Shutdown();

	return 0;
}

void HandleUserInput(sf::RenderWindow &window, const sf::Event &event, RFuncSprites &currentSprite)
{
	switch (event.type)
	{
	case sf::Event::Closed:
		window.close();
		break;
	case sf::Event::MouseButtonPressed:
		if (event.mouseButton.button == sf::Mouse::Left)
		{
			sf::Vector2f mousePos = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));

			sf::Color firstColor = sf::Color::White; 
			sf::Color secondColor = sf::Color::White; 

			currentSprite.upd(firstColor, secondColor); 

			currentSprite.DrawRFunc(
				[&mousePos](const sf::Vector2f &point) -> float
				{
					return std::sqrt((point.x - mousePos.x) * (point.x - mousePos.x) +
									 (point.y - mousePos.y) * (point.y - mousePos.y));
				},
				sf::FloatRect(-10.f, -10.f, 20.f, 20.f));
		}
		break;
	default:
		break;
	}
}

