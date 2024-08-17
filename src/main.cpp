#include <numbers>

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.hpp"


class WalkiesGame : public olc::PixelGameEngine {
//utils
public:
	void DrawRay(olc::vf2d origin, olc::vf2d dir, olc::Pixel colour = olc::WHITE) {
		DrawLine(origin, origin + dir, colour);
	}

	float map(float value,
		float start1, float stop1,
		float start2, float stop2)
	{
		//Processing
		return start2 + (stop2 - start2) * ((value - start1) / (stop1 - start1));
	}

	olc::vf2d RotateVector(olc::vf2d vec, float angleRad) {
		float temp = vec.x;
		vec.x = vec.x * cosf(angleRad) - vec.y * sinf(angleRad);
		vec.y = temp * sinf(angleRad) + vec.y * cosf(angleRad);
		return vec;
	}

	float VectorDist(olc::vf2d a, olc::vf2d b) {
		olc::vf2d d = a - b;
		return d.mag();
	}

	void DrawStringCentered(const std::string &text, int textScale = 4, olc::vf2d offset = {0, 0}, olc::Pixel colour = olc::WHITE) {
		olc::vf2d textSize = GetTextSize(text) * textScale;
		DrawString(GetScreenSize() / 2 - textSize / 2 - offset + olc::vf2d(textScale, textScale), text, olc::BLACK, textScale); //shadow
		DrawString(GetScreenSize() / 2 - textSize / 2 - offset, text, colour, textScale);
	}

public:
	WalkiesGame() { sAppName = "Walkies"; }

private:
	enum class GameState {
		COUNTDOWN,
		PLAYING,
		END
	};

	int countDown = 3;

	float accTime = 0.0f;
	float groundHeight = 70;
	float maxSteppyDist = 30;
	float maxFromBodyDist = maxSteppyDist * 1.1f;

	float headRadius = 10;
	float footSize = 3;
	olc::vf2d poppyOrigin = {headRadius * 2.0f + footSize, 150};
	olc::vf2d torsoOrigin = {0, headRadius};
	float torsoHeight = 40;
	float armWiggleSpeed = 5.0f;
	float armWiggleAmp = 0.02f;
	olc::vf2d armsOrigin = {0, headRadius * 2.0f};
	olc::vf2d armsDir = {-headRadius * 1.5f, 0}; //x is arm length
	olc::vf2d legsOrigin = {0, torsoHeight};

	olc::vf2d leftFoot = poppyOrigin + legsOrigin + olc::vf2d(-10, 20);
	olc::vf2d rightFoot = poppyOrigin + legsOrigin + olc::vf2d(10, 20);

	olc::vf2d leftFootClickPos = {0, 0};
	olc::vf2d rightFootClickPos = {0, 0};

	bool leftFootClicked = false;
	bool rightFootClicked = false;

	GameState gameState = GameState::COUNTDOWN;

	float scoreTime = -1;

public:
	bool OnUserCreate() override {
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override {
		accTime += fElapsedTime;
		float timeLeft = countDown - accTime;

		//background
		Clear(olc::Pixel(52, 210, 245));
		FillRect(0, ScreenHeight() - groundHeight, ScreenWidth(), groundHeight, olc::Pixel(63, 191, 46));
		
		//sun
		olc::vf2d sunOrigin = {(float)ScreenWidth(), 0};
		FillCircle(sunOrigin, 30 + sinf(accTime*0.3f)*3, olc::YELLOW);

		//sun rays
		int sides = 30;
		for (int i = 0; i < sides; i++) {
			olc::vf2d ray = olc::vf2d(-42 - sinf(accTime*0.3f)*4, 0);
			ray = RotateVector(ray, map((float)i, 0.0f, (float)sides, 0, std::numbers::pi_v<float>*2.0f) + accTime*0.1f);
			DrawRay(sunOrigin + ray, ray, olc::YELLOW);
		}


		//head
		DrawCircle(poppyOrigin, headRadius);

		//torso
		DrawLine(poppyOrigin + torsoOrigin, poppyOrigin + legsOrigin);

		//arms
		armsDir = RotateVector(armsDir, sinf(accTime * armWiggleSpeed) * armWiggleAmp);
		DrawRay(poppyOrigin + armsOrigin, armsDir);
		DrawRay(poppyOrigin + armsOrigin, -armsDir);

		//feet
		//foot: left
		{
			if (gameState == GameState::PLAYING) {
				leftFootClicked = GetMouse(olc::Mouse::LEFT).bHeld
					&& VectorDist(GetMousePos(), leftFoot) < (footSize + 1)
					&& !rightFootClicked;

				if (leftFootClicked && GetMouse(olc::Mouse::LEFT).bPressed) {
					leftFootClickPos = GetMousePos();
				}

				//show limits
				if (leftFootClickPos != olc::vf2d(0, 0)) {
					DrawCircle(leftFootClickPos, maxSteppyDist, olc::RED);
					DrawCircle(poppyOrigin + legsOrigin, maxFromBodyDist, olc::DARK_BLUE);
				}
			}

			//apply new position while taking limits into account
			if (leftFootClicked) {
				if (VectorDist(GetMousePos(), leftFootClickPos) < maxSteppyDist
					&& VectorDist(GetMousePos(), poppyOrigin + legsOrigin) < maxFromBodyDist) {
					leftFoot = GetMousePos();
				}
				FillCircle(leftFoot, footSize);
			} else {
				DrawCircle(leftFoot, footSize);
			}
		}

		//foot: right
		{
			if (gameState == GameState::PLAYING) {
				rightFootClicked = GetMouse(olc::Mouse::LEFT).bHeld 
					&& VectorDist(GetMousePos(), rightFoot) < (footSize + 1)
					&& !leftFootClicked;

				if (rightFootClicked && GetMouse(olc::Mouse::LEFT).bPressed) {
					rightFootClickPos = GetMousePos();
				}

				//show limits
				if (rightFootClickPos != olc::vf2d(0, 0)) {
					DrawCircle(rightFootClickPos, maxSteppyDist, olc::RED);
					DrawCircle(poppyOrigin + legsOrigin, maxFromBodyDist, olc::DARK_BLUE);
				}
			}

			//apply new position while taking limits into account
			if (rightFootClicked) {
				if (VectorDist(GetMousePos(), rightFootClickPos) < maxSteppyDist
					&& VectorDist(GetMousePos(), poppyOrigin + legsOrigin) < maxFromBodyDist) {
					rightFoot = GetMousePos();
				}
				FillCircle(rightFoot, footSize);
			} else {
				DrawCircle(rightFoot, footSize);
			}
		}

		//feet reset
		if (GetMouse(olc::Mouse::LEFT).bReleased) {
			leftFootClickPos = {0, 0};
			rightFootClickPos = {0, 0};
			leftFootClicked = false;
			rightFootClicked = false;
		}

		//legs
		DrawLine(poppyOrigin + legsOrigin, leftFoot);
		DrawLine(poppyOrigin + legsOrigin, rightFoot);


		//move Poppy's body
		poppyOrigin.x = (leftFoot.x + rightFoot.x) / 2.0f;


		//state management and state-dependent logic
		std::string stateString;
		switch (gameState) {
			case GameState::COUNTDOWN: {
				stateString = "Countdown";
				DrawStringCentered("Walkies", 5, {0, 70}, olc::RED);
				DrawStringCentered(std::to_string(((int) timeLeft) + 1), 5);
				DrawStringCentered("Game", 5, {0, -60}, olc::RED);
				if (timeLeft < 0) gameState = GameState::PLAYING;
				break;
			}
			case GameState::PLAYING: {
				stateString = "Playing";

				if (timeLeft > -1) {
					DrawStringCentered("RUN!", 6);
				}

				DrawStringCentered(std::to_string(accTime - countDown), 1, {0, 100});

				float leniency = footSize * 3;
				if (poppyOrigin.x > ScreenWidth() - leniency) {
					scoreTime = accTime - countDown;
					gameState = GameState::END;
					leftFootClickPos = {0, 0};
					rightFootClickPos = {0, 0};
					leftFootClicked = false;
					rightFootClicked = false;
				}
				break;
			}
			case GameState::END: {
				stateString = "End";
				DrawStringCentered("You win!", 4, {0, 40});
				DrawStringCentered("Time: " + std::to_string(scoreTime), 2);
				DrawStringCentered("Now go brag about your score", 1, {0, -25});
				DrawStringCentered("And get your friends to play it, too!", 1, {0, -35});
				break;
			}
		}

		DrawString(10, 10, stateString, olc::Pixel(255, 255, 255, 20));

		if (timeLeft > -5) {
			DrawStringCentered("Move your feet by click-dragging them", 1, {0, -110});
		}
		return true;
	}
};

int main() {
	WalkiesGame game;
	if (game.Construct(310, 240, 2, 2, false, true))
		game.Start();
	return 0;
}
