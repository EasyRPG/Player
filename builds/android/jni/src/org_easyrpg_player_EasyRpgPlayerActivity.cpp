#include "org_easyrpg_player_EasyRpgPlayerActivity.h"
#include "graphics.h"
#include "player.h"

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL Java_org_easyrpg_player_EasyRpgPlayerActivity_toggleFps
  (JNIEnv *, jclass)
{
	Graphics::fps_on_screen = !Graphics::fps_on_screen;
}

JNIEXPORT void JNICALL Java_org_easyrpg_player_EasyRpgPlayerActivity_endGame
  (JNIEnv *, jclass)
{
	Player::exit_flag = true;
}

#ifdef __cplusplus
}
#endif
