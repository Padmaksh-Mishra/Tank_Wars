#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

# define LEDR_BASE             0xFF200000
# define HEX3_HEX0_BASE        0xFF200020
# define HEX5_HEX4_BASE        0xFF200030
# define PUSHBUTTONS ((volatile long *) 0xFF200050)
# define RLEDs ((volatile long *) 0xFF200000)
# define KEY_BASE 0xFF200050
# define LEDR_BASE 0xFF200000
# define RED 0xF800      //health == 1
# define CYAN 0x07FF     //health == 2
# define GREEN 0x07E0    //ball
# define BLUE 0x001F     //health == 3
# define PI 3.14159265359
# define MOON_RADIUS 10
# define MOON_POS_X 60
# define MOON_POS_Y 60


// Defining the structure of tank
struct Tank{
    int posx,posy;
    int angle;
    int health;
}player_A,player_B;

struct Bomb{
    float velx,vely;
    int posx,posy;
}bomb;

int turn;   // turn = 1 -> A else B


void write_pixel(int x, int y, int colour)
{
    volatile short *vga_addr = (volatile short *)(0x08000000 + (y << 10) + (x << 1));
    *vga_addr = colour;
}


void draw_line(int x, int y, int color,int z) {
	for(int j=0;j<5;j++){
 	   for (int i = 0; i < 15; i++) {
    	    if(z)write_pixel(x - i+j, y - i-j, color);
		   else write_pixel(x + i-j, y - i-j, color);
    	}
	}
}

void draw_tank_r(int x, int y, int color) {
    // Draw tank body
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 10; j++) {
            write_pixel(x + i, y + j, color);
        }
    }
    // Draw tank barrel
    for (int i = 0; i < 15; i++) {
        for (int j = 0; j < 5; j++) {
            write_pixel(x + 5 + i, y - 5 + j, 0xF800);
        }
    }
		draw_line(x, y, 0xF800,1);


}
void draw_tank_l(int x, int y, int color) {
    // Draw tank body
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 10; j++) {
            write_pixel(150-(x + i), (y + j), color);
        }
    }
    // Draw tank barrel
    for (int i = 0; i < 15; i++) {
        for (int j = 0; j < 5; j++) {
            write_pixel(150-(x + 5 + i), (y - 5 + j), 0xF800);
        }
    }
	draw_line(x+9, y, 0xF800,0);

}
void draw_home_page() {
    // Clear screen to black
    for (int i = 0; i < 320; i++) {
        for (int j = 0; j < 240; j++) {
            write_pixel(i, j, 0x000000);
        }
    }
    
    // Draw tanks
    draw_tank_l(70, 240 / 2 - 20, 0x00FF00);  // Red tank
    draw_tank_r(320 - 85, 240 / 2 - 20, 0x00FF00);  // Green tank
}



/* use write_pixel to set entire screen to black (does not clear the character buffer) */
void clear_screen()
{
    int x, y;
    for (x = 0; x < 320; x++)
    {
        for (y = 0; y < 240; y++)
        {
            write_pixel(x, y, 0);
        }
    }
}

void write_char(int x, int y, char c)
{
    // VGA character buffer
    volatile char *character_buffer = (char *)(0x09000000 + (y << 7) + x);
    *character_buffer = c;
}
void write_string(int x, int y, char *str)
{
    int i = 0;
    while (str[i] != '\0') {
        write_char(x + i, y, str[i]);
        i++;
    }
}


void ball_pos(int x, int y, int r, int c)
{
    int x1, y1;

    for (x1 = x - r; x1 <= x + r; x1++)
    {
        for (y1 = y - r; y1 <= y + r; y1++)
        {
            if ((x1 - x) * (x1 - x) + (y1 - y) * (y1 - y) <= r * r)
            {
                write_pixel(x1, y1, c);
            }
        }
    }
}
void prnt_rect(int x1, int x2, int y1, int y2, int c)
{
    int x, y;
    for (int x = x1; x <= x2; x++)
    {
        for (int y = y1; y <= y2; y++)
        {
            write_pixel(x, y, c);
        }
    }
}
int max(int a, int b)
{
    if (a > b)
        return a;
    else
        return b;
}
int min(int a, int b)
{
    if (b > a)
        return a;
    else
        return b;
}
void updateHealthToLEDR(int health1,int health2)
{
    volatile int *LEDR = (int *)LEDR_BASE;
    *LEDR = ((((int)(1<<health2)) - 1) + ((((int)(1<<10) - 1))^(((int)(1<<(10-health1)) - 1))))& 0b1111111111;
    
}
void draw_crescent_moon() {
    for (int x = -MOON_RADIUS; x < MOON_RADIUS; x++) {
        for (int y = -MOON_RADIUS; y < MOON_RADIUS; y++) {
            if (x*x + y*y <= MOON_RADIUS*MOON_RADIUS) { // inside moon circle
                write_pixel(MOON_POS_X + x, MOON_POS_Y + y, 0x00FFFFFF); // white
                
            }
        }
    }
    
    for (int x = -MOON_RADIUS; x < MOON_RADIUS; x++) {
        for (int y = -MOON_RADIUS; y < MOON_RADIUS; y++) {
            if (x*x + y*y <= MOON_RADIUS*MOON_RADIUS) { // inside moon circle
                write_pixel(MOON_POS_X +5  + x, MOON_POS_Y -5 + y, 0); // white
                
            }
        }
    }
}
void draw_power_bar(int x, int y, int fill_level) {
    int width = 20;
    int height = 10;
    fill_level%=21;
    // Draw empty box
    for (int i = x; i < x + width; i++) {
        for (int j = y; j < y + height; j++) {
            write_pixel(i, j, 0x00000000);
        }
    }

    // Calculate fill level
    int fill_width = (fill_level);// * width) / 100;

    // Draw filled bar
    for (int i = x; i < x + fill_width; i++) {
        for (int j = y; j < y + height; j++) {
            write_pixel(i, j, RED);
        }
    }
}
void erase_power_bar(int x, int y) {
    int length = 20;
    for (int i = x; i < x + length; i++) {
        for (int j = y; j < y + 10; j++) {
            write_pixel(i, j, 0x00000000);
        }
    }
}

void draw_stars() {
  draw_crescent_moon();
  // Draw stars
  int num_stars = 100;
  for (int i = 0; i < num_stars; i++) {
    int x = rand() % 320;
    int y = rand() % 240;
    write_pixel(x, y, 0xFFFFFFFF); // White
  }
}

void display_num(int x)
{
	volatile int *hex3_hex0_ptr = (int *)HEX3_HEX0_BASE; // Pointer to HEX3_HEX0
	
	int digit_values[] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F };
	int ones = x % 10; // Extract the ones digit
    int tens = (x / 10) % 10; // Extract the tens digit
    int hundreds = (x / 100) % 10; // Extract the hundreds digit
    int thousands = (x / 1000) % 10; // Extract the thousands digit

    // Set the values of the individual digits on the 7-segment displays
    *hex3_hex0_ptr = (digit_values[thousands] << 24) | (digit_values[hundreds] << 16) | (digit_values[tens] << 8) | digit_values[ones];
}


int main()
{
    clear_screen();
    draw_home_page();
	write_string(30,26,"Welcome to TANK WARS!!!   ");
    write_string(30,28,"     Loading.....         ");
    for(int i=0;i<10000000;i++);
	write_string(25,26,"                                      ");
    write_string(25,28,"                                      ");
    clear_screen();

    write_string(30,28,"                             ");
    write_string(30,30,"                             ");
    write_string(30,32,"                             ");
    write_string(30,35,"                             ");
    prnt_rect(158, 162, 170, 240, 0x07e0);  // wall 
    draw_stars();
    turn = 0;
    int xo,yo;
    
    //initlizing players
    player_A.health=3;
    player_A.posx = 20;
    player_A.posy = 240;
    player_A.angle = 0;

    player_B.health=3;
    player_B.posx = 280;
    player_B.posy = 240;
    player_B.angle = 0;
    int pwr=0;
    // draw_arrow(player_A.posx,player_A.posy,angle,0x07e0);

    //bomb

    int angle=45;
    bomb.velx = 5;
    int r = 3;
    int pbx = player_A.posx;
    int pby = player_A.posy-40;

    while (1)
    {
        int shoot=0;
        updateHealthToLEDR(player_A.health,player_B.health);
        turn^=1;
        pwr = 0;
        angle=45;
        long PBval;
        
        while (1)
        {
            
            int c1,c2;
            if(player_A.health==3)c1=BLUE;
            else if(player_A.health==2)c1=CYAN;
            else if(player_A.health==1)c1=RED;
            if(player_B.health==3)c2=BLUE;
            else if(player_B.health==2)c2=CYAN;
            else if(player_B.health==1)c2=RED;

            prnt_rect(player_A.posx, player_A.posx + 20, 240 - 20, 240, c1);      //tank 1
            prnt_rect(player_B.posx, player_B.posx + 20, 240 - 20, 240, c2);     // tank 2

            for (int i = 0; i < 2000000; i++);
            PBval = *PUSHBUTTONS;
            display_num(pwr);
            if (PBval & 0x01) // Check if button KEY0 is pressed
            {
                if(turn){
                    erase_power_bar(player_A.posx,player_A.posy-40);
                    player_A.posx++;
                    draw_power_bar(player_A.posx,player_A.posy-40,pwr);
                }
                else{
                    erase_power_bar(player_B.posx,player_B.posy-40);
                    player_B.posx++;
                    draw_power_bar(player_B.posx,player_B.posy-40,pwr);
                }

                prnt_rect(player_A.posx-1, player_A.posx-1 + 20, 240 - 20, 240, 0);      //tank 1
                prnt_rect(player_B.posx-1, player_B.posx-1 + 20, 240 - 20, 240, 0); 
            }
            else if (PBval & 0x02) // Check if button KEY1 is pressed
            {
                 if(turn){
                    erase_power_bar(player_A.posx,player_A.posy-40);
                    player_A.posx--;
                    draw_power_bar(player_A.posx,player_A.posy-40,pwr);
                }
                else{
                    erase_power_bar(player_B.posx,player_B.posy-40);
                    player_B.posx--;
                    draw_power_bar(player_B.posx,player_B.posy-40,pwr);
                }
                prnt_rect(player_A.posx+1, player_A.posx+1 + 20, 240 - 20, 240, 0);      //tank 1
                prnt_rect(player_B.posx+1, player_B.posx+1 + 20, 240 - 20, 240, 0); 
            }
            else if (PBval & 0x04) // Check if button KEY2 is pressed
            {
                
                pwr++;
                pwr%=21;
                if(turn){
                    draw_power_bar(player_A.posx,player_A.posy-40,pwr);
                }else{
                    draw_power_bar(player_B.posx,player_B.posy-40,pwr);
                }
                
            }
            else if (PBval & 0x08) // Check if button KEY3 is pressed
            {
                erase_power_bar(player_A.posx,player_A.posy-40);
                erase_power_bar(player_B.posx,player_B.posy-40);
                shoot=1;
            }
           
            if(shoot) break;

        }

        PBval = 0;
        bomb.velx=pwr;//*(0.70706);
        if(turn){
            bomb.posx = player_A.posx+10;
            bomb.velx *=(1);
        }else{
            bomb.posx = player_B.posx+10;
            bomb.velx *=(-1);
        }
        
        bomb.posy = 217;
        bomb.vely = -pwr;//*(0.70706);
        printf("move\n");

        while (1)
        {
            draw_crescent_moon();
            if (bomb.velx == 0 && bomb.vely == 0 && bomb.posy + r >= 240)
                break;
            else if (bomb.vely == 0 && bomb.posy + r >= 240)
            {
                if (bomb.velx > 0)
                    bomb.velx = max(0, bomb.velx - ((5)));
                if (bomb.velx < 0) 
                    bomb.velx = min(0, bomb.velx + (5));
            }
            // print
            ball_pos(bomb.posx, bomb.posy, 2, 0x07e0);
            prnt_rect(158, 162, 170, 240, 0x07e0);  // wall 

            int c1,c2;
            if(player_A.health==3)c1=BLUE;
            else if(player_A.health==2)c1=CYAN;
            else if(player_A.health==1)c1=RED;
            if(player_B.health==3)c2=BLUE;
            else if(player_B.health==2)c2=CYAN;
            else if(player_B.health==1)c2=RED;

            prnt_rect(player_A.posx, player_A.posx + 20, 240 - 20, 240, c1);      //tank 1
            prnt_rect(player_B.posx, player_B.posx + 20, 240 - 20, 240, c2);     // tank 2
            xo = bomb.posx, yo = bomb.posy;
            bomb.posx += bomb.velx;
            bomb.posy += bomb.vely;
            // printf("%f",bomb.vely);
            if (bomb.posy + r <= 240)
                bomb.vely += ((1));
            if (bomb.posx + bomb.velx - r < 0 || bomb.posx + bomb.velx + r >= 320)
            {
                bomb.velx = -bomb.velx;
                if (bomb.velx > 0)
                    bomb.velx = max(0, bomb.velx - ((5)));
                if (bomb.velx < 0)
                    bomb.velx = min(0, bomb.velx + (5));
            }
            if (bomb.posy + bomb.vely - r < 0 || bomb.posy + bomb.vely + r > 240)
            {
                bomb.vely = -bomb.vely;
                
                if (bomb.vely > 0)
                    bomb.vely = max(0, bomb.vely - ((5)));
                if (bomb.vely < 0)
                    bomb.vely = min(0, bomb.vely + (5));

            }
            // coff of rest and wall bounce
            if (((bomb.posx + bomb.velx + r > 158 && bomb.posx + r <= 158) || (bomb.posx + bomb.velx + r < 162 && bomb.posx + r >= 162)) && (bomb.posy + bomb.vely + r > 170))
            {
                bomb.velx = -bomb.velx;

                if (bomb.velx > 0)
                    bomb.velx = max(0, bomb.velx - ((5)));
                if (bomb.velx < 0)
                    bomb.velx = min(0, bomb.velx + (5));
            }
            if ((bomb.posx + bomb.velx - r < 162 && bomb.posx + bomb.velx + r >= 158) && (bomb.posy + bomb.vely + r > 170 && bomb.posy + r <= 170))
            {
                bomb.vely = -bomb.vely;
                if (bomb.vely > 0)
                    bomb.vely = max(0, bomb.vely - ((5)));
                if (bomb.vely < 0)
                    bomb.vely = min(0, bomb.vely + (5));

            }

            if(((bomb.posx-r<player_A.posx+20 && bomb.posx-r>player_A.posx)||(bomb.posx+r<player_A.posx+20 && bomb.posx+r>player_A.posx))&&(bomb.posy + r > 240 - 20)){
                player_A.health--;
                break;
            }
            if(((bomb.posx-r<player_B.posx+20 && bomb.posx-r>player_B.posx)||(bomb.posx+r<player_B.posx+20 && bomb.posx+r>player_B.posx))&&(bomb.posy + r > 240 - 20)){
                player_B.health--;
                break;
            }
            
            
            for (int i = 0; i < 2000000; i++);

            ball_pos(xo, yo, 2, 0);
            
        }

        ball_pos(xo, yo, 2, 0);
        
        int c1,c2;
        if(player_A.health==3)c1=BLUE;
        else if(player_A.health==2)c1=CYAN;
        else if(player_A.health==1)c1=RED;
        if(player_B.health==3)c2=BLUE;
        else if(player_B.health==2)c2=CYAN;
        else if(player_B.health==1)c2=RED;

        prnt_rect(player_A.posx, player_A.posx + 20, 240 - 20, 240, c1);      //tank 1
        prnt_rect(player_B.posx, player_B.posx + 20, 240 - 20, 240, c2);     // tank 2
        updateHealthToLEDR(player_A.health,player_B.health);
        if (player_A.health == 0 || player_B.health == 0){
            clear_screen();
            write_string(35,28,"---------------");
            write_string(35,30,"|| Game over ||");
            write_string(35,32,"---------------");
            if(player_A.health) write_string(35,35,"Player A wins");
            else write_string(35,35,"Player B wins");
            break;
        }
        
    }
    return 0;
}