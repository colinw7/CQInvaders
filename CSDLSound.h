#ifndef CSDL_SOUND_H
#define CSDL_SOUND_H

#include <map>
#include <string>

#include <SDL/SDL_mixer.h>

class CSDLSound;

#define CSDLSoundMgrInst CSDLSoundMgr::getInstance()

class CSDLSoundMgr {
 public:
  static CSDLSoundMgr *getInstance();

 ~CSDLSoundMgr();

  CSDLSound *createSound(const char *filename);

  void playSound(CSDLSound *sound);
  void stopSound(CSDLSound *sound);

 private:
  void initAudio();
  void termAudio();

  void clearChannel(int channel);

 private:
  CSDLSoundMgr();

 private:
  typedef std::map<int,CSDLSound*> SoundList;

  bool      audioEnabled_;
  int       numChannels_;
  int       nextChannel_;
  SoundList sounds_;
};

class CSDLSound {
 public:
  CSDLSound(const char *filename);

 ~CSDLSound();

  void play();
  void stop();

  int getChannel() const { return channel_; }

  void setChannel(int channel) { channel_ = channel; }

  Mix_Chunk *getChunk() const { return chunk_; }

 private:
  std::string  filename_;
  Mix_Chunk   *chunk_;
  int          channel_;
};

#endif
