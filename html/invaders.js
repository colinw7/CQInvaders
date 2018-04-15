'use strict';

var invaders = new SpaceInvaders();

window.addEventListener("load", eventWindowLoaded, false);

//------

function eventWindowLoaded () {
  if (canvasSupport()) {
    invaders.init();

    (function drawFrame () {
      var canvas = document.getElementById("canvas");

      window.requestAnimationFrame(drawFrame, canvas);

      invaders.update();

      invaders.draw();
    }());
  }
}

function canvasSupport () {
  return true;
  //return Modernizr.canvas;
}

//------

function random () {
  return Math.random();
}

//------

function Point (x, y) {
  this.x = x;
  this.y = y;
}

//---

function Rect (x1, y1, x2, y2) {
  this.x1 = x1;
  this.y1 = y1;
  this.x2 = x2;
  this.y2 = y2;
}

Rect.prototype.overlaps = function(rect) {
  if (this.x2 < rect.x1 || this.x1 > rect.x2 || this.y2 < rect.y1 || this.y1 > rect.y2)
    return false;

  return true;
};

//---

function ImageList () {
  this.ind    = 0;
  this.images = [];
}

ImageList.prototype.addImage = function(i) {
  this.images.push(i);
};

ImageList.prototype.next = function() {
  ++this.ind;

  if (this.ind >= this.images.length)
    this.ind = 0;
};

ImageList.prototype.reset = function() {
  this.ind = 0;
};

ImageList.prototype.draw = function(p) {
  if (this.images.length > 0)
    invaders.drawImage(p.x, p.y, this.images[this.ind]);
};

//---

function Graphic (pos, w, h) {
  this.pos    = pos;
  this.images = new ImageList();
  this.w      = w;
  this.h      = h;
  this.dead   = false;
}

Graphic.prototype.addImage = function(i) {
  this.images.addImage(i);
};

Graphic.prototype.nextImage = function() {
  this.images.next();
};

Graphic.prototype.draw = function() {
  if (this.isDead()) return;

  var pos = new Point(this.pos.x - this.w/2, this.pos.y - this.h/2);

  this.images.draw(pos);
};

Graphic.prototype.reset = function() {
  this.dead = false;
};

Graphic.prototype.isDead = function() {
  return this.dead;
};

Graphic.prototype.setDead = function(dead) {
  this.dead = dead;
};

Graphic.prototype.rect = function() {
  return new Rect(this.pos.x - this.w/2, this.pos.y - this.h/2,
                  this.pos.x + this.w/2, this.pos.y + this.h/2);
};

//---

function ExplodeGraphic (pos, w, h) {
  // public Graphic ...
  Graphic.call(this, pos, w, h);

  ExplodeGraphic.prototype.addImage  = Graphic.prototype.addImage;
  ExplodeGraphic.prototype.isDead    = Graphic.prototype.isDead;
  ExplodeGraphic.prototype.setDead   = Graphic.prototype.setDead;
  ExplodeGraphic.prototype.nextImage = Graphic.prototype.nextImage;
  ExplodeGraphic.prototype.rect      = Graphic.prototype.rect;

  //---

  this.pos = pos;
  this.w   = w;
  this.h   = h;

  this.exploding     = 0;
  this.explodeImages = new ImageList();
}

ExplodeGraphic.prototype.isExploding = function() {
  return this.exploding > 0;
};

ExplodeGraphic.prototype.setExploding = function() {
  this.exploding = 4;
};

ExplodeGraphic.prototype.reset = function() {
  Graphic.prototype.reset.call(this);

  this.exploding = 0;
};

ExplodeGraphic.prototype.draw = function() {
  if (ExplodeGraphic.prototype.isExploding.call(this))
    this.explodeImages.draw(new Point(this.pos.x - 24, this.pos.y - 24));
  else
    Graphic.prototype.draw.call(this);
};

ExplodeGraphic.prototype.update = function() {
  if (ExplodeGraphic.prototype.isExploding.call(this)) {
    --this.exploding;

    if (this.exploding === 0)
      this.setDead(true);

    return;
  }
};

//---

function Bullet (pos, w, h) {
  // public Graphic ...
  Graphic.call(this, pos, w, h);

  Bullet.prototype.addImage = Graphic.prototype.addImage;
  Bullet.prototype.isDead   = Graphic.prototype.isDead;
  Bullet.prototype.setDead  = Graphic.prototype.setDead;
  Bullet.prototype.rect     = Graphic.prototype.rect;
}

Bullet.prototype.update = function() {
};

Bullet.prototype.draw = function() {
  Graphic.prototype.draw.call(this);
};

//---

function AlienBullet (alien, pos) {
  // public Bullet ...
  Bullet.call(this, pos, 9, 26);

  AlienBullet.prototype.addImage = Bullet.prototype.addImage;
  AlienBullet.prototype.isDead   = Bullet.prototype.isDead;
  AlienBullet.prototype.setDead  = Bullet.prototype.setDead;
  AlienBullet.prototype.rect     = Bullet.prototype.rect;
  AlienBullet.prototype.draw     = Bullet.prototype.draw;

  //---

  this.DY = 6;

  this.alien = alien;
}

AlienBullet.prototype.init = function() {
  this.addImage(invaders.loadImage("images/bullet2a.png"));
};

AlienBullet.prototype.update = function() {
  this.pos.y += this.DY;

  if (this.pos.y >= invaders.SCREEN_HEIGHT)
    this.setDead(true);
};

//---

function PlayerBullet (player, pos) {
  // public Bullet ...
  Bullet.call(this, pos, 4, 26);

  PlayerBullet.prototype.addImage = Bullet.prototype.addImage;
  PlayerBullet.prototype.isDead   = Bullet.prototype.isDead;
  PlayerBullet.prototype.setDead  = Bullet.prototype.setDead;
  PlayerBullet.prototype.rect     = Bullet.prototype.rect;
  PlayerBullet.prototype.draw     = Bullet.prototype.draw;

  //---

  this.DY = 16;

  this.player = player;
}

PlayerBullet.prototype.init = function() {
  this.addImage(invaders.loadImage("images/bullet1a.png"));
};

PlayerBullet.prototype.update = function() {
  this.pos.y -= this.DY;

  if (this.pos.y < 10)
    this.setDead(true);
};

//---

function AlienManager () {
  this.NUM_BULLETS = 5;

  this.row_y = [];

  for (var y = 0; y < 5; ++y)
    this.row_y[y] = y*60 + 110;

  this.dir = 1;

  this.speed    = 0.5;
  this.w        = 48;
  this.numAlive = 0;

  this.needsIncRow = false;

  this.bullets = [];

  for (var i = 0; i < this.NUM_BULLETS; ++i) {
    this.bullets[i] = null;
  }
}

AlienManager.prototype.reset = function() {
  this.speed = 0.5;

  for (var y = 0; y < 5; ++y)
    this.row_y[y] = y*60 + 100;

  for (var i = 0; i < this.NUM_BULLETS; ++i) {
    this.bullets[i] = null;
  }
};

AlienManager.prototype.getWidth = function() {
  return this.w;
};

AlienManager.prototype.getRowY = function(row) {
  return this.row_y[row];
};

AlienManager.prototype.getDir = function() {
  return this.dir;
};

AlienManager.prototype.getSpeed = function() {
  return this.speed;
};

AlienManager.prototype.preUpdate = function() {
  this.needsIncRow = false;

  this.numAlive = 0;
};

AlienManager.prototype.postUpdate = function() {
  if (this.needsIncRow) {
    for (var y = 0; y < 5; ++y)
      this.row_y[y] += this.w/2;

    this.dir = -this.dir;

    this.speed *= 1.1;

    this.needsIncRow = false;
  }

  if (this.numAlive == 0)
    invaders.nextLevel();
};

AlienManager.prototype.update = function() {
  for (var i = 0; i < this.NUM_BULLETS; ++i) {
    var bullet = this.bullets[i];

    if (! bullet) continue;

    bullet.update();

    if (! bullet.isDead())
      invaders.checkPlayerHit(bullet);

    if (! bullet.isDead())
      invaders.checkBaseHit(bullet);

    if (bullet.isDead()) {
      this.bullets[i] = null;
    }
  }
};

AlienManager.prototype.incAlive = function() {
  ++this.numAlive;
};

AlienManager.prototype.fire = function(alien) {
  var pos = alien.pos;

  for (var i = 0; i < this.NUM_BULLETS; ++i) {
    var bullet = this.bullets[i];

    if (bullet) continue;

    this.bullets[i] = new AlienBullet(alien, new Point(pos.x, pos.y + 24));

    this.bullets[i].init();

    return;
  }
};

AlienManager.prototype.draw = function() {
  for (var i = 0; i < this.NUM_BULLETS; ++i) {
    var bullet = this.bullets[i];

    if (bullet)
      bullet.draw();
  }
};

AlienManager.prototype.checkHit = function(bullet) {
  if (bullet.isDead()) return;

  for (var i = 0; i < this.NUM_BULLETS; ++i) {
    var bullet1 = this.bullets[i];

    if (! bullet1) continue;

    if (bullet1.rect().overlaps(bullet.rect())) {
      this.bullets[i] = null;

      bullet.setDead(true);

      return;
    }
  }
};

//---

function Alien (col, row, pos, w, h) {
  // public ExplodeGraphic ...
  ExplodeGraphic.call(this, pos, w, h);

  Alien.prototype.addImage     = ExplodeGraphic.prototype.addImage;
  Alien.prototype.isDead       = ExplodeGraphic.prototype.isDead;
  Alien.prototype.setDead      = ExplodeGraphic.prototype.setDead;
  Alien.prototype.isExploding  = ExplodeGraphic.prototype.isExploding;
  Alien.prototype.setExploding = ExplodeGraphic.prototype.setExploding;
  Alien.prototype.nextImage    = ExplodeGraphic.prototype.nextImage;
  Alien.prototype.rect         = ExplodeGraphic.prototype.rect;

  //---

  this.IMAGE_COUNT = 12;

  this.col = col;
  this.row = row;

  this.imageCount = this.IMAGE_COUNT;
}

Alien.prototype.init = function() {
  this.dieSound = invaders.loadSound("sounds/invaderkilled.wav");

  this.explodeImages.addImage(invaders.loadImage("images/explode1.png"));
};

Alien.prototype.getScore = function() {
  return 0;
};

Alien.prototype.reset = function() {
  ExplodeGraphic.prototype.reset.call(this);

  this.imageCount = 0;

  this.pos.x = 34*(2*this.col + 1);
};

Alien.prototype.update = function() {
  ExplodeGraphic.prototype.update.call(this);

  if (this.isDead()) return;

  this.pos.x += invaders.alienMgr.getSpeed()*invaders.alienMgr.getDir();

  if (this.isExploding()) return;

  var hs = invaders.alienMgr.getWidth()/2;

  if (this.pos.x >= invaders.SCREEN_WIDTH - hs) {
    //this.pos.x = invaders.SCREEN_WIDTH - hs - 1;

    invaders.alienMgr.needsIncRow = true;
  }
  else if (this.pos.x < hs) {
    //this.pos.x = hs;

    invaders.alienMgr.needsIncRow = true;
  }

  --this.imageCount;

  if (this.imageCount <= 0) {
    this.nextImage();

    this.imageCount = this.IMAGE_COUNT;
  }

  if (random() < 0.01)
    invaders.alienMgr.fire(this);

  invaders.alienMgr.incAlive();

  if (invaders.alienMgr.getRowY(this.row) > 900)
    invaders.gameOver = true;
};

Alien.prototype.draw = function() {
  this.pos.y = invaders.alienMgr.getRowY(this.row);

  ExplodeGraphic.prototype.draw.call(this);
};

Alien.prototype.checkHit = function(bullet) {
  if (bullet.isDead()) return;

  if (this.isDead() || this.isExploding()) return;

  if (bullet.rect().overlaps(this.rect())) {
    this.setExploding();

    invaders.addScore(this.getScore());

    invaders.playSound(this.dieSound);

    bullet.setDead(true);
  }
};

//---

function Alien1 (col, row, pos) {
  // public Alien ...
  Alien.call(this, col, row, pos, 35, 35);

  Alien1.prototype.addImage     = Alien.prototype.addImage;
  Alien1.prototype.update       = Alien.prototype.update;
  Alien1.prototype.isDead       = Alien.prototype.isDead;
  Alien1.prototype.setDead      = Alien.prototype.setDead;
  Alien1.prototype.isExploding  = Alien.prototype.isExploding;
  Alien1.prototype.setExploding = Alien.prototype.setExploding;
  Alien1.prototype.nextImage    = Alien.prototype.nextImage;
  Alien1.prototype.draw         = Alien.prototype.draw;
  Alien1.prototype.checkHit     = Alien.prototype.checkHit;
  Alien1.prototype.rect         = Alien.prototype.rect;
  Alien1.prototype.reset        = Alien.prototype.reset;
}

Alien1.prototype.init = function() {
  this.addImage(invaders.loadImage("images/invader1a.png"));
  this.addImage(invaders.loadImage("images/invader1b.png"));

  Alien.prototype.init.call(this);
};

Alien1.prototype.getScore = function() {
  return 30;
};

//---

function Alien2 (col, row, pos) {
  // public Alien ...
  Alien.call(this, col, row, pos, 48, 35);

  Alien2.prototype.addImage     = Alien.prototype.addImage;
  Alien2.prototype.update       = Alien.prototype.update;
  Alien2.prototype.isDead       = Alien.prototype.isDead;
  Alien2.prototype.setDead      = Alien.prototype.setDead;
  Alien2.prototype.isExploding  = Alien.prototype.isExploding;
  Alien2.prototype.setExploding = Alien.prototype.setExploding;
  Alien2.prototype.nextImage    = Alien.prototype.nextImage;
  Alien2.prototype.draw         = Alien.prototype.draw;
  Alien2.prototype.checkHit     = Alien.prototype.checkHit;
  Alien2.prototype.rect         = Alien.prototype.rect;
  Alien2.prototype.reset        = Alien.prototype.reset;
}

Alien2.prototype.init = function() {
  this.addImage(invaders.loadImage("images/invader2a.png"));
  this.addImage(invaders.loadImage("images/invader2b.png"));

  Alien.prototype.init.call(this);
};

Alien2.prototype.getScore = function() {
  return 20;
};

//---

function Alien3 (col, row, pos) {
  // public Alien ...
  Alien.call(this, col, row, pos, 52, 35);

  Alien3.prototype.addImage     = Alien.prototype.addImage;
  Alien3.prototype.update       = Alien.prototype.update;
  Alien3.prototype.isDead       = Alien.prototype.isDead;
  Alien3.prototype.setDead      = Alien.prototype.setDead;
  Alien3.prototype.isExploding  = Alien.prototype.isExploding;
  Alien3.prototype.setExploding = Alien.prototype.setExploding;
  Alien3.prototype.nextImage    = Alien.prototype.nextImage;
  Alien3.prototype.draw         = Alien.prototype.draw;
  Alien3.prototype.checkHit     = Alien.prototype.checkHit;
  Alien3.prototype.rect         = Alien.prototype.rect;
  Alien3.prototype.reset        = Alien.prototype.reset;
}

Alien3.prototype.init = function() {
  this.addImage(invaders.loadImage("images/invader3a.png"));
  this.addImage(invaders.loadImage("images/invader3b.png"));

  Alien.prototype.init.call(this);
};

Alien3.prototype.getScore = function() {
  return 10;
};

//---

function MysteryAlien (col, row, pos) {
  // public ExplodeGraphic ...
  ExplodeGraphic.call(this, new Point(0, 60), 71, 31);

  MysteryAlien.prototype.addImage     = ExplodeGraphic.prototype.addImage;
  MysteryAlien.prototype.isDead       = ExplodeGraphic.prototype.isDead;
  MysteryAlien.prototype.setDead      = ExplodeGraphic.prototype.setDead;
  MysteryAlien.prototype.isExploding  = ExplodeGraphic.prototype.isExploding;
  MysteryAlien.prototype.setExploding = ExplodeGraphic.prototype.setExploding;
  MysteryAlien.prototype.draw         = ExplodeGraphic.prototype.draw;
  MysteryAlien.prototype.rect         = ExplodeGraphic.prototype.rect ;

  //---

  this.DX = -3;

  this.dead = true;
}

MysteryAlien.prototype.init = function() {
  this.addImage(invaders.loadImage("images/mystery1a.png"));

  this.explodeImages.addImage(invaders.loadImage("images/explode1.png"));

  this.dieSound = invaders.loadSound("sounds/invaderkilled.wav");
};

MysteryAlien.prototype.getScore = function() {
  var r = random();

  if      (r < 0.50) return 100;
  else if (r < 0.80) return 200;
  else if (r < 0.95) return 300;
  else               return 400;
};

MysteryAlien.prototype.reset = function() {
  ExplodeGraphic.prototype.reset.call(this);

  this.dead = true;

  this.pos.x = invaders.SCREEN_WIDTH + this.w/2;
};

MysteryAlien.prototype.update = function() {
  if (this.isDead()) return;

  ExplodeGraphic.prototype.update.call(this);

  this.pos.x += this.DX;

  if (this.pos.x < 0)
    this.setDead(true);
};

MysteryAlien.prototype.checkHit = function(bullet) {
  if (bullet.isDead()) return;

  if (this.isDead() || this.isExploding()) return;

  if (bullet.rect().overlaps(this.rect())) {
    this.setExploding();

    invaders.addScore(this.getScore());

    invaders.playSound(this.dieSound);

    bullet.setDead(true);
  }
};

//---

function Score (pos) {
  this.pos   = pos;
  this.score = 0;
}

Score.prototype.add = function(i) {
  this.score += i;
};

Score.prototype.draw = function() {
  var str = "Score: ";

  str += String(this.score);

  invaders.drawCenteredText(this.pos.x, this.pos.y, str);
};

Score.prototype.reset = function() {
  this.score = 0;
};

//---

function Player (pos) {
  // public Graphic ...
  Graphic.call(this, new Point(0, 0), 57, 35);

  Player.prototype.addImage = Graphic.prototype.addImage;
  Player.prototype.isDead   = Graphic.prototype.isDead;
  Player.prototype.setDead  = Graphic.prototype.setDead;
  Player.prototype.rect     = Graphic.prototype.rect;

  //---

  this.DX          = 3;
  this.NUM_LIVES   = 4;
  this.NUM_BULLETS = 5;

  this.lives = this.NUM_LIVES;

  this.pos.x = invaders.SCREEN_WIDTH/2;
  this.pos.y = invaders.SCREEN_HEIGHT - 50;

  this.dx = 0
//this.d = this.DX;

  this.fire_block = 0;

  this.bullets = [];

  for (var i = 0; i < this.NUM_BULLETS; ++i)
    this.bullets[i] = null;
}

Player.prototype.init = function() {
  this.addImage(invaders.loadImage("images/player1a.png"));

  this.fireSound = invaders.loadSound("sounds/shoot.wav");
  this.dieSound  = invaders.loadSound("sounds/explosion.wav");
};

Player.prototype.reset = function() {
  Graphic.prototype.reset.call(this);

  this.pos.x = invaders.SCREEN_WIDTH/2;
  this.pos.y = invaders.SCREEN_HEIGHT - 50;

  this.lives = this.NUM_LIVES;

  this.dx = 0;

  this.fire_block = 0;

  for (var i = 0; i < this.NUM_BULLETS; ++i)
    this.bullets[i] = null;
};

Player.prototype.moveLeft = function() {
  this.dx = -this.DX;

  //this.pos.x -= this.d;

  //this.fixPos();
};

Player.prototype.moveRight = function() {
  this.dx = this.DX;

  //this.pos.x += this.d;

  //this.fixPos();
};

Player.prototype.moveStop = function() {
  this.dx = 0;
};

Player.prototype.fire = function() {
  if (this.fire_block > 0) return;

  for (var i = 0; i < this.NUM_BULLETS; ++i) {
    if (this.bullets[i]) continue;

    this.bullets[i] = new PlayerBullet(this, new Point(this.pos.x, this.pos.y - this.h/2));

    this.bullets[i].init();

    this.fire_block = 8;

    invaders.playSound(this.fireSound);

    return;
  }
};

Player.prototype.draw = function() {
  Graphic.prototype.draw.call(this);

  for (var i = 0; i < this.NUM_BULLETS; ++i) {
    if (this.bullets[i])
      this.bullets[i].draw();
  }

  var str = "Lives: ";

  str += String(this.lives);

  invaders.drawLeftText(10, 10, str);
};

Player.prototype.update = function() {
  if (this.isDead()) return;

  this.pos.x += this.dx;

  this.fixPos();

  if (this.fire_block > 0)
    --this.fire_block;

  for (var i = 0; i < this.NUM_BULLETS; ++i) {
    if (! this.bullets[i]) continue;

    this.bullets[i].update();

    if (! this.bullets[i].isDead())
      invaders.checkAlienHit(this.bullets[i]);

    if (! this.bullets[i].isDead())
      invaders.checkBaseHit(this.bullets[i]);

    if (this.bullets[i].isDead()) {
      this.bullets[i] = null;
    }
  }
};

Player.prototype.fixPos = function() {
  var hs = this.w/2;

  if (this.pos.x < hs)
    this.pos.x = hs;

  if (this.pos.x >= invaders.SCREEN_WIDTH - hs)
    this.pos.x = invaders.SCREEN_WIDTH - hs - 1;
};

Player.prototype.checkHit = function(bullet) {
  if (bullet.isDead()) return;

  if (this.isDead()) return;

  if (bullet.rect().overlaps(this.rect())) {
    --this.lives;

    invaders.playSound(this.dieSound);

    bullet.setDead(true);

    if (this.lives <= 0)
      invaders.gameOver = true;
  }
};

//---

function Cell () {
  this.images = new ImageList();
  this.ind    = 0;
  this.dead   = false;
}

Cell.prototype.hit = function() {
  this.images.next();

  ++this.ind;

  if (this.ind >= 4)
    this.dead = true;
};

Cell.prototype.reset = function() {
  this.images.reset();

  this.ind = 0;

  this.dead = false;
};

Cell.prototype.addImage = function(image) {
  this.images.addImage(image);
};

//---

function Base (pos) {
  // public Graphic ...
  Graphic.call(this, pos, 87, 57);

  Base.prototype.addImage = Graphic.prototype.addImage;

  //---

  this.grid = [];

  this.grid[0] = [];
  this.grid[1] = [];

  for (var i1 = 0; i1 < 2; ++i1) {
    for (var i2 = 0; i2 < 4; ++i2) {
      this.grid[i1][i2] = new Cell();
    }
  }
}

Base.prototype.init = function() {
  this.grid[0][0].addImage(invaders.loadImage("images/base1a_1_1.png"));
  this.grid[0][1].addImage(invaders.loadImage("images/base1a_2_1.png"));
  this.grid[0][2].addImage(invaders.loadImage("images/base1a_3_1.png"));
  this.grid[0][3].addImage(invaders.loadImage("images/base1a_4_1.png"));
  this.grid[1][0].addImage(invaders.loadImage("images/base1a_1_2.png"));
  this.grid[1][1].addImage(invaders.loadImage("images/base1a_2_2.png"));
  this.grid[1][2].addImage(invaders.loadImage("images/base1a_3_2.png"));
  this.grid[1][3].addImage(invaders.loadImage("images/base1a_4_2.png"));

  this.grid[0][0].addImage(invaders.loadImage("images/base1a_1_1.png"));
  this.grid[0][1].addImage(invaders.loadImage("images/base1b_2_1.png"));
  this.grid[0][2].addImage(invaders.loadImage("images/base1b_3_1.png"));
  this.grid[0][3].addImage(invaders.loadImage("images/base1b_4_1.png"));
  this.grid[1][0].addImage(invaders.loadImage("images/base1b_1_2.png"));
  this.grid[1][1].addImage(invaders.loadImage("images/base1b_2_2.png"));
  this.grid[1][2].addImage(invaders.loadImage("images/base1b_3_2.png"));
  this.grid[1][3].addImage(invaders.loadImage("images/base1b_4_2.png"));

  this.grid[0][0].addImage(invaders.loadImage("images/base1c_1_1.png"));
  this.grid[0][1].addImage(invaders.loadImage("images/base1c_2_1.png"));
  this.grid[0][2].addImage(invaders.loadImage("images/base1c_3_1.png"));
  this.grid[0][3].addImage(invaders.loadImage("images/base1c_4_1.png"));
  this.grid[1][0].addImage(invaders.loadImage("images/base1c_1_2.png"));
  this.grid[1][1].addImage(invaders.loadImage("images/base1c_2_2.png"));
  this.grid[1][2].addImage(invaders.loadImage("images/base1c_3_2.png"));
  this.grid[1][3].addImage(invaders.loadImage("images/base1c_4_2.png"));

  this.grid[0][0].addImage(invaders.loadImage("images/base1d_1_1.png"));
  this.grid[0][1].addImage(invaders.loadImage("images/base1d_2_1.png"));
  this.grid[0][2].addImage(invaders.loadImage("images/base1d_3_1.png"));
  this.grid[0][3].addImage(invaders.loadImage("images/base1d_4_1.png"));
  this.grid[1][0].addImage(invaders.loadImage("images/base1d_1_2.png"));
  this.grid[1][1].addImage(invaders.loadImage("images/base1d_2_2.png"));
  this.grid[1][2].addImage(invaders.loadImage("images/base1d_3_2.png"));
  this.grid[1][3].addImage(invaders.loadImage("images/base1d_4_2.png"));
};

Base.prototype.reset = function() {
  for (var r = 0; r < 2; ++r)
    for (var c = 0; c < 4; ++c)
      this.grid[r][c].reset();
};

Base.prototype.draw = function() {
  var dx = 22;
  var dy = 29;

  var x = this.pos.x - this.w/2;
  var y = this.pos.y + this.h/2;

  for (var r = 0; r < 2; ++r) {
    for (var c = 0; c < 4; ++c) {
      var cell = this.grid[r][c];

      if (cell.dead) continue;

      cell.images.draw(new Point(x + c*dx, y + r*dy));
    }
  }
};

Base.prototype.checkHit = function(bullet) {
  var dx = 22;
  var dy = 29;

  var y1 = this.pos.y + this.h/2;
  var y2 = y1;

  for (var r = 0; r < 2; ++r) {
    y1 = y2;
    y2 = y1 + dy;

    var x1 = this.pos.x - this.w/2;
    var x2 = x1;

    for (var c = 0; c < 4; ++c) {
      x1 = x2;
      x2 = x1 + dx;

      var cell = this.grid[r][c];

      var rect = new Rect(x1, y1, x2, y2);

      if (bullet.rect().overlaps(rect)) {
        if (cell.dead) continue;

        cell.hit();

        bullet.setDead(true);
      }
    }
  }
};

//---

function Level () {
  this.value = 1;
}

Level.prototype.draw = function() {
  var str = "Level: ";

  str += String(this.value);

  invaders.drawRightText(invaders.SCREEN_WIDTH - 10, 10, str);
};

Level.prototype.reset = function() {
  this.value = 1;
};

Level.prototype.next = function() {
  this.value += 1;
};

//---

function SpaceInvaders () {
  this.SCREEN_WIDTH  = 850;
  this.SCREEN_HEIGHT = 1000;

  this.player       = null;
  this.level        = null;
  this.score        = null;
  this.alienMgr     = null;
  this.aliens       = [];
  this.mysteryAlien = null;
  this.bases        = [];
  this.paused       = false;
  this.gameOver     = false;

  this.leftKey    = 37;
  this.rightKey   = 39;
  this.fireKey    = 32;
  this.pauseKey   = 80;
  this.newGameKey = 27;
}

SpaceInvaders.prototype.init = function() {
  this.canvas = document.getElementById("canvas");
  this.gc     = this.canvas.getContext("2d");

  //---

  window.addEventListener("keydown", function(e) {
    if      (e.which === invaders.leftKey   ) invaders.shipMoveLeft();
    else if (e.which === invaders.rightKey  ) invaders.shipMoveRight();
    else if (e.which === invaders.fireKey   ) invaders.shipFire();
    else if (e.which === invaders.pauseKey  ) invaders.paused = ! invaders.paused;
    else if (e.which === invaders.newGameKey) invaders.restart();
    else                                      console.log(e.which);

    e.preventDefault();
  }, false);

  window.addEventListener("keyup", function(e) {
    if      (e.which === invaders.leftKey ) invaders.shipMoveStop();
    else if (e.which === invaders.rightKey) invaders.shipMoveStop();

    e.preventDefault();
  }, false);

  //---

  this.player = new Player;

  this.player.init();

  //---

  this.level = new Level();

  this.score = new Score(new Point(invaders.SCREEN_WIDTH/2, 10));

  this.alienMgr = new AlienManager();

  //---

  this.mysteryAlien = new MysteryAlien();

  this.mysteryAlien.init();

  //---

  for (var i = 0; i < 4; ++i)
    this.addBase(new Point(98*(2*i + 1), 840));

  for (var y = 0; y < 5; ++y) {
    for (var x = 0; x < 11; ++x) {
      this.addAlien(x, y, 34*(2*x + 1));
    }
  }
};

SpaceInvaders.prototype.draw = function() {
  this.gc.font = "28px Arial";

  this.gc.fillStyle = "#000000";

  this.gc.fillRect(0, 0, this.canvas.width, this.canvas.height);

  //---

  this.level.draw();

  this.score.draw();

  this.player.draw();

  for (var i = 0; i < this.aliens.length; ++i) {
    var alien = this.aliens[i];

    alien.draw();
  }

  for (var i = 0; i < this.bases.length; ++i) {
    var base = this.bases[i];

    base.draw();
  }

  this.alienMgr.draw();

  this.mysteryAlien.draw();

  if      (this.gameOver)
    invaders.drawCenteredText(invaders.SCREEN_WIDTH/2, invaders.SCREEN_HEIGHT/2, "GAME OVER");
  else if (this.paused)
    invaders.drawCenteredText(invaders.SCREEN_WIDTH/2, invaders.SCREEN_HEIGHT/2, "PAUSED");
};

SpaceInvaders.prototype.update = function() {
  if (this.paused || this.gameOver) return;

  this.player.update();

  this.alienMgr.preUpdate();

  for (var i = 0; i < this.aliens.length; ++i)
    this.aliens[i].update();

  this.alienMgr.postUpdate();

  this.alienMgr.update();

  this.mysteryAlien.update();

  if (this.mysteryAlien.isDead()) {
    if (random() < 0.01) {
      this.mysteryAlien.reset();

      this.mysteryAlien.setDead(false);
    }
  }
};

SpaceInvaders.prototype.addAlien = function(x_ind, y_ind, x) {
  var y = this.alienMgr.getRowY(y_ind);

  var pos = new Point(x, y);

  if      (y_ind === 0) {
    var alien1 = new Alien1(x_ind, y_ind, pos);

    alien1.init();

    this.aliens.push(alien1);
  }
  else if (y_ind === 1 || y_ind === 2) {
    var alien2 = new Alien2(x_ind, y_ind, pos);

    alien2.init();

    this.aliens.push(alien2);
  }
  else if (y_ind === 3 || y_ind === 4) {
    var alien3 = new Alien3(x_ind, y_ind, pos);

    alien3.init();

    this.aliens.push(alien3);
  }
};

SpaceInvaders.prototype.addBase = function(pos) {
  var base = new Base(pos);

  base.init();

  this.bases.push(base);
};

SpaceInvaders.prototype.shipMoveLeft = function() {
  if (this.paused || this.gameOver) return;

  this.player.moveLeft();
};

SpaceInvaders.prototype.shipMoveRight = function() {
  if (this.paused || this.gameOver) return;

  this.player.moveRight();
};

SpaceInvaders.prototype.shipMoveStop = function() {
  if (this.paused || this.gameOver) return;

  this.player.moveStop();
};

SpaceInvaders.prototype.shipFire = function() {
  if (this.paused || this.gameOver) return;

  this.player.fire();
};

SpaceInvaders.prototype.checkAlienHit = function(bullet) {
  for (var i = 0; i < this.aliens.length; ++i) {
    var alien = this.aliens[i];

    alien.checkHit(bullet);
  }

  this.alienMgr.checkHit(bullet);

  this.mysteryAlien.checkHit(bullet);
};

SpaceInvaders.prototype.checkPlayerHit = function(bullet) {
  this.player.checkHit(bullet);
};

SpaceInvaders.prototype.checkBaseHit = function(bullet) {
  for (var i = 0; i < this.bases.length; ++i) {
    var base = this.bases[i];

    base.checkHit(bullet);
  }
};

SpaceInvaders.prototype.addScore = function(score) {
  this.score.add(score);
};

SpaceInvaders.prototype.pause = function() {
  this.paused = ! this.paused;
};

SpaceInvaders.prototype.gameOver = function() {
  this.gameOver = true;
};

SpaceInvaders.prototype.nextLevel = function() {
  this.paused   = false;
  this.gameOver = false;

  for (var i = 0; i < this.aliens.length; ++i) {
    var alien = this.aliens[i];

    alien.reset();
  }

  this.alienMgr.reset();

  this.mysteryAlien.reset();

  this.level.next();
};

SpaceInvaders.prototype.restart = function() {
  if (! this.paused && ! this.gameOver) return;

  this.paused   = false;
  this.gameOver = false;

  this.level.reset();

  this.score.reset();

  this.player.reset();

  for (var i = 0; i < this.aliens.length; ++i) {
    var alien = this.aliens[i];

    alien.reset();
  }

  for (var i = 0; i < this.bases.length; ++i) {
    var base = this.bases[i];

    base.reset();
  }

  this.alienMgr.reset();

  this.mysteryAlien.setDead(true);
};

SpaceInvaders.prototype.loadImage = function(filename) {
  var img = new Image();

  img.src = filename;

  return img;
};

SpaceInvaders.prototype.loadSound = function(filename) {
  return new Audio(filename);
};

SpaceInvaders.prototype.drawImage = function(x, y, image) {
  this.gc.drawImage(image, x, y);
};

SpaceInvaders.prototype.drawLeftText = function(x, y, str) {
  this.gc.fillStyle = "#ffffff";

  this.gc.fillText(str, x, y + 12);
};

SpaceInvaders.prototype.drawCenteredText = function(x, y, str) {
  var w = this.gc.measureText(str).width;

  this.gc.fillStyle = "#ffffff";

  this.gc.fillText(str, x - w/2, y + 12);
};

SpaceInvaders.prototype.drawRightText  = function(x, y, str) {
  var w = this.gc.measureText(str).width;

  this.gc.fillStyle = "#ffffff";

  this.gc.fillText(str, x - w, y + 12);
};

SpaceInvaders.prototype.playSound = function(sound) {
  sound.play();
};
