#include "android_ui.h"
#include "al_audio.h"
#include "player.h"

#include <android_native_app_glue.h>
#include <android/sensor.h>
#include <android/log.h>
#include <android/keycodes.h>

#include <boost/chrono.hpp>
#include <boost/thread.hpp>

#include <cassert>


namespace {

	typedef std::map<int, std::pair<float, float> > motion_states_type;
	motion_states_type current_motion_states_;
	BaseUi::KeyStatus current_key_status_;

	void onAppCmd(android_app* const app, int32_t cmd) {
	}

	Input::Keys::InputKey to_easyrpg_keycode(int const android_key) {
#define c(android, easyrpg) \
		(android_key == (AKEYCODE_ ## android))? Input::Keys::easyrpg:

		return
			c(SOFT_LEFT, LEFT)
			c(SOFT_RIGHT, RIGHT)
			c(HOME, HOME)
			c(BACK, BACKSPACE)
			c(0, N0)
			c(1, N1)
			c(2, N2)
			c(3, N3)
			c(4, N4)
			c(5, N5)
			c(6, N6)
			c(7, N7)
			c(8, N8)
			c(9, N9)
			c(STAR, MULTIPLY)
			c(DPAD_UP, UP)
			c(DPAD_DOWN, DOWN)
			c(DPAD_LEFT, LEFT)
			c(DPAD_RIGHT, RIGHT)
			c(A, A)
			c(B, B)
			c(C, C)
			c(D, D)
			c(E, E)
			c(F, F)
			c(G, G)
			c(H, H)
			c(I, I)
			c(J, J)
			c(K, K)
			c(L, L)
			c(M, M)
			c(N, N)
			c(O, O)
			c(P, P)
			c(Q, Q)
			c(R, R)
			c(S, S)
			c(T, T)
			c(U, U)
			c(V, V)
			c(W, W)
			c(X, X)
			c(Y, Y)
			c(Z, Z)
			c(PERIOD, PERIOD)
			c(ALT_LEFT, LALT)
			c(ALT_RIGHT, RALT)
			c(SHIFT_LEFT, LSHIFT)
			c(SHIFT_RIGHT, RSHIFT)
			c(SPACE, SPACE)
			c(TAB, TAB)
			c(ENTER, RETURN)
			c(DEL, DEL)
			c(MINUS, SUBTRACT)
			c(SLASH, DIVIDE)
			c(PLUS, ADD)
			c(PAGE_UP, PGUP)
			c(PAGE_DOWN, PGDN)
			Input::Keys::NONE; // ignore if not match any key
#undef c
	}

	int32_t onInputEvent(android_app* const app, AInputEvent* const ev) {
		switch(AInputEvent_getType(ev)) {
		case AINPUT_EVENT_TYPE_KEY: {
			switch(AKeyEvent_getAction(ev)) {
			case AKEY_EVENT_ACTION_UP:
				current_key_status_[to_easyrpg_keycode(AKeyEvent_getKeyCode(ev))] = false;
				break;
			case AKEY_EVENT_ACTION_DOWN:
				current_key_status_[to_easyrpg_keycode(AKeyEvent_getKeyCode(ev))] = false;
				break;
			default:
				return 0;
			}
			current_key_status_[Input::Keys::ALT]
				=  current_key_status_[Input::Keys::RALT]
				|| current_key_status_[Input::Keys::LALT];
			current_key_status_[Input::Keys::SHIFT]
				=  current_key_status_[Input::Keys::RSHIFT]
				|| current_key_status_[Input::Keys::LSHIFT];
			current_key_status_[Input::Keys::CTRL]
				=  current_key_status_[Input::Keys::RCTRL]
				|| current_key_status_[Input::Keys::LCTRL];

			return 1;
		} break;

		case AINPUT_EVENT_TYPE_MOTION: {
			unsigned const count = AMotionEvent_getPointerCount(ev);

			switch(AMotionEvent_getAction(ev)) {
			case AMOTION_EVENT_ACTION_UP:
			case AMOTION_EVENT_ACTION_POINTER_UP:
				for(unsigned i = 0; i < count; ++i) {
					motion_states_type::iterator const it =
						current_motion_states_.find(AMotionEvent_getPointerId(ev, i));
					assert(i != current_motion_states_.end());
					current_motion_states_.erase(it);
				}

			case AMOTION_EVENT_ACTION_DOWN:
			case AMOTION_EVENT_ACTION_POINTER_DOWN:
				for(unsigned i = 0; i < count; ++i) {
					int const id = AMotionEvent_getPointerId(ev, i);
					assert(current_motion_states_.find(id) == current_motion_states_.end());
					current_motion_states_.insert(std::make_pair
								  (id, std::make_pair
								   (AMotionEvent_getX(ev, i), AMotionEvent_getY(ev, i))));
				}

			case AMOTION_EVENT_ACTION_MOVE:
				for(unsigned i = 0; i < count; ++i) {
					motion_states_type::iterator const it =
						current_motion_states_.find(AMotionEvent_getPointerId(ev, i));
					assert(i != current_motion_states_.end());
					it->second = std::make_pair
						(AMotionEvent_getX(ev, i), AMotionEvent_getY(ev, i));
				}

			default:
				return 0;
			}
			return 1;
		} break;

		default: return 0;
		}
	}

}

AndroidUi::AndroidUi(android_app* const app)
	: app_(app), sensor_man_(ASensorManager_getInstance())
	, sensor_(ASensorManager_getDefaultSensor(sensor_man_, ASENSOR_TYPE_ACCELEROMETER))
	, sensor_queue_(ASensorManager_createEventQueue(sensor_man_, app_->looper,
											   LOOPER_ID_USER, NULL, NULL))
{
	assert(app_);
	app_->userData = this;
	app_->onAppCmd = &onAppCmd;
	app_->onInputEvent = &onInputEvent;
}

AudioInterface& AndroidUi::GetAudio() {
	return *audio_;
}

void AndroidUi::ProcessEvents() {
	android_poll_source* src;
	int events;
	int ident;

	while((ident = ALooper_pollAll(-1, NULL, &events, reinterpret_cast<void**>(&src))) >= 0) {
		if(src) { src->process(app_, src); }

		if (ident == LOOPER_ID_USER && sensor_) {
			ASensorEvent ev;
			while(ASensorEventQueue_getEvents(sensor_queue_, &ev, 1) > 0) {
				/*
				  ev.acceleration.x
				  ev.acceleration.y
				  ev.acceleration.z
				*/
			}
		}

		if(app_->destroyRequested != 0) {
			Player::exit_flag = true;
		}
	}
}

uint32_t AndroidUi::GetTicks() const {
	using namespace boost::chrono;
	return duration_cast<milliseconds>
		(system_clock::now().time_since_epoch()).count();
}

void AndroidUi::Sleep(uint32_t time) {
	boost::this_thread::sleep_for(boost::chrono::milliseconds(time));
}

void AndroidUi::BeginDisplayModeChange() {
}
void AndroidUi::EndDisplayModeChange() {
}
void AndroidUi::Resize(long width, long height) {
}
void AndroidUi::ToggleFullscreen() {
}
void AndroidUi::ToggleZoom() {
}
void AndroidUi::UpdateDisplay() {
}
void AndroidUi::BeginScreenCapture() {
}
BitmapRef AndroidUi::EndScreenCapture() {
}
void AndroidUi::SetTitle(const std::string &title) {
}
bool AndroidUi::ShowCursor(bool flag) {
	return false;
}
bool AndroidUi::IsFullscreen() {
	return false;
}
