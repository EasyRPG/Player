#ifdef USE_LIBRETRO

// Headers
#include "libretro_ui.h"
#include "color.h"
#include "graphics.h"
#include "keys.h"
#include "output.h"
#include "player.h"
#include "bitmap.h"

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
		#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
		//RGBA
		const DynamicFormat format(
			32,
			0x000000FF,
			0x0000FF00,
			0x00FF0000,
			0xFF000000,
			PF::NoAlpha);		
		#else
		//BGRA
		const DynamicFormat format(
			32,
			0x00FF0000,
			0x0000FF00,
			0x000000FF,
			0xff000000,
			PF::NoAlpha);
		#endif
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
		

		
		#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
			if(!retro_gl_framebuffer_ready){
				UpdateWindow(NULL, SCREEN_TARGET_WIDTH, SCREEN_TARGET_HEIGHT, 0);
				return;
			}
			glsm_ctl(GLSM_CTL_STATE_BIND, NULL);

			
			GLuint TextureID;
			glViewport(0, 0, current_display_mode.width, current_display_mode.height);
			glClearColor(0, 0, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_TEXTURE_2D);
			glDepthFunc(GL_LEQUAL);
			
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
			
			UpdateWindow(RETRO_HW_FRAME_BUFFER_VALID, SCREEN_TARGET_WIDTH, SCREEN_TARGET_HEIGHT, 0);
		#else
		UpdateWindow(main_surface->pixels(), current_display_mode.width, current_display_mode.height, main_surface->pitch());
		#endif

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
