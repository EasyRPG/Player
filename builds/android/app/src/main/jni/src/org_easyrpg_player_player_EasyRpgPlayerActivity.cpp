/*
 * This file is part of EasyRPG Player
 *
 * Copyright (c) 2017 EasyRPG Project. All rights reserved.
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *  
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 */

#include "org_easyrpg_player_player_EasyRpgPlayerActivity.h"
#include "player.h"

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL Java_org_easyrpg_player_player_EasyRpgPlayerActivity_toggleFps
  (JNIEnv *, jclass)
{
	Player::fps_flag = !Player::fps_flag;
}

JNIEXPORT void JNICALL Java_org_easyrpg_player_player_EasyRpgPlayerActivity_endGame
  (JNIEnv *, jclass)
{
	Player::exit_flag = true;
}

JNIEXPORT void JNICALL Java_org_easyrpg_player_player_EasyRpgPlayerActivity_setFastForwardMultiplier
  (JNIEnv *, jclass, jint ji)
{
	Player::speed_modifier = ji;
}

#ifdef __cplusplus
}
#endif
