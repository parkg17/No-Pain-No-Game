#include "sound.h"

irrklang::ISoundEngine* engine = nullptr;
irrklang::ISoundSource* sound_src = nullptr;
const char* sound_path = "../bin/sound/sound.mp3";

bool init_sound() {
	engine = irrklang::createIrrKlangDevice();
	if (!engine) return false;
	sound_src = engine->addSoundSourceFromFile(sound_path);
	sound_src->setDefaultVolume(0.2f);
	return true;
}

void play_sound() {
	engine->play2D(sound_src, true);
}

void finalize_sound() {
	engine->drop();
}