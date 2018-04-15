#ifndef CQSound_H
#define CQSound_H

#include <list>
#include <string>

#define CQSoundMgrInst CQSoundMgr::getInstance()

class QSound;
class CQSound;
class CSDLSound;

class CQSoundMgr {
 public:
  static CQSoundMgr *getInstance();

  void setActive(bool active);

  CQSound *addSound(const char *filename);

  void playSound(CQSound *sound);

 private:
  CQSoundMgr();
 ~CQSoundMgr() { }

 private:
  typedef std::list<CQSound *> SoundList;

  bool       active_;
  bool       qsound_;
  CSDLSound *sdl_sound_;
  SoundList  sounds_;
};

class CQSound {
 public:
  CQSound(const char *filename, bool qsound);

 ~CQSound();

  const std::string &getFilename() const { return filename_; }

  void play();

  void stop();

 private:
  std::string  filename_;
  QSound      *qsound_;
  CSDLSound   *sound_;
};

#endif
