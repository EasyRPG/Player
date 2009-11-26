DEBUG = -g
SDL_CFLAGS = `sdl-config --cflags` $(DEBUG) -Wall -Wextra -ansi -pedantic -fno-rtti -fno-exceptions
SDL_LDFLAGS = `sdl-config --libs`
LDFLAGS = -lSDL_image -lSDL_gfx -lSDL_mixer $(SDL_LDFLAGS) $(DEBUG)

quickly:
	g++ -c *.cpp  -c $(SDL_CFLAGS)
	g++ *.o -o easyrpg $(LDFLAGS)

all: main.o log.o graphics.o scene_title.o main_data.o zobj.o sprite.o plane.o window.o tilemap.o audio.o window_base.o game_system.o bitmap.o input.o tone.o color.o rect.o viewport.o ldb_reader.o ldb_actor.o tools.o rpg_actor.o ldb_system.o ldb_sound.o rpg_sound.o ldb_music.o rpg_music.o rpg_system.o ldb_skill.o rpg_skill.o ldb_item.o rpg_item.o ldb_state.o rpg_state.o ldb_attribute.o rpg_attribute.o ldb_terrain.o rpg_terrain.o ldb_enemy.o rpg_enemy.o rpg_e_action.o
	g++ main.o log.o graphics.o scene_title.o main_data.o zobj.o sprite.o plane.o window.o tilemap.o audio.o window_base.o game_system.o bitmap.o input.o tone.o color.o rect.o viewport.o ldb_reader.o ldb_actor.o tools.o rpg_actor.o ldb_system.o ldb_sound.o rpg_sound.o ldb_music.o rpg_music.o rpg_system.o ldb_skill.o rpg_skill.o ldb_item.o rpg_item.o ldb_state.o rpg_state.o ldb_attribute.o rpg_attribute.o ldb_terrain.o rpg_terrain.o ldb_enemy.o rpg_enemy.o rpg_e_action.o -o easyrpg $(LDFLAGS)

clean:
	rm *.o easyrpg

main.o:
	g++ main.cpp -c $(SDL_CFLAGS)
	
log.o: log.h
	g++ log.cpp -c

graphics.o: graphics.h
	g++ graphics.cpp -c $(SDL_CFLAGS)

scene_title.o: scene_title.h
	g++ scene_title.cpp -c $(SDL_CFLAGS)

main_data.o: main_data.h
	g++ main_data.cpp -c $(SDL_CFLAGS)

zobj.o: zobj.h
	g++ zobj.cpp -c $(SDL_CFLAGS)

viewport.o: viewport.h
	g++ viewport.cpp -c $(SDL_CFLAGS)

sprite.o: sprite.h
	g++ sprite.cpp -c $(SDL_CFLAGS)

plane.o: plane.h
	g++ plane.cpp -c $(SDL_CFLAGS)

window.o: window.h
	g++ window.cpp -c $(SDL_CFLAGS)

tilemap.o: tilemap.h
	g++ tilemap.cpp -c $(SDL_CFLAGS)

audio.o: audio.h
	g++ audio.cpp -c $(SDL_CFLAGS)

window_base.o: window_base.h
	g++ window_base.cpp -c $(SDL_CFLAGS)

game_system.o: game_system.h
	g++ game_system.cpp -c $(SDL_CFLAGS)

bitmap.o: bitmap.h
	g++ bitmap.cpp -c $(SDL_CFLAGS)

input.o: input.h
	g++ input.cpp -c $(SDL_CFLAGS)

tone.o: tone.h
	g++ tone.cpp -c $(SDL_CFLAGS)

color.o: color.h
	g++ color.cpp -c $(SDL_CFLAGS)

rect.o: rect.h
	g++ rect.cpp -c $(SDL_CFLAGS)

ldb_reader.o: ldb_reader.h
	g++ ldb_reader.cpp -c $(SDL_CFLAGS)

ldb_actor.o:
	g++ ldb_actor.cpp -c $(SDL_CFLAGS)

tools.o: tools.h
	g++ tools.cpp -c $(SDL_CFLAGS)

rpg_actor.o: rpg_actor.h
	g++ rpg_actor.cpp -c $(SDL_CFLAGS)

ldb_system.o: ldb_system.h
	g++ ldb_system.cpp -c $(SDL_CFLAGS)

ldb_sound.o:
	g++ ldb_sound.cpp -c $(SDL_CFLAGS)

rpg_sound.o: rpg_sound.h
	g++ rpg_sound.cpp -c $(SDL_CFLAGS)

ldb_music.o:
	g++ ldb_music.cpp -c $(SDL_CFLAGS)

rpg_music.o: rpg_music.h
	g++ rpg_music.cpp -c $(SDL_CFLAGS)

rpg_system.o: rpg_system.h
	g++ rpg_system.cpp -c $(SDL_CFLAGS)

ldb_skill.o:
	g++ ldb_skill.cpp -c $(SDL_CFLAGS)

rpg_skill.o: rpg_skill.h
	g++ rpg_skill.cpp -c $(SDL_CFLAGS)

ldb_item.o:
	g++ ldb_item.cpp -c $(SDL_CFLAGS)

rpg_item.o: rpg_item.h
	g++ rpg_item.cpp -c $(SDL_CFLAGS)

ldb_state.o:
	g++ ldb_state.cpp -c $(SDL_CFLAGS)

rpg_state.o: rpg_state.h
	g++ rpg_state.cpp -c $(SDL_CFLAGS)

ldb_attribute.o:
	g++ ldb_attribute.cpp -c $(SDL_CFLAGS)

rpg_attribute.cpp: rpg_attribute.h
	g++ rpg_attribute.cpp -c $(SDL_CFLAGS)

ldb_terrain.o:
	g++ ldb_terrain.cpp -c $(SDL_CFLAGS)

rpg_terrain.o: rpg_terrain.h
	g++ rpg_terrain.cpp -c $(SDL_CFLAGS)

ldb_enemy.o:
	g++ ldb_enemy.cpp -c $(SDL_CFLAGS)

rpg_enemy.o: rpg_enemy.h
	g++ rpg_enemy.cpp -c $(SDL_CFLAGS)

rpg_e_action.o: rpg_e_action.h
	g++ rpg_e_action.cpp -c $(SDL_CFLAGS)

