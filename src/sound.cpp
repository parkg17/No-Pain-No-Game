#include "sound.h"

irrklang::ISoundEngine* engine = nullptr;
irrklang::ISoundSource* sound_src = nullptr;
const char* sound_path = "../bin/sound/sound.mp3";

float sound_volume = 0.15f;

bool init_sound() {
	engine = irrklang::createIrrKlangDevice();
	if (!engine) return false;
	sound_src = engine->addSoundSourceFromFile(sound_path);
	sound_src->setDefaultVolume(sound_volume);
	return true;
}

void play_sound() {
	engine->play2D(sound_src, true);
}

void up_sound() {
	sound_volume += 0.02f;
	if (sound_volume > 0.5f) sound_volume = 0.5f;
	finalize_sound();
	init_sound();
	play_sound();
}

void down_sound() {
	sound_volume -= 0.02f;
	if (sound_volume < 0) sound_volume = 0.0f;
	finalize_sound();
	init_sound();
	play_sound();
}

void finalize_sound() {
	engine->drop();
}