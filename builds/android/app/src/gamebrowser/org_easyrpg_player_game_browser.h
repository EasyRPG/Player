/*
 * This file is part of EasyRPG Player
 *
 * Copyright (c) EasyRPG Project. All rights reserved.
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

#include <jni.h>

#ifndef _Included_org_easyrpg_player_game_browser_GameScanner
#define _Included_org_easyrpg_player_game_browser_GameScanner
#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jobject JNICALL
Java_org_easyrpg_player_game_1browser_GameScanner_findGames(JNIEnv *env, jclass clazz, jstring path, jstring jmain_dir_name);

JNIEXPORT void JNICALL
Java_org_easyrpg_player_game_1browser_Game_reencodeTitle(JNIEnv *env, jobject thiz);

JNIEXPORT jbyteArray JNICALL
Java_org_easyrpg_player_settings_SettingsFontActivity_DrawText(JNIEnv *env, jclass clazz, jstring font, jint size, jboolean first_font);

JNIEXPORT void JNICALL
Java_org_easyrpg_player_settings_SettingsGamesFolderActivity_DetectRtp(JNIEnv *env, jobject thiz, jstring path, jobject hit_info, int version);

#ifdef __cplusplus
}
#endif
#endif
