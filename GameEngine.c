
#include "GameEngine.h"
#include "Nokia5110.h"
#include "Sprites.h"
#include "ADC.h"
#include "Sound.h"
#include "SwitchLed.h"
#include "Random.h"

#define MAX_REG_MISSILES 4
#define MAX_SPEC_MISSILES 2
#define MAX_LASERS 5

unsigned long FrameCount=0; //to animate enemies while they move
unsigned long Distance; // units 0.001 cm
unsigned long ADCdata;  // 12-bit 0 to 4095 sample
unsigned long Score; //game score
unsigned char RegularMissileCount; //number of active RMTyp on screen
unsigned char SpecialMissileCount; //number of active SLTyp on screen
unsigned char SpecialMissileDecrementCheck; //if 1 decrement the SpecialMissileCount until other conditions also satisfied
unsigned char LaserCount; //number of active ELTyp on screen
unsigned char KilledEnemyCount;//number of alive ETyp (enemies on screen)
unsigned char LaserDelay; //creating a delay between fring of lasers


struct GameObject {
  unsigned long x;
  unsigned long y;
  unsigned char life;  // 0=dead, greater than 0 = alive
};         
typedef struct GameObject GTyp;

struct PlayerSprite {
	GTyp GObj;
	const unsigned char *image;
															
	unsigned char explode; 
};
typedef struct PlayerSprite PTyp;

struct EnemySprite {
	GTyp GObj;
	const unsigned char *image[2]; 
	unsigned char explode; 
	unsigned long hitBonus;																	
};
typedef struct EnemySprite ETyp;

struct BunkerSprite {
	GTyp GObj;
	const unsigned char *image[4]; 
																		
};
typedef struct BunkerSprite BTyp;

//Lasers fired by enemy
struct EnemyLaserSprite {
	GTyp GObj;
	const unsigned char *image; 
	unsigned long yspeed;																	
};
typedef struct EnemyLaserSprite ELTyp; 

//Regular missiles fired by the player ship
struct RegMissileSprite {
	GTyp GObj;
	const unsigned char *image; 
	unsigned long yspeed;																	
};
typedef struct RegMissileSprite RMTyp; 

//Special missiles fired by the player ship
struct SpecMissileSprite {
	GTyp GObj1;
	GTyp GObj2;
	const unsigned char *image[2];
	unsigned long hitBonus;
	unsigned long xspeed;	
	unsigned long yspeed;																	
};
typedef struct SpecMissileSprite SMTyp; 

PTyp Player;
RMTyp RegularMissiles[MAX_REG_MISSILES];
SMTyp SpecialMissiles[MAX_SPEC_MISSILES];
BTyp Bunkers[2];
ETyp Enemy[12];
ELTyp Lasers[MAX_LASERS];



// Convert a 12-bit binary ADC sample into a 32-bit unsigned
// fixed-point distance (resolution 0.001 cm).
// Input: sample  12-bit ADC sample
// Output: 32-bit distance (resolution 0.001cm)
unsigned long ConvertToDistance(unsigned long sample){
	return ((750*sample) >> 10) + (429 >> 10); 
}


unsigned long RandomGenerator(unsigned long enemies){
  return ((Random()>>22)%enemies);  
}

void Game_Init(void){ 
	unsigned char i, j, k, l, m;
	Score = 0;
	RegularMissileCount = 0;
  SpecialMissileCount = 0; 
  SpecialMissileDecrementCheck = 0;
  LaserCount = 0; 
  KilledEnemyCount = 0;
	LaserDelay = RandomGenerator(4);
	Player.GObj.x = 32;
	Player.GObj.y = 47;
	Player.image = PlayerShip0;
	Player.explode = 0;
	Player.GObj.life = 1;
	
	for(j=0;j<2;j++){
		Bunkers[j].GObj.x = (83-BUNKERW)*j;
    Bunkers[j].GObj.y = 47 - PLAYERH;
    Bunkers[j].image[0] = Bunker3;
    Bunkers[j].image[1] = Bunker2;
		Bunkers[j].image[2] = Bunker1;
		Bunkers[j].image[3] = Bunker0;
    Bunkers[j].GObj.life = 3;
	}
	
  for(i=0;i<12;i++){
		Enemy[i].GObj.life = 1;
		Enemy[i].explode = 0;
		if(i < 4){
			Enemy[i].GObj.x = 16*i;
			Enemy[i].GObj.y = ENEMY10H;
			Enemy[i].image[0] = SmallEnemy30PointA;
			Enemy[i].image[1] = SmallEnemy30PointB;
			Enemy[i].hitBonus = 30;
		}
		if((i < 8) && (i > 3)){
			Enemy[i].GObj.x = 16*(i-4);
			Enemy[i].GObj.y = 2*ENEMY10H;
			Enemy[i].image[0] = SmallEnemy20PointA;
			Enemy[i].image[1] = SmallEnemy20PointB;
			Enemy[i].hitBonus = 20;
		}
		if(i > 7){
			Enemy[i].GObj.x = 16*(i-8);
			Enemy[i].GObj.y = 3*ENEMY10H;
			Enemy[i].image[0] = SmallEnemy10PointA;
			Enemy[i].image[1] = SmallEnemy10PointB;
			Enemy[i].hitBonus = 10;
		}
   }
	
	 for(k = 0; k < MAX_LASERS; k++){
		 Lasers[k].GObj.life = 0;
	 }
	 for(l = 0; l < MAX_REG_MISSILES; l++){
		 RegularMissiles[l].GObj.life = 0;
	 }
	 for(m = 0; m < MAX_SPEC_MISSILES; m++){
		 SpecialMissiles[m].GObj1.life = 0;
		 SpecialMissiles[m].GObj2.life = 0;
	 }
}

//Fire laser from an enemy 
void EnemyLaserFire(void){unsigned char i, generate;
	if(LaserDelay){
		LaserDelay--;
		return;
	}
	LaserDelay = RandomGenerator(4); 
	for(i = 0; i < 12; i++){
		generate = RandomGenerator(2); //Random number which is either 0 or 1
		if(Enemy[i].GObj.life && (LaserCount < MAX_LASERS) && generate){
			Lasers[LaserCount].GObj.x = Enemy[i].GObj.x + 6;
			Lasers[LaserCount].GObj.y = Enemy[i].GObj.y;
			Lasers[LaserCount].GObj.life = 1;
			Lasers[LaserCount].image = Laser0;
			Lasers[LaserCount].yspeed = 2;
			LaserCount++;
			Sound_InvaderShoot();
			return;
		}
	}

}

void RegMissile_Fire(void){
	if(RegularMissileCount < MAX_REG_MISSILES){
			RegularMissiles[RegularMissileCount].GObj.x = Player.GObj.x + 7; 
			RegularMissiles[RegularMissileCount].GObj.y = 47 - PLAYERH;
			RegularMissiles[RegularMissileCount].GObj.life = 1;
			RegularMissiles[RegularMissileCount].image = Laser0;
			RegularMissiles[RegularMissileCount].yspeed = 2;
			RegularMissileCount++;
	}
}

void SpecMissile_Fire(void){
	if(SpecialMissileCount < MAX_SPEC_MISSILES){
			SpecialMissiles[SpecialMissileCount].GObj1.x = Player.GObj.x + 6; 
			SpecialMissiles[SpecialMissileCount].GObj1.y = 47 - PLAYERH;
			SpecialMissiles[SpecialMissileCount].GObj1.life = 1;
			SpecialMissiles[SpecialMissileCount].GObj2.x = Player.GObj.x + 6; 
			SpecialMissiles[SpecialMissileCount].GObj2.y = 47 - PLAYERH;
			SpecialMissiles[SpecialMissileCount].GObj2.life = 1;
			//Missile0 will rise upwards and also move to the right
			//Missile1 will rise upwards and also move to the left
			SpecialMissiles[SpecialMissileCount].image[0] =Missile0;
			SpecialMissiles[SpecialMissileCount].image[1] =Missile1;
			SpecialMissiles[SpecialMissileCount].hitBonus = 10;
			SpecialMissiles[SpecialMissileCount].xspeed = 2;
			SpecialMissiles[SpecialMissileCount].yspeed = 2;
			SpecialMissileCount++;
	}
}

void CheckEnemyRegMissileCollisions(void){unsigned char i, j;
	for(i = 0; i < 12; i++){		
		if(Enemy[i].GObj.life){
			for(j = 0; j < MAX_REG_MISSILES; j++){
					if((RegularMissiles[j].GObj.life) && 
						!(((RegularMissiles[j].GObj.x+LASERW) < Enemy[i].GObj.x) || (RegularMissiles[j].GObj.x > (Enemy[i].GObj.x + ENEMY10W))) &&
						!((RegularMissiles[j].GObj.y < (Enemy[i].GObj.y - ENEMY10H)) || ((RegularMissiles[j].GObj.y - LASERH) > Enemy[i].GObj.y))){
								
							Score += Enemy[i].hitBonus;
							Enemy[i].GObj.life = 0;
							RegularMissiles[j].GObj.life = 0;
							Enemy[i].explode = 1;
							Success_LedOn(1000); // 1000 Timer2A periods approximately equal 0.9s
							Sound_Killed();
							RegularMissileCount--;
							KilledEnemyCount++;
							break;
					}
			}
		}
	}
}

void CheckEnemySpecMissileCollisions(void){unsigned char i, j;
	for(i = 0; i < 12; i++){		
		if(Enemy[i].GObj.life){
			for(j = 0; j < MAX_SPEC_MISSILES; j++){
					if((SpecialMissiles[j].GObj1.life) && 
						!(((SpecialMissiles[j].GObj1.x+MISSILEW) < Enemy[i].GObj.x) || (SpecialMissiles[j].GObj1.x > (Enemy[i].GObj.x + ENEMY10W))) &&
						!((SpecialMissiles[j].GObj1.y < (Enemy[i].GObj.y - ENEMY10H)) || ((SpecialMissiles[j].GObj1.y - MISSILEH) > Enemy[i].GObj.y))){
								
							Score += Enemy[i].hitBonus + SpecialMissiles[j].hitBonus;
							Enemy[i].GObj.life = 0;
							SpecialMissiles[j].GObj1.life = 0;
							Enemy[i].explode = 1;
							Success_LedOn(1000); // 1000 Timer2A periods approximately equal 0.9s
							Sound_Killed();
							SpecialMissileDecrementCheck = 1;
							KilledEnemyCount++;
							break;
					}
						
					if((SpecialMissiles[j].GObj2.life) && 
						!(((SpecialMissiles[j].GObj2.x+MISSILEW) < Enemy[i].GObj.x) || (SpecialMissiles[j].GObj2.x > (Enemy[i].GObj.x + ENEMY10W))) &&
						!((SpecialMissiles[j].GObj2.y < (Enemy[i].GObj.y - ENEMY10H)) || ((SpecialMissiles[j].GObj2.y - MISSILEH) > Enemy[i].GObj.y))){
								
							Score += Enemy[i].hitBonus + SpecialMissiles[j].hitBonus;
							Enemy[i].GObj.life = 0;
							SpecialMissiles[j].GObj2.life = 0;
							Enemy[i].explode = 1;
							Success_LedOn(1000); // 1000 Timer2A periods approximately equal 0.9s
							Sound_Killed();
							SpecialMissileDecrementCheck = 1;
							KilledEnemyCount++;
							break;
					}
			}
		}
	}
}

void CheckBumperLaserCollision(void){unsigned char i, j;
		for(i = 0; i < 2; i++){		
		if(Bunkers[i].GObj.life){
			for(j = 0; j < MAX_LASERS; j++){
					if((Lasers[j].GObj.life) && 
						!(((Lasers[j].GObj.x+LASERW) < Bunkers[i].GObj.x) || (Lasers[j].GObj.x > (Bunkers[i].GObj.x + BUNKERW))) &&
						!((Lasers[j].GObj.y < (Bunkers[i].GObj.y - BUNKERH)) || ((Lasers[j].GObj.y - LASERH) > Bunkers[i].GObj.y))){
					
							Bunkers[i].GObj.life--;
							Lasers[j].GObj.life = 0;
							Failure_LedOn(1000); // 1000 Timer2A periods approximately equal 0.9s
							LaserCount--;
							if(Bunkers[i].GObj.life == 0){
								Sound_Explosion();
								break;
							}
					}
			}
		}
	}
}

void CheckPlayerLaserCollision(void){ unsigned char j;
		if(Player.GObj.life){
			for(j = 0; j < MAX_LASERS; j++){
					if((Lasers[j].GObj.life) && 
						!(((Lasers[j].GObj.x+LASERW) < Player.GObj.x) || (Lasers[j].GObj.x > (Player.GObj.x + PLAYERW))) &&
						!((Lasers[j].GObj.y < (Player.GObj.y - PLAYERH)) || ((Lasers[j].GObj.y - LASERH) > Player.GObj.y))){
					
							Player.GObj.life = 0;
							Player.explode = 1;
							Lasers[j].GObj.life = 0;
							Failure_LedOn(1000); // 1000 Timer2A periods approximately equal 0.9s
							LaserCount--;
							Sound_Explosion();
							break;
					}
			}
		}
}

void CheckLaserRegMissileCollision(void){unsigned char i, j;
	for(i = 0; i < MAX_LASERS; i++){		
		if(Lasers[i].GObj.life){
			for(j = 0; j < MAX_REG_MISSILES; j++){
					if((RegularMissiles[j].GObj.life) && 
						!(((RegularMissiles[j].GObj.x+LASERW) < Lasers[i].GObj.x) || (RegularMissiles[j].GObj.x > (Lasers[i].GObj.x + LASERW))) &&
						!((RegularMissiles[j].GObj.y < (Lasers[i].GObj.y - LASERH)) || ((RegularMissiles[j].GObj.y - LASERH) > Lasers[i].GObj.y))){
					
							Score += 1;
							Lasers[i].GObj.life = 0;
							RegularMissiles[j].GObj.life = 0;
							RegularMissileCount--;
							LaserCount--;
							Success_LedOn(100); 
							break;
					}
			}
		}
	}
}

void CheckLaserSpecMissileCollision(void){unsigned char i, j;
	for(i = 0; i < MAX_LASERS; i++){		
		if(Lasers[i].GObj.life){
			for(j = 0; j < MAX_SPEC_MISSILES; j++){
					if((SpecialMissiles[j].GObj1.life) && 
						!(((SpecialMissiles[j].GObj1.x+MISSILEW) < Lasers[i].GObj.x) || (SpecialMissiles[j].GObj1.x > (Lasers[i].GObj.x + LASERW))) &&
						!((SpecialMissiles[j].GObj1.y < (Lasers[i].GObj.y - LASERH)) || ((SpecialMissiles[j].GObj1.y - MISSILEH) > Lasers[i].GObj.y))){
					
							Score += 2;
							Lasers[i].GObj.life = 0;
							SpecialMissiles[j].GObj1.life = 0;
							SpecialMissileDecrementCheck = 1;
							LaserCount--;
							Success_LedOn(100); 
							break;
					}
						
						if((SpecialMissiles[j].GObj2.life) && 
						!(((SpecialMissiles[j].GObj2.x+MISSILEW) < Lasers[i].GObj.x) || (SpecialMissiles[j].GObj2.x > (Lasers[i].GObj.x + LASERW))) &&
						!((SpecialMissiles[j].GObj2.y < (Lasers[i].GObj.y - LASERH)) || ((SpecialMissiles[j].GObj2.y - MISSILEH) > Lasers[i].GObj.y))){
					
							Score += 2;
							Lasers[i].GObj.life = 0;
							SpecialMissiles[j].GObj2.life = 0;
							SpecialMissileDecrementCheck = 1;
							LaserCount--;
							Success_LedOn(100);
							break;
					}
			}
		}
	}
}

// Checking Collisions
void Checking_Collisions(void){
	CheckEnemyRegMissileCollisions();
	CheckEnemySpecMissileCollisions();
	CheckBumperLaserCollision();
	CheckPlayerLaserCollision();
	CheckLaserRegMissileCollision();
	CheckLaserSpecMissileCollision();	
}


//Move player horizontally across the screen
void PlayerMove(void){
	Player.GObj.x = (ConvertToDistance(ADC0_In())*22) >> 10; 
}

//Move all living enemies 2 pixels to the right
void EnemyMove(void){ unsigned char i;
  for(i=0;i<12;i++){
    if(Enemy[i].GObj.x < 72){
      Enemy[i].GObj.x += 2; // move to right
    }else{
      Enemy[i].GObj.x = 0; //reached end, start from left most end again
    }
  }
}

void RegularMissileMove(void){unsigned char i;
	for(i = 0; i < MAX_REG_MISSILES; i++){
		if(RegularMissiles[i].GObj.life){
			if(RegularMissiles[i].GObj.y <= LASERH ){
				RegularMissiles[i].GObj.life = 0;
				RegularMissileCount--;	
			}
			else{
				RegularMissiles[i].GObj.y -= RegularMissiles[i].yspeed; 
			}
		}
	}
}

void SpecialMissileMove(void){
	unsigned char i;
	for(i = 0; i < MAX_SPEC_MISSILES; i++){
		if(SpecialMissiles[i].GObj1.life){
			//Move Missile0 image right
			if((SpecialMissiles[i].GObj1.y > LASERH) && (SpecialMissiles[i].GObj1.x < MAX_X-1-MISSILEW)){
				SpecialMissiles[i].GObj1.x += SpecialMissiles[i].xspeed; 
				SpecialMissiles[i].GObj1.y -= SpecialMissiles[i].yspeed; 
			}
			else{
				SpecialMissileDecrementCheck = 1;
				SpecialMissiles[i].GObj1.life = 0;
			}
		}
		
		if(SpecialMissiles[i].GObj2.life){
			//Move Missile1 image left
			if((SpecialMissiles[i].GObj2.y > LASERH) && (SpecialMissiles[i].GObj2.x > MISSILEW)){
				SpecialMissiles[i].GObj2.x -= SpecialMissiles[i].xspeed; 
				SpecialMissiles[i].GObj2.y -= SpecialMissiles[i].yspeed; 
			}
			else{
				SpecialMissileDecrementCheck = 1;
				SpecialMissiles[i].GObj2.life = 0;
			}
	  }
		if((SpecialMissiles[i].GObj1.life == 0) && (SpecialMissiles[i].GObj2.life==0) && SpecialMissileDecrementCheck){
			SpecialMissileCount--;
		}
		 SpecialMissileDecrementCheck = 0;
	}
}

void LaserMove(void){unsigned char i;
	for(i = 0; i < MAX_LASERS; i++){
		if(Lasers[i].GObj.life){
			if(Lasers[i].GObj.y >= (47)){
				Lasers[i].GObj.life = 0;
				LaserCount--;	
			}
			else{
				Lasers[i].GObj.y += Lasers[i].yspeed; 
			}
		}
	}
	EnemyLaserFire(); //create new laser if possible
}

void Move_Objects(void){
	PlayerMove();
	EnemyMove();
	RegularMissileMove();
	SpecialMissileMove();
	LaserMove();
}

void DrawPlayer(void){
	if(Player.GObj.life){
		Nokia5110_PrintBMP(Player.GObj.x, Player.GObj.y, Player.image, 0); // player ship middle bottom
	}
	else if(Player.explode){
		Player.explode = 0;
		Nokia5110_PrintBMP(Player.GObj.x, Player.GObj.y,  BigExplosion0, 0); // player ship middle botto
	}
}

void DrawBunkers(void){unsigned char j;
	for(j=0;j<2;j++){
		Nokia5110_PrintBMP(Bunkers[j].GObj.x, Bunkers[j].GObj.y, Bunkers[j].image[Bunkers[j].GObj.life], 0); 
	}
}

void DrawEnemies(void){unsigned char i;
	 for(i=0;i<12;i++){
		//CheckEnemyRegMissileCollisions(i);
    if(Enemy[i].GObj.life > 0){
     Nokia5110_PrintBMP(Enemy[i].GObj.x, Enemy[i].GObj.y, Enemy[i].image[FrameCount], 0);
		}
		else{
			if(Enemy[i].explode){
					Nokia5110_PrintBMP(Enemy[i].GObj.x, Enemy[i].GObj.y, SmallExplosion0, 0);
				  Enemy[i].explode = 0;
			}
		}
  }
}

void DrawRegMissiles(void){unsigned char i;
	for(i=0;i<MAX_REG_MISSILES;i++){
    if(RegularMissiles[i].GObj.life > 0){
     Nokia5110_PrintBMP(RegularMissiles[i].GObj.x, RegularMissiles[i].GObj.y, RegularMissiles[i].image, 0);
		}
  }
}

void DrawSpecMissiles(void){unsigned char i;
	for(i=0;i<MAX_SPEC_MISSILES;i++){
    if(SpecialMissiles[i].GObj1.life > 0){
     Nokia5110_PrintBMP(SpecialMissiles[i].GObj1.x, SpecialMissiles[i].GObj1.y, SpecialMissiles[i].image[0], 0);
		}
		
		if(SpecialMissiles[i].GObj2.life > 0){
     Nokia5110_PrintBMP(SpecialMissiles[i].GObj2.x, SpecialMissiles[i].GObj2.y, SpecialMissiles[i].image[1], 0);
		}
  }
	
}

void DrawLasers(void){unsigned char i;
	for(i=0;i<MAX_LASERS;i++){
    if(Lasers[i].GObj.life){
     Nokia5110_PrintBMP(Lasers[i].GObj.x, Lasers[i].GObj.y, Lasers[i].image, 0);
		}
  }
}

void Drawing_GameFrame(void){
  Nokia5110_ClearBuffer();
	DrawPlayer();
	DrawBunkers();
  DrawEnemies();
	DrawRegMissiles();
	DrawSpecMissiles();
	DrawLasers();
  Nokia5110_DisplayBuffer();      // draw buffer
  FrameCount = (FrameCount+1)&0x01; // 0,1,0,1,...
}

unsigned long Set_Difficulty(void){
	/*return 2666666 - KilledEnemyCount*166666; //2666666 corresponds to period of SysTick interrupt with 30 Hz frequency
																						//12 (max number of killed enemies) * 166666 approximately equals 2666666* (3/4)
																						//hence period varies from 2666666 to 2666666/4 making frequency vary from 30 Hz to 120Hz
	*/
	//Increased returned period to make game run at a more playable speed
	return 2666666*4 - KilledEnemyCount*666666; //2666666*4 corresponds to period of SysTick interrupt with 7.5 Hz frequency
																						//12 (max number of killed enemies) * 666666 approximately equals (2666666*4)* (3/4)
																						//hence frequency varies from 7.5 Hz to 30 Hz
}

//returns 1 if game over; 0 otherwise
unsigned char Checking_GameOver(void){
	if((KilledEnemyCount == 12) || (Player.GObj.life == 0))
		return 1;
	return 0;
}

//Output the frame for the Game Over State
void GameOver(void){
	Nokia5110_Clear();
  Nokia5110_SetCursor(1, 0);
  Nokia5110_OutString("GAME OVER");
  Nokia5110_SetCursor(1, 1);
	if(KilledEnemyCount == 12){
		Nokia5110_OutString("You Won!");
		Nokia5110_SetCursor(1, 2);
		Nokia5110_OutString("Good job,");
	}
	else{
		Nokia5110_OutString("You Lost!");
		Nokia5110_SetCursor(1, 2);
		Nokia5110_OutString("Nice try,");
	}
  Nokia5110_SetCursor(1, 3);
  Nokia5110_OutString("Earthling!");
	Nokia5110_SetCursor(1, 4);
  Nokia5110_OutString("Score:");
  Nokia5110_SetCursor(7, 4);
  Nokia5110_OutUDec(Score);
}

