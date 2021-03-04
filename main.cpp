////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////  
// Game Name: Dunk It Together
// Date: JAN 2021
// Author: Nandinbold Norovsambuu
// Description: This is a simple platformer game 
/////////////// The creator myself wanted to point out one of the worldwide environmental issues that are demanding in today's world, Air Pollution. 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include <allegro5/allegro5.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include<allegro5/allegro_audio.h>
#include<allegro5/allegro_acodec.h>
#include<allegro5/allegro_image.h>


#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <tuple>


#include "mappy_A5.h"
#include <pthread.h>

#define WIDTH 960
#define HEIGHT 470
#define FPS 50

#define NUM_THREADS 5

using namespace std;

bool keys[] = {false,false,false,false,false};
enum KEYS{LEFT,RIGHT,SPACE,UP,DOWN};

//Finite states
enum carState{CLOSE,FAR};
enum basketballState{NORMAL, LONGDUNK, REG_DUNK, TOO_MANY_ATTEMPT, FAIR_ATTEMPTS, GOOD_ATTEMPTS, PERFECT_ATTEMPTS, POOR_ATTEMPTS, TOO_CLOSE};

int textCenterY = 80 , textX = WIDTH;
int moveSpeed = 12 , moveSpeed1 = 5,moveSpeed2 = 2;
int current_x_off = 0;
int carState = -1;
int hoopState = -1;
int hoopNum = 0;

struct BALLER{
	ALLEGRO_BITMAP *sprite_sheet;
    ALLEGRO_BITMAP *image;
	int top[4];
	int bottom[4];
    // int x1,x2,y1,y2;
	int w_img,h_img;
};


struct Objects{
	ALLEGRO_BITMAP *sprite_sheet;
	ALLEGRO_BITMAP *image;
    int positionX,positionY;
	int x1,x2,y1,y2;
	int w_img,h_img;
	int sprite_count;
	int counter = 0,counter2;
	bool draw = true;
};


void loadObjects(Objects * objects);
void changeCarState(int &state, int newState);
void changeHoopState(int &hoopState, int newState);

int main(int argc, char **argv)
{	
    //al_init
    if(!al_init())
        al_show_native_message_box(NULL,NULL,NULL,"failed to initialize allegro!!",NULL,0);


    //display
    ALLEGRO_DISPLAY *display = NULL;
    ALLEGRO_EVENT_QUEUE * queue = NULL;
    ALLEGRO_TIMER * timer;
	
    display = al_create_display(WIDTH,HEIGHT);

    if(!display)
        al_show_native_message_box(NULL,NULL,"Couldn't open the display",NULL,NULL,0);

	queue = al_create_event_queue();
	timer = al_create_timer(1.0f/FPS);

    // addons
    al_init_font_addon();
    al_init_ttf_addon();
	al_init_primitives_addon();
	al_init_acodec_addon();
    al_init_image_addon();
    al_install_keyboard();
	al_install_audio();
    
	//coordinates
    int x_off = 0, y_off = 0;
	int x_player = 10 , y_player = 240;
    int mapColumns = 0,mapSize= 0, tilesize = 32;
	int counter_baller= 0, counter_stopping =0, counter_dunking = 0;
	int checkPoint = 10;
	int jumpRange = 0;
	int attemptNum = 0, madeNum = 0,randNum = 1;
	int animation_count = 0 , hoopCount = 0;
	int jumpDistance,jumpDistance2;
	int i_rand = rand() % 3;



	//game logic
    bool done = false,render = false;
	bool playerDribbling = false, playerStopping = false, playerDunking = false,longDunk=false,boundary = true;
	bool gamePlay = false,gamePlayFront = true; 
	bool frontItems = true;
	bool intro = true;

	bool dunkAttempt, dunkMade = false;
	bool drawPlayer = true , drawPlayerBack = false;
	bool hoopEffect = false, hoopEffect2 = false;
	bool collided = false,getRandNum = false;
	bool musicPlaying = true;
	//booleans for help menu
	bool help = false ,showDunking = false,showLongDunking = false , endGame = false;



    //game fonts
	ALLEGRO_FONT *font_40_man = al_load_font("fonts/manaspc.ttf",40,0);
    ALLEGRO_FONT *font_30_man = al_load_font("fonts/manaspc.ttf",30,0);
	ALLEGRO_FONT *font_25_man = al_load_font("fonts/manaspc.ttf",25,0);
	ALLEGRO_FONT *font_20_man = al_load_font("fonts/manaspc.ttf",20,0);
	ALLEGRO_FONT * font_100_tarrget = al_load_font("fonts/Tarrget3D.otf",100,0);
	ALLEGRO_FONT * font_80_tarrget = al_load_font("fonts/Tarrget3D.otf",80,0);
	ALLEGRO_FONT * font_40_tarrget = al_load_font("fonts/Tarrget3D.otf",40,0);
	ALLEGRO_FONT * font_20_tarrget = al_load_font("fonts/Tarrget3D.otf",20,0);

	//text colors
    ALLEGRO_COLOR text_col = al_map_rgb(255,191,70);
    ALLEGRO_COLOR text_col_green = al_map_rgb(170,230,136);
	ALLEGRO_COLOR text_col_red = al_map_rgb(255,107,107);
	// ALLEGRO_COLOR text_col_green = al_map_rgb(57,202,243);
	

    //register events
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_register_event_source(queue,al_get_display_event_source(display));
    al_register_event_source(queue,al_get_keyboard_event_source());
    
	//game sound
    al_reserve_samples(1);
    ALLEGRO_SAMPLE *gamesample = al_load_sample("shopTheme.wav");
    ALLEGRO_SAMPLE_INSTANCE * gamesound = al_create_sample_instance(gamesample);
    al_attach_sample_instance_to_mixer(gamesound,al_get_default_mixer());
	al_set_sample_instance_gain(gamesound,0.5);




    //load the map using mappy_A5
	if(MapLoad("map.fmp", 1))
		return -5;
	


	BALLER baller;
	baller.sprite_sheet = al_load_bitmap("bitmaps/baller1.png");
	baller.w_img = al_get_bitmap_width(baller.sprite_sheet);
	baller.h_img = al_get_bitmap_height(baller.sprite_sheet);
	baller.image = al_create_sub_bitmap(baller.sprite_sheet, 0, 0, baller.w_img/21, baller.h_img);
	
	BALLER baller2;
	baller2.sprite_sheet = al_load_bitmap("bitmaps/baller2.png");
	baller2.w_img = al_get_bitmap_width(baller2.sprite_sheet);
	baller2.h_img = al_get_bitmap_height(baller2.sprite_sheet);
	baller2.image = al_create_sub_bitmap(baller2.sprite_sheet, 0, 0, baller2.w_img/7 , baller2.h_img);
	
	BALLER baller3;
	baller3.sprite_sheet = al_load_bitmap("bitmaps/baller3.png");
	baller3.w_img = al_get_bitmap_width(baller3.sprite_sheet);
	baller3.h_img = al_get_bitmap_height(baller3.sprite_sheet);
	baller3.image = al_create_sub_bitmap(baller3.sprite_sheet, 0, 0, baller3.w_img/17 , baller3.h_img);
	


	//main array conaining objects in the game 
	Objects objects[5];
	loadObjects(objects);

	

	Objects hoop;
	hoop.sprite_sheet = al_load_bitmap ("bitmaps/hoop.png");
	hoop.w_img = al_get_bitmap_width(hoop.sprite_sheet);
	hoop.h_img = al_get_bitmap_height(hoop.sprite_sheet);
	hoop.positionX = WIDTH + 200;
	hoop.positionY = 50;
 	hoop.sprite_count = 14;


	Objects hoop2;
	hoop2.sprite_sheet = al_load_bitmap ("bitmaps/hoop.png");
	hoop2.w_img = al_get_bitmap_width(hoop2.sprite_sheet);
	hoop2.h_img = al_get_bitmap_height(hoop2.sprite_sheet);
	hoop2.positionX = WIDTH + 600;
	hoop2.positionY = -20;
	hoop2.counter = 0;
 	hoop2.sprite_count = 14;


	Objects hoop_green;
	hoop_green.sprite_sheet = al_load_bitmap("bitmaps/hoop1.png");
	hoop_green.image = al_create_sub_bitmap(hoop_green.sprite_sheet,0,0,hoop.w_img, hoop.h_img/hoop.sprite_count);


	al_start_timer(timer);


	while(!done)
	{
		ALLEGRO_EVENT ev;
		ALLEGRO_KEYBOARD_STATE keyState;

		al_wait_for_event(queue, &ev);
		al_get_keyboard_state (&keyState);



		if(ev.type == ALLEGRO_EVENT_KEY_DOWN)
		{
			switch(ev.keyboard.keycode)
			{
				case ALLEGRO_KEY_ESCAPE:
					done = true;
					break;
				case ALLEGRO_KEY_LEFT:
					keys[LEFT] = true;
					break;


				case ALLEGRO_KEY_RIGHT:
					if(!playerDunking){
						keys[RIGHT] = true;
						playerDribbling = true;
						playerStopping = false;
						intro = false;
					}

					break;

				case ALLEGRO_KEY_UP:
					if(!playerDunking){
						keys[UP] = true;
						playerDribbling = true;
						playerStopping = false;

						moveSpeed1 -= 4;
						moveSpeed -=9;
					}
					
					break;
				case ALLEGRO_KEY_DOWN:
					if(!playerDunking){
						keys[DOWN] = true;
						playerDribbling = true;
						playerStopping = false;
						moveSpeed1 -= 4;
						moveSpeed -=9;
					}
					break;
				case ALLEGRO_KEY_SPACE:

					if(al_key_down(&keyState,ALLEGRO_KEY_LSHIFT)) longDunk = true;
					keys[SPACE] = true;

					playerDunking = true;
					playerDribbling = false;
					playerStopping = false;
					counter_dunking = 0;

					moveSpeed1 -= 2;
					moveSpeed -= 4;

					break;
				case ALLEGRO_KEY_ENTER:
					if(collided){
						

						//reset finite States
						changeHoopState(hoopState,NORMAL);

						//reset the coordinates
						x_player = 10;
						y_player = 240;


						gamePlay = true;
						gamePlayFront = true;
						collided = false;
						drawPlayer = true;


						
						objects[0].positionX = WIDTH + 200;
						objects[1].positionX = -300;
						objects[2].positionX = WIDTH + 1000;
						
						//reset the hoop positions when restarts
						if(checkPoint == 10){
							hoop.positionX = WIDTH;
						}else if(checkPoint ==12){
							hoop2.positionX = WIDTH;
						}else if (checkPoint == 14){
							hoop2.positionX = WIDTH;
						}

						if(checkPoint == 15){
							//reset hoop positions
							hoop2.positionX  = WIDTH + 600;
							hoop.positionX  = WIDTH;
							hoopEffect2 =false;
							objects[4].counter = 0;
							hoopCount = 0;
						}

					}

					break;
			}

			if(al_key_down(&keyState , ALLEGRO_KEY_LCTRL)){

					//help Screen
					if(al_key_down(&keyState,ALLEGRO_KEY_H))
						help = true;    
					
					//Music Toggle
					if(al_key_down(&keyState,ALLEGRO_KEY_M)){
						if(musicPlaying) musicPlaying  = false;
						else musicPlaying = true;
					}

			}




		}
		else if(ev.type == ALLEGRO_EVENT_KEY_UP)
		{
			switch(ev.keyboard.keycode)
			{
				case ALLEGRO_KEY_ESCAPE:
					done = true;
					break;
				case ALLEGRO_KEY_LEFT:
					keys[LEFT] = false;
					break;
				case ALLEGRO_KEY_RIGHT:
					keys[RIGHT] = false;
					 
					if(!playerDunking){
						playerDribbling = false;
						playerStopping = true;
					}

					break;
				case ALLEGRO_KEY_UP:
					keys[UP] = false;
					if(!playerDunking){
						playerDribbling = false;
						playerStopping = true;
					}
					moveSpeed1 += 4;
					moveSpeed += 9;

					break;
				case ALLEGRO_KEY_DOWN:
					keys[DOWN] = false;
					if(!playerDunking){
						playerDribbling = false;
						playerStopping = true;
					}
					moveSpeed1 += 4;
					moveSpeed += 9;

					break;

				case ALLEGRO_KEY_SPACE:
					keys[SPACE] = false;
					longDunk = false;
					dunkAttempt = true;
					counter_dunking = 16;
					attemptNum++;


					moveSpeed1 += 2;
					moveSpeed  += 4;
					break;

				case ALLEGRO_KEY_H:
					if(x_off < 50)
						intro = true;
					else
					{
						gamePlay = true;
						gamePlayFront = true;
					}
					
					
					help = false;
					break;


			}
		}
		else if(ev.type == ALLEGRO_EVENT_TIMER)
		{	


			//toggle the music on/off with musicPlaying boolean
			al_set_sample_instance_playmode(gamesound, ALLEGRO_PLAYMODE_LOOP);		
			al_set_sample_instance_playing(gamesound,musicPlaying);
			
			y_player -= keys[UP] * 1;
			y_player += keys[DOWN] * 1 ;

			if(x_off < 0)
				x_off = 0;
			if(y_off < 0)
				y_off = 0;
			if(x_off > (mapwidth*mapblockwidth - WIDTH)){
				x_off = 9028;

			}
			if(y_off > mapheight*mapblockheight - HEIGHT)
				y_off = mapheight*mapblockheight - HEIGHT;

			//keep the player within the y boundaries
			if(y_player > 240)
				y_player = 240;
			if(!playerDunking){
				//if the player moves up make objects seem in front of the player
				if(y_player < 160 && checkPoint >=11){
					drawPlayer = false;
					drawPlayerBack = true;
					if(y_player < 120)
					y_player = 120;
				}else{
					if(!collided){
						drawPlayer = true;
						drawPlayerBack =false;
					}
				}
			}

			//x boundaries
			if( !playerDunking && x_player > 800 && boundary){
				x_player = 800;
			}

			if(x_player < 0)
				x_player = 0;

			


	
			//checkpoint examination
			if(x_off > 50 && !collided && !endGame && !help){
				gamePlay = true;

			}

			render = true;
		}




		if(render && al_is_event_queue_empty(queue))
		{
			render = false;

			MapDrawBG(x_off,y_off, 0, 0, WIDTH, HEIGHT);
			MapDrawFG(x_off,y_off,0,0,WIDTH,HEIGHT,2);

			



			//// collision detection for dunking
			// if(jumpDistance< 340 && jumpDistance > 322 && baller.top[3]> hoop.y1 && baller.top[1] < hoop.y2 && baller.top[2] > hoop.x1 && baller.top[0] < hoop.x2  ){
			// 	//if player dunked on the current hoop already, don't increment the score
			// 	if(!hoopEffect) dunkMade = true;

			// } 
			
			
			

			if(playerDribbling){


				if(counter_baller >= 20) counter_baller =0;
				baller.image = al_create_sub_bitmap(baller.sprite_sheet, baller.w_img * counter_baller / 21, 0, baller.w_img/21 , baller.h_img);	
				counter_baller++;

				if(y_player < 140){
					y_player = 140;
				}else if (y_player > 245) {
					y_player = 240;
				}	
				
				x_off += moveSpeed1;
				x_player +=1;


				al_rest(0.008);
			}




			if(playerStopping){
				if (counter_stopping >= 8){
					// playerStopping = false;
					counter_stopping = 7;
				}
				
				// if(x_player = )
				baller.image = al_create_sub_bitmap(baller2.sprite_sheet, baller2.w_img * counter_stopping / 8, 0, baller2.w_img/8 , baller2.h_img/2);
				
				counter_stopping ++;

				if(y_player < 120){
					y_player = 120;
				}else if (y_player > 245) {
					y_player = 240;
				}




				if(animation_count < jumpRange){
					x_off += 2;
					x_player -= 2;

					

					animation_count += 1;
					if(x_player < 20)
						jumpRange = 0;

				}else{
					animation_count = 0;
					jumpRange = 0;
				}

				x_off += keys[LEFT] * (moveSpeed1 -2);
				x_player -= keys[LEFT] * (moveSpeed1 -2);
					

				
			}



			if(playerDunking){
				int dunkingSpeed = 20;
				int y_before_jumping;
				int x_before_jumping;
				//get the initial y position
				if(counter_dunking == 0){
					y_before_jumping = y_player;
					x_before_jumping = x_player;
					//get the distance between player and hoop to get the accuracy
					jumpDistance = hoop.positionX - x_player;
					jumpDistance2 = hoop2.positionX - x_player;

				}

				if(counter_dunking >= 11){
					if(counter_dunking == 16){
						playerDunking = false;
						playerStopping = true;
						y_player = y_player - 20 * counter_dunking + 4 * pow(counter_dunking,2);
						y_player = y_before_jumping;
						
					}
				}
				else{
					x_player += dunkingSpeed + longDunk * 20;
					jumpRange +=dunkingSpeed;
					y_player = y_player - 30 * counter_dunking + 4 * pow(counter_dunking,2);		
				}
				
				
			
				baller.image = al_create_sub_bitmap (baller3.sprite_sheet,baller3.w_img * counter_dunking / 17 ,0, baller3.w_img/17,baller3.h_img);
				
				counter_dunking++;


				al_rest(0.07);


			}

		
			if(intro){
				al_draw_text(font_25_man,text_col_green , 50,50,0,"LCTRL + H for help");
				al_draw_text(font_25_man,text_col_green , 50,80,0,"LCTRL + M to switch music on/off");
				al_draw_multiline_text(font_100_tarrget,text_col,540,textCenterY + 120,600,70,ALLEGRO_ALIGN_CENTER, "dunk it together");
				al_draw_text(font_30_man,text_col,560,textCenterY + 300,ALLEGRO_ALIGN_CENTER,"please hold on a RIGHT key to start");
			}






	////////////////////////////////////////////  	GAME PLAY 	////////////////////////////////////////////
	////////////////////////////////////  Gameplay is divided into checkpoints 	(10 - 16)	  //////////////////
	////////////////////////////////////  Each checkpoint is unique 			 			  //////////////////
	////////////////////////////////////  The game restarts from the most recent checkpoint	  //////////////////

			if (gamePlay){

				//finite States for dodging the car closely,or dunking over the car are BELOW
				if(carState == CLOSE){
					//show feedback only when user is dodging with either UP or DOWN 
					if(keys[UP] || keys[DOWN]){
						if(i_rand == 0){
							al_draw_text(font_25_man , text_col_red, x_player, y_player - 40, 0, "NICE DODGE");
						}
						else if (i_rand == 1) {
							al_draw_text(font_25_man , text_col_red, x_player, y_player - 40, 0, "GOOD DODGE");
						}
						else if (i_rand == 2) {
							al_draw_text(font_25_man , text_col_red, x_player, y_player - 40, 0, "AMAZING");
						}
						else if (i_rand == 3) {
							al_draw_text(font_25_man , text_col_red, x_player, y_player - 40, 0, "EXCELENT SKILLS");
						}
						else{
							al_draw_text(font_25_man , text_col_red, x_player, y_player - 40, 0, "CLOSE ONE");
						}
					}
					else {
						changeCarState(carState,FAR);
					}
						
				}
				//NORMAL
				else {

					//collision detection for dodging cars
					if(!playerDunking && baller.bottom[3] > objects[0].y1 - 40 && baller.bottom[1] < objects[0].y2 && baller.bottom[2] > objects[0].x1 -25 && baller.bottom[0] < objects[0].x2 ){
						changeCarState(carState,CLOSE);
						//refresh the random number
						i_rand = rand() % 5;
					}
					if(!playerDunking && baller.bottom[3] > objects[2].y1 && baller.bottom[1] < objects[2].y2 + 40 && baller.bottom[2] > objects[2].x1 -25 && baller.bottom[0] < objects[2].x2 ){
						changeCarState(carState,CLOSE);
						//refresh the random number
						i_rand = rand() % 5;
					}

				}




				//finite states for dunks mostly, and for moving the hoop if necessary
				if(hoopState == REG_DUNK){
					//perfect range for perfect dunking 
					if ( jumpDistance <= 330 && jumpDistance >=325){
						al_draw_text(font_30_man , text_col_red, x_player, y_player - 80, 0, "PERFECT DUNK !!!");

					}else{
						//print out feedback randomly
						if(i_rand == 0)
							al_draw_text(font_30_man , text_col_red, x_player, y_player - 80, 0, "GOOD DUNK");
						else if (i_rand == 1) {
							al_draw_text(font_30_man , text_col_red,  x_player, y_player - 80, 0, "FAIR DUNK");
						}else{
							al_draw_text(font_30_man , text_col_red,  x_player, y_player - 80, 0, "LOOK AT YA");

						}
					}

				}
				else if (hoopState == LONGDUNK) {
					al_draw_text(font_30_man , text_col_red, x_player, y_player - 80, 0, "OMG, YOU ARE KILLING IT !!!");	
				}
				else if (hoopState == TOO_MANY_ATTEMPT) {
					if(i_rand == 0)
						al_draw_textf(font_30_man , text_col_red, 200, 200, 0, "%d/%d - WE CAN CHANGE THE 0",madeNum,attemptNum);
					else
						al_draw_text(font_30_man , text_col_red, 200, 200, 0, "COME ON, I KNOW YOU CAN DO THIS");
				}
				else if (hoopState == FAIR_ATTEMPTS) {
					al_draw_text(font_30_man , text_col_red, 200, 200, 0, "I KNOW YOU CAN DO BETTER");

				}
				else if (hoopState == GOOD_ATTEMPTS) {
					if(i_rand == 0){
						al_draw_text(font_30_man , text_col_red, 200, 200, 0, "YOU ARE DOING GOOD");

					}else if (i_rand == 1) {
						al_draw_text(font_30_man , text_col_red, 200, 200, 0, "KEEP PUSHING");

					}else{
						al_draw_text(font_30_man , text_col_red, 200, 200, 0, "YOU GOT THIS");

					}

				}
				else if (hoopState == PERFECT_ATTEMPTS) {
						al_draw_textf(font_30_man , text_col_red, 200, 200, 0, "%d OUT OF %d YOU ARE ON POINT!!!", madeNum,attemptNum);

				}
				//move the hoop itself if the player is already past the distance for succesful dunks
				else if (hoopState == TOO_CLOSE) {
						if(hoopNum == 1){
							hoop2.positionX -=moveSpeed1;
						}
						if(hoopNum == 2){
							hoop.positionX -= moveSpeed1;
						}					
				}
				//NORMAL
				else{
					//refresh the random number for getting a feedback on dunks
					i_rand = rand() % 3;
					// get the ratio of number of succesful dunks over number of dunk attempts
					double percentage = 1.0 * madeNum/attemptNum;

					//give feedback to user depending on their success percentage
					if(percentage >= 0.5){
						if (percentage > 0.9)
							changeHoopState(hoopState,PERFECT_ATTEMPTS);
						else
							changeHoopState(hoopState, GOOD_ATTEMPTS);
					}else if(  percentage >= 0.3 && percentage < 0.5){
						changeHoopState(hoopState,FAIR_ATTEMPTS);
					}else if(percentage >= 0.1 && percentage < 0.3){
						changeHoopState(hoopState, POOR_ATTEMPTS);
					}else{
						//if the user doesn't make dunk after more than 5 attempts
						if(attemptNum >= 5) changeHoopState(hoopState,TOO_MANY_ATTEMPT);
					}

				}

				
				//if the hoop positions are less than x = 320, too late
				if(!hoopEffect && hoop.positionX < 320){
					changeHoopState(hoopState,TOO_CLOSE);
					hoopNum = 2;
				}
				if(!hoopEffect && hoop2.positionX < 320){
					changeHoopState(hoopState,TOO_CLOSE);
					hoopNum = 1;
				}
				

				

				textX -= playerDribbling * moveSpeed1;

				
				if(jumpRange)
					textX -= playerStopping * 2;


				if(hoop.counter == 13)
						hoop.counter = 5;

				if(hoop2.counter == 13)
						hoop2.counter = 5;


				if(!longDunk){
						//collision detection for dunking 
					if(jumpDistance< 344 && jumpDistance > 322 && baller.top[3]> hoop.y1 && baller.top[1] < hoop.y2 && baller.top[2] > hoop.x1 && baller.top[0] < hoop.x2  ){
						//if player dunked on the current hoop already, don't increment the score
						if(!hoopEffect) {
							dunkMade = true;
							changeHoopState(hoopState,REG_DUNK);
						}
					}

					if(jumpDistance2 < 344 && jumpDistance2 > 322 && baller.top[3]> hoop2.y1 && baller.top[1] < hoop2.y2 && baller.top[2] > hoop2.x1 && baller.top[0] < hoop2.x2  ){
						//if player dunked on the current hoop already, don't increment the score
						if(!hoopEffect) {
							dunkMade = true;
							changeHoopState(hoopState,REG_DUNK);
						}

					}

				}else{
					//collision detection for long ranged dunking 
					if(baller.top[3]> hoop.y1 && baller.top[1] < hoop.y2 && baller.top[2] > hoop.x1 && baller.top[0] < hoop.x2  ){
						//if player dunked on the current hoop already, don't increment the score
						if(!hoopEffect) {
							dunkMade = true;
							changeHoopState(hoopState,LONGDUNK);
						}

					}

					if(baller.top[3]> hoop2.y1 && baller.top[1] < hoop2.y2 && baller.top[2] > hoop2.x1 && baller.top[0] < hoop2.x2  ){
						//if player dunked on the current hoop already, don't increment the score
						if(!hoopEffect) {
							dunkMade = true;
							changeHoopState(hoopState,LONGDUNK);
						}

					}

				}
				

				if(checkPoint >= 11){


					//car in the back
					if(objects[1].draw){
							if(objects[1].counter == objects[1].sprite_count)
							objects[1].counter = 0;
						al_draw_bitmap_region(objects[1].sprite_sheet,objects[1].w_img * objects[1].counter / objects[1].sprite_count , 0, objects[1].w_img/objects[1].sprite_count,objects[1].h_img,objects[1].positionX,objects[1].positionY,0);
						objects[1].counter += 1;
						objects[1].positionX += moveSpeed1;
					}
					

					if(objects[1].positionX > WIDTH + 10){
						objects[1].positionX =  (-rand()%200) - 100 ;

					}

					//draw player before the car in middle when they move up
					if(drawPlayerBack)
						al_draw_bitmap(baller.image, x_player,y_player,0);


					//car in the middle	
					if(objects[2].draw){
								
						if(objects[2].counter == objects[2].sprite_count)
						objects[2].counter = 0;
						al_draw_bitmap_region(objects[2].sprite_sheet,objects[2].w_img * objects[2].counter / objects[2].sprite_count , 0, objects[2].w_img/objects[2].sprite_count,objects[2].h_img,objects[2].positionX,objects[2].positionY,0);


						objects[2].counter += 1;
						objects[2].positionX -= moveSpeed;

						if(objects[2].positionX < - 1000){
							objects[2].positionX = rand() % 1800 + WIDTH;
						}
					}
					





				}


				//checkpoint 1.0
				if(checkPoint == 10){

					al_draw_text(font_30_man,text_col,textX + 100,textCenterY + 200,ALLEGRO_ALIGN_CENTER,"Hold on SPACE to dunk and LSHIFT + SPACE for longer dunks");
					al_draw_text(font_30_man,text_col,textX + 1000,textCenterY + 200,ALLEGRO_ALIGN_CENTER,"Use LEFT to slide faster");


					//hoop animation below
					al_draw_bitmap_region(hoop.sprite_sheet,0,hoop.h_img * hoop.counter / 14 , hoop.w_img,hoop.h_img/14,hoop.positionX, hoop.positionY ,0);
				
					if(dunkMade){
						hoopEffect = true;
						madeNum++;
						dunkMade = false;


					}

					if(hoopEffect){
						hoop.counter += 1;
						hoop.positionX -=  moveSpeed2;
						hoop.positionX -=  playerDribbling * (moveSpeed1 - moveSpeed2);
						if(hoop.positionX < -150) {
							current_x_off = x_off;
							hoopEffect = false;
							checkPoint ++;
						}
					}else{
						hoop.positionX -= playerDribbling * moveSpeed1;
						if(hoop.positionX < - 100){
							collided = true;
							hoop.positionX = WIDTH;
						} 

					}
					

					

				}


				//checkPoint - 1.1
				if(checkPoint == 11){

					if(textX < -50)
						textX = WIDTH;

					al_draw_multiline_text(font_40_man,text_col,textX - 200,textCenterY,400,50,0,"please use arrow keys UP,DOWN to slow down and dodge objects");
					

					if(x_off > current_x_off + 300 ){


						//if the car in the middle passes, advance to the next checkpoint
						if(objects[2].positionX < - 200){
							checkPoint ++;
						}

					}

									
				}

				//checkpoint 1.2
				if(checkPoint == 12 ){


					//hoop becomes green - avialable for dunk - when only the car passed the screen
					if(!hoopEffect && objects[2].positionX < -50){
						al_draw_bitmap(hoop_green.image,hoop2.positionX, hoop2.positionY ,0);
						
					}else{
						al_draw_bitmap_region(hoop2.sprite_sheet,0,hoop2.h_img * hoop2.counter / 14 , hoop2.w_img,hoop2.h_img/14,hoop2.positionX, hoop2.positionY ,0);
					}
						
					al_draw_text(font_30_man , text_col, 50 ,120, 0, "Dunk when the rim is green");

					//hoop animation below
					if(dunkMade ){
						//advance only when the rim is green
						if(objects[2].positionX < -50){
							hoopEffect = true;
							madeNum++;
							dunkMade = false;
						}else {
							dunkMade = false;
						}
					}

					if(hoopEffect){
						hoop2.counter += 1;
						hoop2.positionX -=  moveSpeed1;
						hoop2.positionX -=  playerDribbling * (moveSpeed1 - moveSpeed2);
						if(hoop2.positionX < -150) {
							// current_x_off = x_off;
							hoopEffect = false;
							checkPoint++;

							//reset hoop positions
							hoop2.positionX  = WIDTH +200;
							hoop.positionX  = WIDTH +700;
						}

					}else{
						hoop2.positionX -= playerDribbling * moveSpeed1;
						if(hoop2.positionX < -100){
							collided = true;
							getRandNum = true;
						}
						

					}



				}



				//checkpoint 1.3
				if(checkPoint == 13){



					if(textX < -50)
						textX = WIDTH;
					// al_draw_text(font_30_man,text_col_green,textX, 250 ,0,"Dunk on either of the hoops to advance");
					al_draw_text(font_30_man , text_col, 50 ,120, 0, "Dunk on either of the hoops to advance");

	

					//hoop animation below

					al_draw_bitmap_region(hoop.sprite_sheet,0,hoop2.h_img * hoop2.counter / 14 , hoop2.w_img,hoop2.h_img/14,hoop2.positionX, hoop2.positionY ,0);
					
			
					//hoop animation below

					al_draw_bitmap_region(hoop.sprite_sheet,0,hoop.h_img * hoop.counter / 14 , hoop.w_img,hoop.h_img/14,hoop.positionX, hoop.positionY ,0);

					

					if(dunkMade){
						hoopEffect = true;
						madeNum++;
						dunkMade = false;

					}

					if(hoopEffect){


						hoop2.counter += 1;
						hoop2.positionX -=  moveSpeed2;
						hoop2.positionX -=  playerDribbling * (moveSpeed1 - moveSpeed2);


						hoop.counter += 1;
						hoop.positionX -=  moveSpeed2;
						hoop.positionX -=  playerDribbling * (moveSpeed1 - moveSpeed2);



						if(hoop.positionX < -150) {
							// current_x_off = x_off;
							hoopEffect = false;
							checkPoint ++;



							//reset hoop positions
							hoop2.positionX  = WIDTH +200;
							hoop.positionX  = WIDTH +700;


						}
					}
					else{

						hoop2.positionX -= (moveSpeed1 - 2);
						hoop.positionX -= (moveSpeed1 - 2);
						//if the player doesn't dunk on either of the hoops, game over
						if(hoop.positionX < -100) {
							collided = true;
							getRandNum = true;
							hoop2.positionX  = WIDTH +200;
							hoop.positionX  = WIDTH +700;
						
						}

					}


				}



				//checkpoint 1.4
				if(checkPoint == 14){
					//smoke coordinates
					objects[3].positionX = hoop2.positionX - 600;
					objects[3].positionY = hoop2.positionY + 280;

					al_draw_bitmap(objects[3].sprite_sheet,hoop2.positionX - 400,hoop2.positionY + 280,0 );

					//collision detection for smoke
					if(!playerDunking && !hoopEffect && baller.bottom[3] > objects[3].y1 && baller.bottom[1] < objects[3].y2 && baller.bottom[2] > objects[3].x1 && baller.bottom[0] < objects[3].x2 ){
						//reset the position
						collided = true;
						getRandNum = true;
						hoop2.positionX  = WIDTH +200;
						objects[3].positionX = hoop2.positionX - 600;

					}
						

					//hoop animation below
					al_draw_bitmap_region(hoop.sprite_sheet,0,hoop2.h_img * hoop2.counter / 14 , hoop2.w_img,hoop2.h_img/14,hoop2.positionX, hoop2.positionY ,0);

					if(dunkMade){
						hoopEffect = true;
						madeNum++;
						dunkMade = false;

					}

					if(hoopEffect){
						hoop2.counter += 1;
						hoop2.positionX -=  moveSpeed2;
						hoop2.positionX -=  playerDribbling * (moveSpeed1 - moveSpeed2);
						if(hoop2.positionX < -150) {
							// current_x_off = x_off;
							hoopEffect = false;
							checkPoint++;

							//reset hoop positions
							hoop2.positionX  = WIDTH +600;
							hoop.positionX  = WIDTH;
						}

					}else{
						hoop2.positionX -= moveSpeed2;

						if(hoop2.positionX < - 100) {
							collided = true;
							getRandNum = true;						
						}

					}

				}


				//checkpoint 1.5
				if(checkPoint == 15){

					//smoke
					objects[3].positionX = hoop2.positionX - 600;
					objects[3].positionY = hoop2.positionY + 280;
					al_draw_bitmap(objects[3].sprite_sheet,hoop2.positionX - 400,hoop2.positionY + 280,0 );


					//collision detection for smoke
					if(!playerDunking && !hoopEffect && baller.bottom[3] > objects[3].y1 && baller.bottom[1] < objects[3].y2 && baller.bottom[2] > objects[3].x1 && baller.bottom[0] < objects[3].x2 ){
						collided = true;
						getRandNum = true;
						objects[3].positionX = hoop2.positionX - 600;

					}

					//trump sprite
					// bool trump_animation = false;
					al_draw_bitmap_region(objects[4].sprite_sheet,objects[4].w_img * objects[4].counter / objects[4].sprite_count,0 , objects[4].w_img / objects[4].sprite_count, objects[4].h_img ,hoop.positionX - 50 , hoop.positionY + 200, 0);
					objects[4].x1 = hoop.positionX - 20;
					objects[4].x2 = hoop.positionX + 40;
					objects[4].y1 = hoop.positionY;   
					objects[4].y2 = hoop.positionY + 200;

					int x1_trump = objects[4].x2 , x2_trump = x1_trump + 100, y1_trump = objects[4].y2, y2_trump = y1_trump + 180;

					
					//collision detection for trump animation
					if(playerDunking && baller.top[3] > objects[4].y1 && baller.top[1] < objects[4].y2 && baller.top[2] > objects[4].x1 && baller.top[0] < objects[4].x2 )
							objects[4].counter++;

					if(objects[4].counter == objects[4].sprite_count -1)
							objects[4].counter = 4;


					//if the player doesn't make the dunk and collides with trump
					if(!hoopEffect && playerDunking && baller.bottom[3] > y1_trump && baller.bottom[1] < y2_trump && baller.bottom[2] > x1_trump && baller.bottom[0] < x2_trump ){
						collided = true;
						getRandNum = true;

					}


					//hoop animation below
					al_draw_bitmap_region(hoop2.sprite_sheet,0,hoop2.h_img * hoop2.counter / 14 , hoop2.w_img,hoop2.h_img/14,hoop2.positionX, hoop2.positionY ,0);
					al_draw_bitmap_region(hoop.sprite_sheet,0,hoop.h_img * hoop.counter / 14 , hoop.w_img,hoop.h_img/14,hoop.positionX, hoop.positionY ,0);

					


					if(dunkMade){
						hoopEffect = true;
						madeNum++;
						hoopCount ++;
						dunkMade = false;

						

					}


					// if(hoopEffect ){
					if(hoopCount == 1){
						hoop.counter += 1;

						hoop.positionX -=  moveSpeed1;
						hoop2.positionX -=  moveSpeed2;

						hoop2.positionX -=  playerDribbling * (moveSpeed1 - moveSpeed2);

						if(hoop.positionX  <0 ) hoopEffect =false;

					}else if(hoopCount == 2){

						hoop.counter += 1;
						hoop2.counter += 1;
						hoop.positionX -=  moveSpeed1;
						hoop2.positionX -=  moveSpeed1;

						//end of the game - checkpoint 1.6
						if(hoop2.positionX < -150 ) {
							current_x_off = x_off;
							checkPoint++;
						}

					}else{
						hoop2.positionX -= playerDribbling * moveSpeed2;
						hoop.positionX -= playerDribbling * moveSpeed2;

						if(hoop.positionX  < -100 ) {
							collided = true;
							getRandNum = true;
						}	

					}




				}



				
				if(checkPoint == 16 ){
					if(objects[1].positionX > WIDTH ) objects[1].draw =false; 
					if(objects[2].positionX < - 100) objects[2].draw = false;
					if(objects[0].positionX < -100 ) objects[0].draw = false;

					//wait until all the car passes the screen then end the game
					if(!objects[1].draw && !objects[0].draw && !objects[2].draw ){
						textX = WIDTH ;
						endGame = true;
					} 

					

				}	


				//coordinates for collision detection
				hoop.x1 = hoop.positionX + 15;
				hoop.x2 = hoop.positionX  + 35;
				hoop.y1 = hoop.positionY + 70;
				hoop.y2 =  hoop.positionY + 95;

				hoop2.x1 = hoop2.positionX + 15;
				hoop2.x2 = hoop2.positionX  + 35;
				hoop2.y1 = hoop2.positionY + 70;
				hoop2.y2 =  hoop2.positionY + 95;


				objects[2].x1 = objects[2].positionX + 50;
				objects[2].y1 = objects[2].positionY + 200 ;
				objects[2].x2 = objects[2].positionX + objects[2].w_img / (2 * objects[2].sprite_count);
				objects[2].y2 =	objects[2].positionY + 240;

				objects[3].x1 = objects[3].positionX + 200;
				objects[3].y1 = objects[3].positionY + 50 ;
				objects[3].x2 = objects[3].positionX + objects[3].w_img + 200;
				objects[3].y2 =	objects[3].positionY + 120;
		


				//collision detection for car in the middle
				if(!playerDunking && !hoopEffect && baller.bottom[3] > objects[2].y1 && baller.bottom[1] < objects[2].y2 && baller.bottom[2] > objects[2].x1 && baller.bottom[0] < objects[2].x2 ){
					collided = true;
					getRandNum = true;
				}
					

				//if the player is near the first hoop get y difference with that hoop, else take it with the other hoop
				if(y_player > 205){
					int y_difference = y_player - 190 - hoop.positionY;
					al_draw_textf(font_20_man , text_col_green, 50 ,80, 0, "Alignment with hoop%d (true when 0): %d" ,2, y_difference);
				}else{
					int y_difference = y_player - 185 - hoop2.positionY;
					al_draw_textf(font_20_man , text_col_green, 50 ,80, 0, "Alignment with hoop%d (true when 0): %d" ,1, y_difference);

				}
				 
				al_draw_textf(font_20_man , text_col_green, 50 ,30, 0, "Dunk attempts: %d/%d " , attemptNum,madeNum);
				al_draw_textf(font_20_man , text_col_green, 50 ,55, 0, "Distance traveled: %d" , x_off);


				//testing
				// al_draw_textf(font_20_man , text_col_green, 50 ,120, 0, "x_hoop: %d" , hoop.positionX);
				// al_draw_text(font_30_man , text_col_green, x_player, y_player - 80, ALLEGRO_ALIGN_LEFT, "GOOD DUNK");

				al_draw_textf(font_20_man , text_col_green, WIDTH - 200 ,20, 0, "checkpoint:%d" , checkPoint);



			}


			
			if(drawPlayer)
				al_draw_bitmap (baller.image, x_player,y_player,0);

			//coordinates for detecting collision for the dunk
			baller.top[0] = x_player + al_get_bitmap_width(baller.image)/2 + 10;	//x1
			baller.top[1] = y_player + 10;											//y1
			baller.top[2] = x_player + al_get_bitmap_width(baller.image)/2 + 25;	//x2
			baller.top[3]= y_player + 25;											//y2

			//coordinates for detecting collisions with the objects
			baller.bottom[0] = x_player + 100;  //x1
			baller.bottom[1] = y_player + 180;  //y1
			baller.bottom[2] = x_player + 120;  //x2
			baller.bottom[3]= y_player + 200;	//y2


	

		//the car in the front
			if(gamePlayFront){
				if( checkPoint >= 11  && x_off > current_x_off + 300){
					
					if(objects[0].draw){
							//the car in the very front
						if(objects[0].counter == objects[0].sprite_count)
							objects[0].counter = 0;
						al_draw_bitmap_region(objects[0].sprite_sheet,objects[0].w_img * objects[0].counter / objects[0].sprite_count , 0, objects[0].w_img/objects[0].sprite_count,objects[0].h_img,objects[0].positionX,objects[0].positionY,0);
						
						//get coordinates for collision detection
						objects[0].x1 = objects[0].positionX + 50;
						objects[0].y1 = objects[0].positionY + 200 ;
						objects[0].x2 = objects[0].positionX + objects[0].w_img / (2 * objects[0].sprite_count);
						objects[0].y2 =	objects[0].positionY + 240;


						
						objects[0].counter += 1;
						objects[0].positionX -= moveSpeed;

						if(objects[0].positionX < - 300)
							objects[0].positionX = rand() % 1800 + WIDTH;


						//collision detection for car in the front
						if(!playerDunking && !hoopEffect &&  baller.bottom[3] > objects[0].y1 && baller.bottom[1] < objects[0].y2 && baller.bottom[2] > objects[0].x1 && baller.bottom[0] < objects[0].x2 ){
							collided = true;
							getRandNum = true;
						}
							
					}
					

				}

			}



			//when the user collides with object, or the hoop passes the screen without succesful dunk
			if(collided){
				
				// getRandNum = true;
				gamePlay = false;
				gamePlayFront = false;
				drawPlayer = false;
				drawPlayerBack = false;

				playerStopping = false;
				playerDribbling = false;
				playerDunking = false;

				al_draw_rotated_bitmap(baller.image,al_get_bitmap_width(baller.image)/2, al_get_bitmap_height(baller.image)/2,x_player+ 20,y_player + 200, -90,0);


				al_draw_text(font_100_tarrget,text_col,100,textCenterY + 100,0,"game over");
				al_draw_multiline_text(font_30_man ,text_col,200,textCenterY + 220,1000,40,0,"Press ENTER to restart\n  from the last checkpoint, else\n    ESC to quit");
				al_draw_text(font_20_man ,text_col_green,50,textCenterY - 50,0,"Did you know that:");



				//display one of the facts below randomly
				switch (randNum) {
					case 1:
					al_draw_multiline_text(font_20_man ,text_col_green,50,textCenterY - 10,1000,25,0,"Cars, trucks and other forms of transportation\nare the single largest contributor to air pollution\nin most of the countries");
					break;
					case 2:
					al_draw_multiline_text(font_20_man ,text_col_green,50,textCenterY - 10,1000,25,0,"Vehicles emit carbon dioxide and other greenhouse gases,\nand car pollution is one of the major causes of global warming");
					break;
					case 3:
					al_draw_multiline_text(font_20_man ,text_col_green,50,textCenterY - 10,1000,25,0,"Vehicles release emissions even when it's standing still");
					break;
					case 4:
					al_draw_multiline_text(font_20_man ,text_col_green,50,textCenterY - 10,1000,25,0,"The effects of car pollution are widespread,\naffecting air, soil and water quality");
					break;
					case 5:
					al_draw_multiline_text(font_20_man ,text_col_green,50,textCenterY - 10,1000,25,0,"Old and poorly maintained vehicles cause most pollution,\nbut electric, hybrid and other clean, fuel-efficient cars\nhave a reduced impact");
					break;
					
				
				}
				
				

				keys[UP] = false;
				keys[DOWN] = false;
				keys[RIGHT] = false;

				
				
				
			}

			//get a random number each time user fails in order to showing from the list of the facts
			if(getRandNum){
				randNum = rand() % 5 + 1;
				getRandNum = false;
			}

			//help menu
			if(help){

				//user provokes help menu in the middle of the game 
				if(!intro){
					gamePlay = false;
					gamePlayFront = false;
				}
				//help menu before starting the game
				else {
					intro = false;
				}
				
				
				al_draw_multiline_text(font_25_man,text_col_green,220,HEIGHT - 180,1500,40,0,"RIGHT - dribble and move forward\nLEFT - slide back (after dunk attempts only)\nSPACE - regular dunks\nLSHIFT + SPACE - long dunks");
				al_draw_multiline_text(font_25_man,text_col_green,50,50,1500,40,0,"Hold on SPACE when according distant to hoop for a\nsuccesful dunk.Player becomes invicible after\neach succesful dunk but only until that hoop passes\nthe screen.");


			}




			//end of the game
			if(endGame){

				gamePlay = false;
				gamePlayFront = false;

				playerStopping = false;
				playerDribbling = true;
				boundary =false;

				al_draw_multiline_textf(font_40_tarrget,text_col,textX ,textCenterY,800,40,0,"thank you so much for playing this game\n\nyou made %d dunks out of total %d attempts\nand traveled for %d",madeNum,attemptNum,current_x_off);
				al_draw_text(font_30_man,text_col_green,textX + 30,textCenterY - 60,0,"let\'s keep our earth healthy");
				
				moveSpeed1 = 1;
				textX -= playerDribbling  * moveSpeed1;
			}


			//testing
			// al_draw_rectangle(objects[1].x1,objects[1].y1,objects[1].x2,objects[1].y2,text_col,2);
			// al_draw_rectangle(objects[0].x1,objects[0].y1,objects[0].x2,objects[0].y2,text_col,2);
			// al_draw_rectangle(objects[0].x1 - 50,objects[0].y1 - 300,objects[0].x2,objects[0].y2,text_col_green,2);
			// al_draw_rectangle(objects[2].x1,objects[2].y1,objects[2].x2,objects[2].y2,text_col,2);
			// al_draw_rectangle(objects[2].x1 - 50,objects[2].y1 - 300,objects[2].x2,objects[2].y2,text_col_green,2);


			al_flip_display();
			al_clear_to_color(al_map_rgb(0,0,0));



		//end of render
		}	 

	//end of while
	}




	MapFreeMem();
    al_stop_timer(timer);
	al_destroy_event_queue(queue);
	al_destroy_display(display);
	

	al_destroy_font(font_40_man);
    al_destroy_font(font_30_man);
	al_destroy_font(font_25_man);
	al_destroy_font(font_20_man);
    al_destroy_bitmap(baller.image);
	al_destroy_bitmap(baller2.image);
	al_destroy_bitmap(baller3.image);

    
    return 0;


}


//loads objects used in the game
void loadObjects(Objects *objects){

	objects[1].sprite_sheet = al_load_bitmap("bitmaps/pollution_car1.png");
	objects[1].w_img = al_get_bitmap_width(objects[1].sprite_sheet);
	objects[1].h_img = al_get_bitmap_height(objects[1].sprite_sheet);
	objects[1].positionX = -300;
	objects[1].positionY = 170;
	objects[1].sprite_count = 14;

	objects[2].sprite_sheet = al_load_bitmap("bitmaps/pollution_car3.png");
	objects[2].w_img = al_get_bitmap_width(objects[2].sprite_sheet);
	objects[2].h_img = al_get_bitmap_height(objects[2].sprite_sheet);
	objects[2].positionX = WIDTH + 1500;
	objects[2].positionY = 120;
	objects[2].sprite_count = 14;


	objects[0].sprite_sheet = al_load_bitmap("bitmaps/pollution_car3.png");
	objects[0].w_img = al_get_bitmap_width(objects[0].sprite_sheet);
	objects[0].h_img = al_get_bitmap_height(objects[0].sprite_sheet);
	objects[0].positionX = WIDTH + 200;
	objects[0].positionY = 200;
	objects[0].sprite_count = 14;

	objects[3].sprite_sheet = al_load_bitmap("bitmaps/pollution_smoke.png");
	objects[3].w_img = al_get_bitmap_width(objects[3].sprite_sheet);
	objects[3].h_img = al_get_bitmap_height(objects[3].sprite_sheet);
	objects[3].positionX = WIDTH;
	objects[3].positionY = 300;
	objects[3].counter2 = 39;


	objects[4].sprite_sheet = al_load_bitmap("bitmaps/trump.png");
	objects[4].w_img = al_get_bitmap_width(objects[4].sprite_sheet);
	objects[4].h_img = al_get_bitmap_height(objects[4].sprite_sheet);
	objects[4].positionX = WIDTH + 200;
	objects[4].positionY = 200;
	objects[4].sprite_count = 8;
}

//change the states 
void changeCarState(int &carState, int newState){
	carState = newState;

}
void changeHoopState(int &hoopState, int newState){
	hoopState = newState;
}