#include <cstdio>
#include <vector>
#include <algorithm>
#include <functional>

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 1000

struct Point {
  Point() = default;

  Point(int x, int y) :
   x(x), y(y) {
  }

  int x { 0 };
  int y { 0 };
};

//---

struct Rect {
  Rect() = default;

  Rect(int x1, int y1, int x2, int y2) :
   x1(x1), y1(y1), x2(x2), y2(y2) {
  }

  bool overlaps(const Rect &rect) const {
    if (x2 < rect.x1 || x1 > rect.x2 || y2 < rect.y1 || y1 > rect.y2)
      return false;

    return true;
  }

  int x1 { 0 }, y1 { 0 };
  int x2 { 0 }, y2 { 0 };
};

//---

struct Util {
  static double random() {
    return (1.0*rand())/RAND_MAX;
  }
};

//---

class ImageList {
 public:
  ImageList() { }

  void addImage(Image *i) { images_.push_back(i); }

  void next() { ++ind_; if (ind_ >= int(images_.size())) ind_ = 0; }

  void reset() { ind_ = 0; }

  void draw(const Point &p) const {
    if (! images_.empty())
      App::drawImage(p.x, p.y, images_[ind_]);
  }

 private:
  using Images = std::vector<Image *>;

  int    ind_ { 0 };
  Images images_;
};

//---

class Graphic {
 public:
  Graphic(const Point &pos, int w, int h) :
   pos_(pos), w_(w), h_(h) {
  }

  virtual ~Graphic() { }

  const Point &getPos() const { return pos_; }

  void addImage(Image *i) { images_.addImage(i); }

  void nextImage() { images_.next(); }

  virtual void draw() {
    if (isDead()) return;

    Point pos(pos_.x - w_/2, pos_.y - h_/2);

    images_.draw(pos);
  }

  void reset() { dead_ = false; }

  bool isDead() const { return dead_; }
  void setDead(bool dead=true) { dead_ = dead; }

  Rect rect() const { return Rect(pos_.x - w_/2, pos_.y - h_/2, pos_.x + w_/2, pos_.y + h_/2); }

 protected:
  Point     pos_  { 0, 0 };
  ImageList images_;
  int       w_    { 0 } ;
  int       h_    { 0 } ;
  bool      dead_ { false };
};

//---

class ExplodeGraphic : public Graphic {
 public:
  ExplodeGraphic(const Point &pos, int w, int h) :
   Graphic(pos, w, h) {
  }

  bool isExploding() const { return exploding_ > 0; }

  void setExploding() { exploding_ = 4; }

  void reset() {
    Graphic::reset();

    exploding_ = 0;
  }

  virtual void draw() {
    if (isExploding())
      explodeImages_.draw(Point(pos_.x - 24, pos_.y - 24));
    else
      Graphic::draw();
  }

  virtual void update() {
    if (isExploding()) {
      --exploding_;

      if (exploding_ == 0)
        setDead();

      return;
    }
  }

 protected:
  int       exploding_ { 0 };
  ImageList explodeImages_;
};

//---

class Bullet : public Graphic {
 public:
  Bullet(const Point &pos, int w, int h) :
   Graphic(pos, w, h) {
  }

  virtual ~Bullet() { }

  virtual void update() = 0;
};

//---

class Alien;

class AlienBullet : public Bullet {
 private:
  enum { DY = 8 };

 public:
  AlienBullet(Alien *alien, const Point &pos) :
   Bullet(pos, 9, 26), alien_(alien) {
    addImage(App::loadImage("images/bullet2a.png"));
  }

  Alien *alien() const { return alien_; }

  void update() {
    pos_.y += DY;

    if (pos_.y >= SCREEN_HEIGHT)
      setDead();
  }

 private:
  Alien *alien_ { nullptr };
};

//---

class Player;

class PlayerBullet : public Bullet {
 private:
  enum { DY = 32 };

 public:
  PlayerBullet(Player *player, const Point &pos) :
   Bullet(pos, 4, 26), player_(player) {
    addImage(App::loadImage("images/bullet1a.png"));
  }

  Player *player() const { return player_; }

  void update() {
    pos_.y -= DY;

    if (pos_.y < 10)
      setDead();
  }

 private:
  Player *player_ { nullptr };
};

//---

class CSpaceInvaders;

class AlienManager {
 private:
  enum { NUM_BULLETS = 5 };

 public:
  AlienManager(CSpaceInvaders *invaders) :
   invaders_(invaders) {
    for (int y = 0; y < 5; ++y)
      row_y_[y] = y*60 + 110;

    bullets_.resize(NUM_BULLETS);

    for (uint i = 0; i < NUM_BULLETS; ++i)
      bullets_[i] = nullptr;
  }

  void reset() {
    speed_ = 8;

    for (int y = 0; y < 5; ++y)
      row_y_[y] = y*60 + 100;

    for (uint i = 0; i < NUM_BULLETS; ++i) {
      delete bullets_[i];

      bullets_[i] = nullptr;
    }
  }

  CSpaceInvaders *getInvaders() const { return invaders_; }

  int getWidth() const { return w_; }

  int getRowY(int row) const { return row_y_[row]; }

  int getDir() const { return dir_; }

  int getSpeed() const { return speed_/4; }

  void setNeedsIncRow(bool b=true) { needsIncRow_ = b; }

  void preUpdate();

  void postUpdate();

  void update();

  void incAlive() { ++numAlive_; }

  void fire(Alien *alien);

  void draw() {
    for (uint i = 0; i < NUM_BULLETS; ++i) {
      if (bullets_[i])
        bullets_[i]->draw();
    }
  }

  void checkHit(PlayerBullet *bullet);

 private:
  typedef std::vector<AlienBullet *> BulletList;

  CSpaceInvaders *invaders_    { nullptr };
  int             row_y_[5];
  int             dir_         { 1 };
  int             speed_       { 8 };
  int             w_           { 48 };
  int             numAlive_    { 0 };
  bool            needsIncRow_ { false };
  BulletList      bullets_;
};

//---

class Alien : public ExplodeGraphic {
 public:
  Alien(AlienManager *mgr, int col, int row, const Point &pos, int w, int h) :
   ExplodeGraphic(pos, w, h), mgr_(mgr), col_(col), row_(row) {
    dieSound_ = App::loadSound("sounds/invaderkilled.wav");

    explodeImages_.addImage(App::loadImage("images/explode1.png"));
  }

  virtual ~Alien() { }

  virtual int getScore() const = 0;

  void reset() {
    ExplodeGraphic::reset();

    imageCount_ = 0;

    pos_.x = 34*(2*col_ + 1);
  }

  void update();

  void draw() {
    pos_.y = mgr_->getRowY(row_);

    ExplodeGraphic::draw();
  }

  void checkHit(PlayerBullet *bullet);

 private:
  AlienManager *mgr_        { nullptr };
  int           col_        { 0 };
  int           row_        { 0 };
  int           imageCount_ { 4 };
  Sound        *dieSound_   { nullptr };
};

//---

class Alien1 : public Alien {
 public:
  Alien1(AlienManager *mgr, int col, int row, const Point &pos) :
   Alien(mgr, col, row, pos, 35, 35) {
    addImage(App::loadImage("images/invader1a.png"));
    addImage(App::loadImage("images/invader1b.png"));
  }

  int getScore() const { return 30; }
};

//---

class Alien2 : public Alien {
 public:
  Alien2(AlienManager *mgr, int col, int row, const Point &pos) :
   Alien(mgr, col, row, pos, 48, 35) {
    addImage(App::loadImage("images/invader2a.png"));
    addImage(App::loadImage("images/invader2b.png"));
  }

  int getScore() const { return 20; }
};

//---

class Alien3 : public Alien {
 public:
  Alien3(AlienManager *mgr, int col, int row, const Point &pos) :
   Alien(mgr, col, row, pos, 52, 35) {
    addImage(App::loadImage("images/invader3a.png"));
    addImage(App::loadImage("images/invader3b.png"));
  }

  int getScore() const { return 10; }
};

//---

class MysteryAlien : public ExplodeGraphic {
 private:
  enum { DX = -4 };

 public:
  MysteryAlien(CSpaceInvaders *invaders) :
   ExplodeGraphic(Point(0, 60), 71, 31), invaders_(invaders) {
    addImage(App::loadImage("images/mystery1a.png"));

    explodeImages_.addImage(App::loadImage("images/explode1.png"));

    dieSound_ = App::loadSound("sounds/invaderkilled.wav");

    dead_ = true;
  }

  int getScore() const {
    double r = Util::random();

    if      (r < 0.50) return 100;
    else if (r < 0.80) return 200;
    else if (r < 0.95) return 300;
    else               return 400;
  }

  void reset() {
    ExplodeGraphic::reset();

    dead_ = true;

    pos_.x = SCREEN_WIDTH + w_/2;
  }

  void update() {
    ExplodeGraphic::update();

    if (isDead()) return;

    pos_.x += DX;

    if (pos_.x < 0)
      setDead();
  }

  void checkHit(PlayerBullet *bullet);

 private:
  CSpaceInvaders *invaders_ { nullptr };
  Sound          *dieSound_ { nullptr };
};

//---

class Score {
 public:
  Score(const Point &pos) :
   pos_(pos) {
  }

  void add(int i) { score_ += i; }

  void draw() {
    char str[64];

    sprintf(str, "Score: %d", score_);

    App::drawCenteredText(pos_.x, pos_.y, str);
  }

  void reset() {
    score_ = 0;
  }

 private:
  Point pos_;
  int   score_ { 0 };
};

//---

class Player : public Graphic {
 private:
  enum { DX          = 8 };
  enum { NUM_LIVES   = 3 };
  enum { NUM_BULLETS = 5 };

 public:
  Player(CSpaceInvaders *invaders, const Point &pos) :
   Graphic(pos, 57, 35), invaders_(invaders), lives_(NUM_LIVES),
   d_(DX), fire_block_(0) {
    addImage(App::loadImage("images/player1a.png"));

    bullets_.resize(NUM_BULLETS);

    for (uint i = 0; i < NUM_BULLETS; ++i)
      bullets_[i] = nullptr;

    fireSound_ = App::loadSound("sounds/shoot.wav");
    dieSound_  = App::loadSound("sounds/explosion.wav");
  }

  void reset() {
    Graphic::reset();

    lives_      = NUM_LIVES;
    fire_block_ = 0;

    for (uint i = 0; i < NUM_BULLETS; ++i) {
      delete bullets_[i];

      bullets_[i] = nullptr;
    }
  }

  void moveLeft () {
    pos_.x -= d_;

    int hs = w_/2;

    if (pos_.x <  hs) pos_.x = hs;
  }

  void moveRight() {
    pos_.x += d_;

    int hs = w_/2;

    if (pos_.x >= SCREEN_WIDTH - hs) pos_.x = SCREEN_WIDTH - hs - 1;
  }

  void fire();

  void draw() {
    Graphic::draw();

    for (uint i = 0; i < NUM_BULLETS; ++i) {
      if (bullets_[i])
        bullets_[i]->draw();
    }

    char str[64];

    sprintf(str, "Lives: %d", lives_);

    App::drawLeftText(10, 10, str);
  }

  void update();

  void checkHit(AlienBullet *bullet);

 private:
  typedef std::vector<PlayerBullet *> BulletList;

  CSpaceInvaders *invaders_   { nullptr };
  int             lives_      { 0 };
  int             d_          { 0 };
  int             fire_block_ { 0 };
  BulletList      bullets_;
  Sound          *fireSound_  { nullptr };
  Sound          *dieSound_   { nullptr };
};

//---

class Base : public Graphic {
 private:
  struct Cell {
    ImageList images;
    int       ind  { 0 };
    bool      dead { false };

    Cell() { }

    void hit() { images.next(); ++ind; if (ind >= 4) dead = true; }

    void reset() { images.reset(); ind = 0; dead = false; }

    void addImage(Image *image) { images.addImage(image); }
  };

 public:
  Base(const Point &pos) :
   Graphic(pos, 87, 57) {
     grid_[0][0].addImage(App::loadImage("images/base1a_1_1.png"));
     grid_[0][1].addImage(App::loadImage("images/base1a_2_1.png"));
     grid_[0][2].addImage(App::loadImage("images/base1a_3_1.png"));
     grid_[0][3].addImage(App::loadImage("images/base1a_4_1.png"));
     grid_[1][0].addImage(App::loadImage("images/base1a_1_2.png"));
     grid_[1][1].addImage(App::loadImage("images/base1a_2_2.png"));
     grid_[1][2].addImage(App::loadImage("images/base1a_3_2.png"));
     grid_[1][3].addImage(App::loadImage("images/base1a_4_2.png"));

     grid_[0][0].addImage(App::loadImage("images/base1a_1_1.png"));
     grid_[0][1].addImage(App::loadImage("images/base1b_2_1.png"));
     grid_[0][2].addImage(App::loadImage("images/base1b_3_1.png"));
     grid_[0][3].addImage(App::loadImage("images/base1b_4_1.png"));
     grid_[1][0].addImage(App::loadImage("images/base1b_1_2.png"));
     grid_[1][1].addImage(App::loadImage("images/base1b_2_2.png"));
     grid_[1][2].addImage(App::loadImage("images/base1b_3_2.png"));
     grid_[1][3].addImage(App::loadImage("images/base1b_4_2.png"));

     grid_[0][0].addImage(App::loadImage("images/base1c_1_1.png"));
     grid_[0][1].addImage(App::loadImage("images/base1c_2_1.png"));
     grid_[0][2].addImage(App::loadImage("images/base1c_3_1.png"));
     grid_[0][3].addImage(App::loadImage("images/base1c_4_1.png"));
     grid_[1][0].addImage(App::loadImage("images/base1c_1_2.png"));
     grid_[1][1].addImage(App::loadImage("images/base1c_2_2.png"));
     grid_[1][2].addImage(App::loadImage("images/base1c_3_2.png"));
     grid_[1][3].addImage(App::loadImage("images/base1c_4_2.png"));

     grid_[0][0].addImage(App::loadImage("images/base1d_1_1.png"));
     grid_[0][1].addImage(App::loadImage("images/base1d_2_1.png"));
     grid_[0][2].addImage(App::loadImage("images/base1d_3_1.png"));
     grid_[0][3].addImage(App::loadImage("images/base1d_4_1.png"));
     grid_[1][0].addImage(App::loadImage("images/base1d_1_2.png"));
     grid_[1][1].addImage(App::loadImage("images/base1d_2_2.png"));
     grid_[1][2].addImage(App::loadImage("images/base1d_3_2.png"));
     grid_[1][3].addImage(App::loadImage("images/base1d_4_2.png"));
  }

  void reset() {
    for (int r = 0; r < 2; ++r)
      for (int c = 0; c < 4; ++c)
        grid_[r][c].reset();
  }

  void draw() {
    int dx = 22;
    int dy = 29;

    int x = pos_.x - w_/2;
    int y = pos_.y + h_/2;

    for (int r = 0; r < 2; ++r) {
      for (int c = 0; c < 4; ++c) {
        const Cell &cell = grid_[r][c];

        if (cell.dead) continue;

        cell.images.draw(Point(x + c*dx, y + r*dy));
      }
    }
  }

  void checkHit(Bullet *bullet) {
    int dx = 22;
    int dy = 29;

    int y1 = pos_.y + h_/2;
    int y2 = y1;

    for (int r = 0; r < 2; ++r) {
      y1 = y2;
      y2 = y1 + dy;

      int x1 = pos_.x - w_/2;
      int x2 = x1;

      for (int c = 0; c < 4; ++c) {
        x1 = x2;
        x2 = x1 + dx;

        Cell &cell = grid_[r][c];

        Rect rect(x1, y1, x2, y2);

        if (bullet->rect().overlaps(rect)) {
          if (cell.dead) continue;

          cell.hit();

          bullet->setDead();
        }
      }
    }
  }

 private:
  Cell grid_[2][4];
};

//---

class Level {
 public:
  Level() { }

  void draw() {
    char str[64];

    sprintf(str, "Level: %d", value_);

    App::drawRightText(SCREEN_WIDTH - 10, 10, str);
  }

  void reset() { value_ = 1; }

 private:
  int value_ { 1 };
};

//---

class CSpaceInvaders {
 public:
  CSpaceInvaders()  {
    init();
  }

  void init() {
    player_ = new Player(this, Point(400, 950));

    score_ = new Score(Point(SCREEN_WIDTH/2, 10));

    alienMgr_ = new AlienManager(this);

    mysteryAlien_ = new MysteryAlien(this);

    for (int i = 0; i < 4; ++i)
      addBase(Point(98*(2*i + 1), 840));

    for (int y = 0; y < 5; ++y) {
      for (int x = 0; x < 11; ++x) {
        addAlien(x, y, 34*(2*x + 1));
      }
    }
  }

  void draw() {
    level_.draw();

    score_->draw();

    player_->draw();

    for (auto &alien : aliens_)
      alien->draw();

    for (auto &base : bases_)
      base->draw();

    alienMgr_->draw();

    mysteryAlien_->draw();

    if (gameOver_)
      App::drawCenteredText(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, "GAME OVER");
  }

  void update() {
    if (paused_ || gameOver_) return;

    player_->update();

    alienMgr_->preUpdate();

    for (auto &alien : aliens_)
      alien->update();

    alienMgr_->postUpdate();

    alienMgr_->update();

    mysteryAlien_->update();

    if (mysteryAlien_->isDead()) {
      if (Util::random() < 0.01) {
        mysteryAlien_->reset();

        mysteryAlien_->setDead(false);
      }
    }
  }

  void addAlien(int x_ind, int y_ind, int x) {
    int y = alienMgr_->getRowY(y_ind);

    Point pos(x, y);

    if      (y_ind == 0              ) aliens_.push_back(new Alien1(alienMgr_, x_ind, y_ind, pos));
    else if (y_ind == 1 || y_ind == 2) aliens_.push_back(new Alien2(alienMgr_, x_ind, y_ind, pos));
    else if (y_ind == 3 || y_ind == 4) aliens_.push_back(new Alien3(alienMgr_, x_ind, y_ind, pos));
  }

  void addBase(const Point &pos) {
    bases_.push_back(new Base(pos));
  }

  void moveShipLeft() {
    if (paused_ || gameOver_) return;

    player_->moveLeft();
  }

  void moveShipRight() {
    if (paused_ || gameOver_) return;

    player_->moveRight();
  }

  void shipFire() {
    if (paused_ || gameOver_) return;

    player_->fire();
  }

  void checkAlienHit(PlayerBullet *bullet) {
    for (auto &alien : aliens_)
      alien->checkHit(bullet);

    alienMgr_->checkHit(bullet);

    mysteryAlien_->checkHit(bullet);
  }

  void checkPlayerHit(AlienBullet *bullet) {
    player_->checkHit(bullet);
  }

  void checkBaseHit(Bullet *bullet) {
    for (auto &base : bases_)
      base->checkHit(bullet);
  }

  void addScore(int score) {
    score_->add(score);
  }

  void pause() {
    paused_ = ! paused_;
  }

  void setGameOver(bool b=true) {
    gameOver_ = b;
  }

  void nextLevel() {
    paused_   = false;
    gameOver_ = false;

    for (auto &alien : aliens_)
      alien->reset();

    alienMgr_->reset();

    mysteryAlien_->reset();
  }

  void restart() {
    if (! paused_ && ! gameOver_) return;

    paused_   = false;
    gameOver_ = false;

    level_.reset();

    score_->reset();

    player_->reset();

    for (auto &alien : aliens_)
      alien->reset();

    for (auto &base : bases_)
      base->reset();

    alienMgr_->reset();
  }

 private:
  typedef std::vector<Alien *> AlienList;
  typedef std::vector<Base *>  BaseList;

  Player       *player_       { nullptr };
  Level         level_;
  Score        *score_        { nullptr };
  AlienManager *alienMgr_     { nullptr };
  AlienList     aliens_;
  MysteryAlien *mysteryAlien_ { nullptr };
  BaseList      bases_;
  bool          paused_       { false };
  bool          gameOver_     { false };
};

//--------------

void
Player::
fire()
{
  if (fire_block_ > 0) return;

  for (uint i = 0; i < NUM_BULLETS; ++i) {
    if (bullets_[i]) continue;

    bullets_[i] = new PlayerBullet(this, Point(pos_.x, pos_.y - h_/2));

    fire_block_ = 8;

    App::playSound(fireSound_);

    return;
  }
}

void
Player::
update()
{
  if (fire_block_ > 0) --fire_block_;

  for (uint i = 0; i < NUM_BULLETS; ++i) {
    if (! bullets_[i]) continue;

    bullets_[i]->update();

    if (! bullets_[i]->isDead())
      invaders_->checkAlienHit(bullets_[i]);

    if (! bullets_[i]->isDead())
      invaders_->checkBaseHit(bullets_[i]);

    if (bullets_[i]->isDead()) {
      delete bullets_[i];

      bullets_[i] = nullptr;
    }
  }
}

void
Player::
checkHit(AlienBullet *bullet)
{
  if (bullet->isDead()) return;

  if (isDead()) return;

  if (bullet->rect().overlaps(rect())) {
    --lives_;

    App::playSound(dieSound_);

    bullet->setDead();

    if (lives_ <= 0)
      invaders_->setGameOver();
  }
}

//--------------

void
AlienManager::
preUpdate()
{
  needsIncRow_ = false;

  numAlive_ = 0;
}

void
AlienManager::
postUpdate()
{
  if (needsIncRow_) {
    for (int y = 0; y < 5; ++y)
      row_y_[y] += w_/2;

    dir_ = -dir_;

    ++speed_;

    needsIncRow_ = false;
  }

  if (numAlive_ == 0)
    invaders_->nextLevel();
}

void
AlienManager::
update()
{
  for (uint i = 0; i < NUM_BULLETS; ++i) {
    if (! bullets_[i]) continue;

    bullets_[i]->update();

    if (! bullets_[i]->isDead())
      invaders_->checkPlayerHit(bullets_[i]);

    if (! bullets_[i]->isDead())
      invaders_->checkBaseHit(bullets_[i]);

    if (bullets_[i]->isDead()) {
      delete bullets_[i];

      bullets_[i] = nullptr;
    }
  }
}

void
AlienManager::
fire(Alien *alien)
{
  const Point &pos = alien->getPos();

  for (uint i = 0; i < NUM_BULLETS; ++i) {
    if (bullets_[i]) continue;

    bullets_[i] = new AlienBullet(alien, Point(pos.x, pos.y + 24));

    return;
  }
}

void
AlienManager::
checkHit(PlayerBullet *bullet)
{
  if (bullet->isDead()) return;

  for (uint i = 0; i < NUM_BULLETS; ++i) {
    if (! bullets_[i]) continue;

    if (bullet->rect().overlaps(bullets_[i]->rect())) {
      delete bullets_[i];

      bullets_[i] = nullptr;

      bullet->setDead();

      return;
    }
  }
}

//--------------

void
Alien::
checkHit(PlayerBullet *bullet)
{
  if (bullet->isDead()) return;

  if (isDead() || isExploding()) return;

  if (bullet->rect().overlaps(rect())) {
    setExploding();

    mgr_->getInvaders()->addScore(getScore());

    App::playSound(dieSound_);

    bullet->setDead();
  }
}

void
Alien::
update()
{
  ExplodeGraphic::update();

  if (isDead()) return;

  pos_.x += mgr_->getSpeed()*mgr_->getDir();

  if (isExploding()) return;

  int hs = mgr_->getWidth()/2;

  if (pos_.x >= SCREEN_WIDTH - hs) {
    //pos_.x = SCREEN_WIDTH - hs - 1;

    mgr_->setNeedsIncRow();
  }
  else if (pos_.x < hs) {
    //pos_.x = hs;

    mgr_->setNeedsIncRow();
  }

  --imageCount_;

  if (imageCount_ <= 0) {
    nextImage();

    imageCount_ = 4;
  }

  if (Util::random() < 0.01)
    mgr_->fire(this);

  mgr_->incAlive();

  if (mgr_->getRowY(row_) > 900)
    mgr_->getInvaders()->setGameOver();
}

//--------------

void
MysteryAlien::
checkHit(PlayerBullet *bullet)
{
  if (bullet->isDead()) return;

  if (isDead() || isExploding()) return;

  if (bullet->rect().overlaps(rect())) {
    setExploding();

    invaders_->addScore(getScore());

    App::playSound(dieSound_);

    bullet->setDead();
  }
}
