#include "android_ui.h"
#include "player.h"
#include "input.h"
#include "graphics.h"
#include "al_audio.h"

struct android_app;

extern "C" void android_main(android_app* app) {
	DisplayUi = EASYRPG_MAKE_SHARED<AndroidUi>(app);

	Player::Init(0, NULL);
	Graphics::Init();
	Input::Init();

	Player::Run();
}
