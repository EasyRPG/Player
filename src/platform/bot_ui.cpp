#include "bitmap.h"
#include "bot_ui.h"
#include "lua_bot.h"
#include "player.h"
#include "output.h"
#include "input.h"

#include <boost/chrono.hpp>
#include <boost/thread.hpp>


BotUi::BotUi(EASYRPG_SHARED_PTR<BotInterface> const& inf)
	: bot_(inf), counter_(0)
{
	current_display_mode.width = SCREEN_TARGET_WIDTH;
	current_display_mode.height = SCREEN_TARGET_HEIGHT;

	DynamicFormat const format
		(8*4,
#ifdef READER_BIG_ENDIAN
		 0xff000000,
		 0x00ff0000,
		 0x0000ff00,
		 0x000000ff,
#else
		 0x000000ff,
		 0x0000ff00,
		 0x00ff0000,
		 0xff000000,
#endif
		 PF::NoAlpha);
	Bitmap::SetFormat(Bitmap::ChooseFormat(format));

	main_surface = Bitmap::Create(SCREEN_TARGET_WIDTH, SCREEN_TARGET_HEIGHT, false, 4);
}

void BotUi::ProcessEvents() {
	if(bot_->is_finished()) {
		Player::exit_flag = true;
		return;
	}

	// interval: 0.1ms
	static const int INTERVAL = 6;

	if(counter_++ > INTERVAL) {
		keys.reset();
		counter_ = 0;

		if(Input::IsWaitingInput()) {
			assert(!bot_->is_finished());
			bot_->resume();
		}
	}
}

BitmapRef BotUi::EndScreenCapture() {
	return Bitmap::Create(*main_surface, main_surface->GetRect());
}

uint32_t BotUi::GetTicks() const {
	using namespace boost::chrono;
	return duration_cast<milliseconds>
		(system_clock::now().time_since_epoch()).count();
}

void BotUi::Sleep(uint32_t time) {
	boost::this_thread::sleep_for(boost::chrono::milliseconds(time));
}
