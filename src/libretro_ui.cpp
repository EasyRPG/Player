#ifdef USE_LIBRETRO

// Headers
#include "libretro_ui.h"
#include "color.h"
#include "graphics.h"
#include "keys.h"
#include "output.h"
#include "player.h"
#include "bitmap.h"
#include <iostream>

#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include <glsm/glsm.h>
#endif

#include <cstring>
#include <stdio.h>

#ifdef SUPPORT_AUDIO
#include "audio_libretro.h"
AudioInterface& LibretroUi::GetAudio() {
	return *audio_;
}
#endif

retro_usec_t LibretroUi::time_in_microseconds=0;

	LibretroUi::LibretroUi(int width, int height){
		
		current_display_mode.width=width;
		current_display_mode.height=height;
		current_display_mode.bpp=32;
			const DynamicFormat format(
		32,
		0x000000FF,
		0x0000FF00,
		0x00FF0000,
		0xFF000000,
		PF::NoAlpha);
		Bitmap::SetFormat(Bitmap::ChooseFormat(format));
		main_surface.reset();
				main_surface = Bitmap::Create(current_display_mode.width,
											  current_display_mode.height,
											  false,
											  current_display_mode.bpp);
		#ifdef SUPPORT_AUDIO								  
		audio_.reset(new LibretroAudio());
		#endif								  
											  
	}

	LibretroUi::~LibretroUi(){
		
	}

	void LibretroUi::BeginDisplayModeChange(){
		
	}
	void LibretroUi::EndDisplayModeChange(){
		
	}
	void LibretroUi::Resize(long width, long height){
		
	}
	void LibretroUi::ToggleFullscreen(){
		
	}
	void LibretroUi::ToggleZoom(){
		
	}
	void LibretroUi::UpdateDisplay(){
		if(UpdateWindow==0){
			return;
		}
		
		if(!retro_gl_framebuffer_ready){
			UpdateWindow(NULL, SCREEN_TARGET_WIDTH, SCREEN_TARGET_HEIGHT, 0);
			return;
		}
		
			#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
   glsm_ctl(GLSM_CTL_STATE_BIND, NULL);

		
		GLuint TextureID;
		glViewport(0, 0, current_display_mode.width, current_display_mode.height);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // glEnable(GL_CULL_FACE);
         glEnable(GL_DEPTH_TEST);
		 glEnable(GL_TEXTURE_2D);
         glDepthFunc(GL_LEQUAL);
		/*glBegin(GL_TRIANGLES);
        glColor3f(0.1, 0.2, 0.3);
        glVertex3f(0, 0, 0);
        glVertex3f(1, 0, 0);
        glVertex3f(0, 1, 0);
        glEnd();*/
		
		glGenTextures(1,&TextureID);
		
		
		glBindTexture(GL_TEXTURE_2D,TextureID);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,current_display_mode.width,current_display_mode.height,0,GL_RGBA,GL_UNSIGNED_BYTE,main_surface->pixels());
		
		
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0.0,current_display_mode.width, 0.0, current_display_mode.height, -1.0, 1.0);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();


		glLoadIdentity();
		glDisable(GL_LIGHTING);
		
		  glBegin(GL_QUADS);
			glColor3f(1,1,1);
			glTexCoord2f(0,0); glVertex3f(0,current_display_mode.height,0);
			glTexCoord2f(1,0); glVertex3f(current_display_mode.width,current_display_mode.height,0);
			glTexCoord2f(1,1); glVertex3f(current_display_mode.width,0,0);
			glTexCoord2f(0,1); glVertex3f(0,0,0);
		  glEnd();
		  
		  glDisable(GL_TEXTURE_2D);
		  
			glPopMatrix();


			glMatrixMode(GL_PROJECTION);
			glPopMatrix();

			glMatrixMode(GL_MODELVIEW);
		  glDeleteTextures(1,&TextureID);

   glsm_ctl(GLSM_CTL_STATE_UNBIND, NULL);
#endif

   UpdateWindow(RETRO_HW_FRAME_BUFFER_VALID, SCREEN_TARGET_WIDTH, SCREEN_TARGET_HEIGHT, 0);
	}
	void LibretroUi::SetTitle(const std::string &title){
		
	}
	bool LibretroUi::ShowCursor(bool flag){
		return false;
	}

	void LibretroUi::ProcessEvents(){	
	if(CheckInputState==0){
		return;
	}
	
	keys[Input::Keys::UP]=(CheckInputState(0,RETRO_DEVICE_JOYPAD,0,RETRO_DEVICE_ID_JOYPAD_UP) != 0);       //UP
	keys[Input::Keys::DOWN]=(CheckInputState(0,RETRO_DEVICE_JOYPAD,0,RETRO_DEVICE_ID_JOYPAD_DOWN) != 0);   //DOWN
	keys[Input::Keys::LEFT]=(CheckInputState(0,RETRO_DEVICE_JOYPAD,0,RETRO_DEVICE_ID_JOYPAD_LEFT) != 0);   //LEFT
	keys[Input::Keys::RIGHT]=(CheckInputState(0,RETRO_DEVICE_JOYPAD,0,RETRO_DEVICE_ID_JOYPAD_RIGHT) != 0); //RIGHT
	keys[Input::Keys::SPACE]=(CheckInputState(0,RETRO_DEVICE_JOYPAD,0,RETRO_DEVICE_ID_JOYPAD_A) != 0);    //DECISION
	keys[Input::Keys::RETURN]=(CheckInputState(0,RETRO_DEVICE_JOYPAD,0,RETRO_DEVICE_ID_JOYPAD_START) != 0); //DECISION
	keys[Input::Keys::AC_BACK]=(CheckInputState(0,RETRO_DEVICE_JOYPAD,0,RETRO_DEVICE_ID_JOYPAD_B) != 0);   //CHANCEL
	/*keys[Input::Keys::Z]=(CheckInputState(0,RETRO_DEVICE_JOYPAD,0,RETRO_DEVICE_ID_JOYPAD_X) != 0);
	keys[Input::Keys::Z]=(CheckInputState(0,RETRO_DEVICE_JOYPAD,0,RETRO_DEVICE_ID_JOYPAD_Y) != 0);
	keys[Input::Keys::Z]=(CheckInputState(0,RETRO_DEVICE_JOYPAD,0,RETRO_DEVICE_ID_JOYPAD_SELECT) != 0);
	keys[Input::Keys::Z]=(CheckInputState(0,RETRO_DEVICE_JOYPAD,0,RETRO_DEVICE_ID_JOYPAD_L) != 0);
	keys[Input::Keys::Z]=(CheckInputState(0,RETRO_DEVICE_JOYPAD,0,RETRO_DEVICE_ID_JOYPAD_R) != 0);
	keys[Input::Keys::Z]=(CheckInputState(0,RETRO_DEVICE_JOYPAD,0,RETRO_DEVICE_ID_JOYPAD_L2) != 0);
	keys[Input::Keys::Z]=(CheckInputState(0,RETRO_DEVICE_JOYPAD,0,RETRO_DEVICE_ID_JOYPAD_R2) != 0);
	keys[Input::Keys::Z]=(CheckInputState(0,RETRO_DEVICE_JOYPAD,0,RETRO_DEVICE_ID_JOYPAD_L3) != 0);
	keys[Input::Keys::Z]=(CheckInputState(0,RETRO_DEVICE_JOYPAD,0,RETRO_DEVICE_ID_JOYPAD_R3) != 0);*/

	
		
/*	buttons[UP].push_back(Keys::UP);
	buttons[UP].push_back(Keys::K);
	buttons[UP].push_back(Keys::KP8);
	buttons[DOWN].push_back(Keys::DOWN);
	buttons[DOWN].push_back(Keys::J);
	buttons[DOWN].push_back(Keys::KP2);
	buttons[LEFT].push_back(Keys::LEFT);
	buttons[LEFT].push_back(Keys::H);
	buttons[LEFT].push_back(Keys::KP4);
	buttons[RIGHT].push_back(Keys::RIGHT);
	buttons[RIGHT].push_back(Keys::L);
	buttons[RIGHT].push_back(Keys::KP6);
	buttons[DECISION].push_back(Keys::Z);
	buttons[DECISION].push_back(Keys::Y);
	buttons[DECISION].push_back(Keys::SPACE);
	buttons[DECISION].push_back(Keys::RETURN);
	buttons[DECISION].push_back(Keys::SELECT);
	buttons[CANCEL].push_back(Keys::AC_BACK);
	buttons[CANCEL].push_back(Keys::X);
	buttons[CANCEL].push_back(Keys::C);
	buttons[CANCEL].push_back(Keys::V);
	buttons[CANCEL].push_back(Keys::B);
	buttons[CANCEL].push_back(Keys::N);
	buttons[CANCEL].push_back(Keys::ESCAPE);
	buttons[CANCEL].push_back(Keys::KP0);
	buttons[SHIFT].push_back(Keys::LSHIFT);
	buttons[SHIFT].push_back(Keys::RSHIFT);
	buttons[N0].push_back(Keys::N0);
	buttons[N1].push_back(Keys::N1);
	buttons[N2].push_back(Keys::N2);
	buttons[N3].push_back(Keys::N3);
	buttons[N4].push_back(Keys::N4);
	buttons[N5].push_back(Keys::N5);
	buttons[N6].push_back(Keys::N6);
	buttons[N7].push_back(Keys::N7);
	buttons[N8].push_back(Keys::N8);
	buttons[N9].push_back(Keys::N9);
	buttons[PLUS].push_back(Keys::ADD);
	buttons[MINUS].push_back(Keys::SUBTRACT);
	buttons[MULTIPLY].push_back(Keys::MULTIPLY);
	buttons[DIVIDE].push_back(Keys::DIVIDE);
	buttons[PERIOD].push_back(Keys::PERIOD);
	buttons[DEBUG_MENU].push_back(Keys::F9);
	buttons[DEBUG_THROUGH].push_back(Keys::LCTRL);
	buttons[DEBUG_THROUGH].push_back(Keys::RCTRL);
	buttons[DEBUG_SAVE].push_back(Keys::F11);
	buttons[TAKE_SCREENSHOT].push_back(Keys::F10);
	buttons[TOGGLE_FPS].push_back(Keys::F2);
	buttons[SHOW_LOG].push_back(Keys::F3);*/
	}

	bool LibretroUi::IsFullscreen(){
		return false;
	}

	uint32_t LibretroUi::GetTicks() const {
		//Despite it's name this function should obviously return the amount of milliseconds since the game started.
		return (uint32_t)(time_in_microseconds/1000);
	}
	void LibretroUi::Sleep(uint32_t time_milli){
		//Sleep is not needed libretro will ensure 60 fps
	}
	
	
	#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
	bool LibretroUi::retro_gl_framebuffer_ready = false;
	void LibretroUi::ResetRetroGLContext(void){
		   glsm_ctl(GLSM_CTL_STATE_CONTEXT_RESET, NULL);

		   if (!glsm_ctl(GLSM_CTL_STATE_SETUP, NULL))
			  return;
		  
		  retro_gl_framebuffer_ready=true;
	}
	void LibretroUi::DestroyRetroGLContext(void){
	}
	bool LibretroUi::LockRetroGLFramebuffer(void *data){
		   if (retro_gl_framebuffer_ready)
			  return false;
		   return true;
	}

	#endif

	retro_video_refresh_t LibretroUi::UpdateWindow=0;
	void LibretroUi::SetRetroVideoCallback(retro_video_refresh_t cb){
		UpdateWindow=cb;
	}
	
	retro_input_state_t LibretroUi::CheckInputState=0;
	void LibretroUi::SetRetroInputStateCallback(retro_input_state_t cb){
		CheckInputState=cb;
	}

#endif
