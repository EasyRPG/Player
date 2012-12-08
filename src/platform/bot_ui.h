#ifndef _BOT_UI_H_
#define _BOT_UI_H_

#include "system.h"
#include "baseui.h"
#include "audio.h"

struct BotInterface;

struct BotUi : public BaseUi {
	BotUi(EASYRPG_SHARED_PTR<BotInterface> const& inf);

	void BeginDisplayModeChange() {}
	void EndDisplayModeChange() {}
	void Resize(long, long) {}
	void ToggleFullscreen() {}
	void ToggleZoom() {}
	void UpdateDisplay() {}
	void BeginScreenCapture() {}
	void SetTitle(const std::string&) {}
	void DrawScreenText(const std::string&) {}
	void DrawScreenText(const std::string&, int, int, Color const&) {}
	void DrawScreenText(const std::string&, Rect const&, Color const&) {}
	bool ShowCursor(bool) { return false; }
	bool IsFullscreen() { return false; }

	BitmapRef EndScreenCapture();
	void ProcessEvents();

	uint32_t GetTicks() const;
	void Sleep(uint32_t time_milli);

	AudioInterface& GetAudio();

 private:
	EASYRPG_SHARED_PTR<BotInterface> bot_;
	size_t counter_;
	EmptyAudio audio_;
}; // struct BotUi

#endif // _BOT_UI_H_
