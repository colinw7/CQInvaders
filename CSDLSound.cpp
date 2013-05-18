#include <CSDLSound.h>

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

#include <iostream>

CSDLSoundMgr *
CSDLSoundMgr::
getInstance()
{
  static CSDLSoundMgr *instance;

  if (! instance)
    instance = new CSDLSoundMgr;

  return instance;
}

CSDLSoundMgr::
CSDLSoundMgr() :
 audioEnabled_(false), numChannels_(4), nextChannel_(0), sounds_()
{
  SDL_Init(SDL_INIT_AUDIO);

  initAudio();
}

CSDLSoundMgr::
~CSDLSoundMgr()
{
  termAudio();

  SDL_Quit();
}

CSDLSound *
CSDLSoundMgr::
createSound(const char *filename)
{
  CSDLSound *sound = new CSDLSound(filename);

  return sound;
}

void
CSDLSoundMgr::
initAudio()
{
  if (! audioEnabled_) {
    int    audio_rate     = 22050;
    Uint16 audio_format   = AUDIO_S16SYS;
    int    audio_buffers  = 4096;

    if (Mix_OpenAudio(audio_rate, audio_format, numChannels_, audio_buffers) != 0) {
      std::cerr << "Error: Mix_OpenAudio failed " << Mix_GetError() << std::endl;
      return;
    }

    Mix_QuerySpec(&audio_rate, &audio_format, &numChannels_);

    for (int i = 0; i < numChannels_; ++i)
      sounds_[i] = NULL;

    audioEnabled_ = true;
  }
}

void
CSDLSoundMgr::
playSound(CSDLSound *sound)
{
  if (! audioEnabled_) return;

#if 0
  // set channel 0 to settings volume
  Mix_Volume(0, myConfig->readConfigInt("SoundVolume")*10);
#endif

#if 0
  //set 3d position for player
  int position = 180;
  int distance = 10;

  // set 3d effect
  if (! Mix_SetPosition(0, position, distance)) {
    std::cerr << "Error: Mix_SetPosition: %s\n", Mix_GetError());
    // no position effect, is it ok?
  }
#endif

  clearChannel(nextChannel_);

  int channel = Mix_PlayChannel(-1, sound->getChunk(), 0);

  if (channel == -1) {
    std::cerr << "Error: Mix_PlayChannel failed " << Mix_GetError() << std::endl;
    return;
  }

  clearChannel(channel);

  sounds_[channel] = sound;

  sound->setChannel(channel);
}

void
CSDLSoundMgr::
stopSound(CSDLSound *sound)
{
  int channel = sound->getChannel();

  if (channel >= 0)
    clearChannel(channel);

  sound->setChannel(-1);
}

void
CSDLSoundMgr::
clearChannel(int channel)
{
  if (sounds_[channel] != NULL) {
    Mix_HaltChannel(channel);

    sounds_[channel] = NULL;
  }
}

void
CSDLSoundMgr::
termAudio()
{
  if (audioEnabled_) {
    for (int i = 0; i < numChannels_; ++i)
      clearChannel(i);

    Mix_CloseAudio();

    audioEnabled_ = false;
  }
}

//------

CSDLSound::
CSDLSound(const char *filename) :
 filename_(filename), channel_(-1)
{
  chunk_ = Mix_LoadWAV(filename);

  if (chunk_ == NULL) {
    std::cerr << "Error: Mix_LoadWAV failed " << Mix_GetError() << std::endl;
    return;
  }
}

void
CSDLSound::
play()
{
  CSDLSoundMgrInst->playSound(this);
}

void
CSDLSound::
stop()
{
  CSDLSoundMgrInst->stopSound(this);
}

CSDLSound::
~CSDLSound()
{
  Mix_FreeChunk(chunk_);
}
