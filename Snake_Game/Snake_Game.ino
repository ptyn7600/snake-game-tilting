#include "U8glib.h"
#include "Wire.h" // This library allows you to communicate with I2C devices.

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);//Set device name：I2C-SSD1306-128*64(OLED)
/********************* Pin Assignment ***********************/
//============ JOYSTICK PIN =====================================
int URX = A0;
int URY = A1;
#define PinSW 2
//================================================================

// =========== Defined Variables for Joystick Direction ==========
#define RIGHT        0
#define LEFT         1
#define UP           2
#define DOWN         3
#define PRESSDOWN    4
// ===============================================================

// ========= Global Variable ============
// --------- Joystick Status ------------
int xPosition = 0;
int yPosition = 0;
int buttonState = 0;
// --------------------------------------

// ---------- Acceleromater Status ------
int16_t accelerometer_x, accelerometer_y; // variables for accelerometer raw data
int16_t gyro_x, gyro_y; // variables for gyro raw data

// --------- Box Param ------------------
int box_x = 0;
int box_y = 0;
int box_x_length = 98;//x direction 32  0-31
int box_y_length = 62;//y direction 20  0-19
//---------------------------------------

// --------- Snake Param ----------------
int snake_max_length = 100; //The maximum body length of a snake
int snake_x[100];//x-coordinate
int snake_y[100];//y-coordinate
int snake_body_width = 3;  //Square width of snake body (square)
int snake_length = 3;      //Defines the initial snake body length
// Snake direction - default = Right
int snake_dir = RIGHT ;
// --------- Food position -------------
int food_x;   //Food position x
int food_y;   //Food position y
// Game speed
unsigned int game_speed = 20;
int game_mode = 0 ;
int wall_block_mode = 0;
int touch_wall = 0;
int score = 0;
// -------------------------------------
const int MPU_ADDR = 0x68; // I2C address of the MPU-6050. If AD0 pin is set to HIGH, the I2C address will be 0x69.

/********************** SET UP ************************/
void setup(void)
{
  // Serial for testing
  Serial.begin(9600);
  // ---- Setting pin for Joystick -----
  pinMode(URX, INPUT);
  pinMode(URY, INPUT);
  pinMode(PinSW, INPUT_PULLUP);
   // ---- Setting up accelerometer
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  // ---- Showing welcome window -----
  welcome();
  delay(2500);
  // ---- Showing the menu to start the game -----
  // chose_game_control();
 
}


/***********************************************/
/**
   This function will print the window to introduce the game
*/
void welcome()
{
  u8g.firstPage();//First page display
  do
  {
    u8g.setFont(u8g_font_gdr14r);//coordinate function
    u8g.setPrintPos(0, 20);
    u8g.print("Snake Game");
    u8g.setPrintPos(0, 38);
    u8g.print("   Nhu Phan");
  } while (u8g.nextPage());
}

/***********************************************/
/**
   There are two modes: tilting to play or using the joystick
   return an integer that represents the chosen mode: 1 for tilting, 2 for joystick
*/
void chose_game_control()
{
  int flag = 1;
  while (flag)
  {
    int key = read_key();
    if (key == UP)
    {
      u8g.firstPage();
      do
      {
        u8g.setFont(u8g_font_9x18);
        u8g.setPrintPos(5, 20);
        u8g.print("Tilting  <");
        u8g.setPrintPos(5, 40);
        u8g.print("Joystick");
      } while (u8g.nextPage());
      game_mode = 1;
    }
    if (key == DOWN)
    {
      u8g.firstPage();
      do
      {
        u8g.setFont(u8g_font_9x18);
        u8g.setPrintPos(5, 20);
        u8g.print("Tilting");
        u8g.setPrintPos(5, 40);
        u8g.print("Joystick <");
      } while (u8g.nextPage());
      game_mode = 2;
    }
    if (key == PRESSDOWN)
    {
      flag = 0;
    }
  }
  Serial.println("to the wall block");
  chose_wall_block_mode();
}

/***********************************************/
/**
   There are two modes: tilting to play or using the joystick
   return an integer that represents the chosen mode: 1 for tilting, 2 for joystick
*/
void chose_wall_block_mode()
{
  delay(1000);
  int flag = 1;
  while (flag)
  {
    int key = read_key();
    if (key == UP)
    {
      u8g.firstPage();
      do
      {
        u8g.setFont(u8g_font_9x18);
        u8g.setPrintPos(5, 20);
        u8g.print("Wall Block  <");
        u8g.setPrintPos(5, 40);
        u8g.print("Open Wall");
      } while (u8g.nextPage());
      wall_block_mode = 1;
    }
    if (key == DOWN)
    {
      u8g.firstPage();
      do
      {
        u8g.setFont(u8g_font_9x18);
        u8g.setPrintPos(5, 20);
        u8g.print("Wall Block");
        u8g.setPrintPos(5, 40);
        u8g.print("Open Wall <");
      } while (u8g.nextPage());
      wall_block_mode = 0;
    }
    if (key == PRESSDOWN)
    {
      flag = 0;
    }
  }
}



/***********************************************/
/**
   This function reads the joystick to determine if user pressed UP, DOWN, RIGHT, LEFT, PUSH-DOWN
   return An integer to represent the direction
*/
int read_accelerometer()
{
  int key_return = -1;
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H) [MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2, p.40]
  Wire.endTransmission(false); // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept active.
  Wire.requestFrom(MPU_ADDR, 7*2, true); // request a total of 7*2=14 registers
  
  accelerometer_x = Wire.read()<<8 | Wire.read(); // reading registers: 0x3B (ACCEL_XOUT_H) and 0x3C (ACCEL_XOUT_L)
  accelerometer_y = Wire.read()<<8 | Wire.read(); // reading registers: 0x3D (ACCEL_YOUT_H) and 0x3E (ACCEL_YOUT_L)
  gyro_x = Wire.read()<<8 | Wire.read(); // reading registers: 0x43 (GYRO_XOUT_H) and 0x44 (GYRO_XOUT_L)
  gyro_y = Wire.read()<<8 | Wire.read(); // reading registers: 0x45 (GYRO_YOUT_H) and 0x46 (GYRO_YOUT_L)
  if (accelerometer_x < -5000) {
    //    Serial.println("DOWN");
    key_return = DOWN;
    return key_return;

  }
  if (accelerometer_x > 5000) {
    //    Serial.println("UP");
    key_return = UP;
    return key_return;

  }
  if (accelerometer_y < -5000) {
    //    Serial.println("RIGHT");
    key_return = RIGHT;
    return key_return;
  }
  if (accelerometer_y > 5000) {
    //    Serial.println("LEFT");
    key_return = LEFT;
    return key_return;
  }
  return key_return;
}

/***********************************************/
/**
   This function reads the joystick to determine if user pressed UP, DOWN, RIGHT, LEFT, PUSH-DOWN
   return An integer to represent the direction
*/
int read_key()
{
  int key_return = -1;
  xPosition = analogRead(URX);
  yPosition = analogRead(URY);
  buttonState = digitalRead(PinSW);
  xPosition = map(xPosition, 0, 1023, -512, 512);
  yPosition = map(yPosition, 0, 1023, -512, 512);
  if (yPosition < -20) {
    //    Serial.println("UP");
    key_return = UP;
    return key_return;

  }
  if (yPosition > 500) {
    //    Serial.println("DOWN");
    key_return = DOWN;
    return key_return;

  }
  if (xPosition < -20) {
    //    Serial.println("LEFT");
    key_return = LEFT;
    return key_return;
  }
  if (xPosition > 500) {
    //    Serial.println("RIGHT");
    key_return = RIGHT;
    return key_return;
  }
  if (digitalRead(PinSW) == LOW) {
    //    Serial.println("PRESS DOWN");
    key_return = PRESSDOWN;
    return key_return;
  }
  return key_return;
}

/***********************************************/
void gameOver()
{
   u8g.firstPage();
   do
   {
     u8g.setFont(u8g_font_gdr14r);
     u8g.setPrintPos(0, 40);
     u8g.print(" GAME OVER!");
   } while (u8g.nextPage());  
   snake_length = 3;
   snake_dir = RIGHT;

   snake_x[0] = 15; snake_y[0] = 15;//snake starting coordinates
   snake_x[1] = snake_x[0]  - 1; snake_y[1] = snake_y[0];//snake starting coordinates
   snake_x[2] = snake_x[1]  - 1; snake_y[2] = snake_y[1];//snake starting coordinates

   score = 0;
   touch_wall = 0;
}

/**
   Snake funtion to mainly control the snake game
*/
void snake()
{
  Serial.println("Inside the snake");
  int flag = 1;
  int readKey;
  Serial.println(game_mode);
  Serial.println(wall_block_mode);
  // Initial Position of the Snake - length = 3
  snake_x[0] = 15; snake_x[1] = snake_x[0]  - 1; snake_x[2] = snake_x[1]  - 1;
  snake_y[0] = 15; snake_y[1] = snake_y[0]; snake_y[2] = snake_y[1];
  // Generate the very first food
  food();
  while (flag) {
    int temp = gameDisplay();
    flag = ((!wall_block_mode && temp) || (wall_block_mode && !touch_wall));
    delay(game_speed);
    if (game_mode == 1)
    {  
      readKey = read_accelerometer();
    } else {
      readKey = read_key();
    }
    switch (readKey)
    {
      case UP:
        if (snake_dir != DOWN)
          snake_dir = UP;
        break;
      case DOWN:
        if (snake_dir != UP)
          snake_dir = DOWN;
        break;
      case LEFT:
        if (snake_dir != RIGHT)
          snake_dir = LEFT;
        break;
      case RIGHT:
        if (snake_dir != LEFT)
          snake_dir = RIGHT;
        break;
      default: break;
    }
    moveSnake();
    Serial.println(touch_wall);
  }
  gameOver();
}

/***********************************************/
int gameDisplay()
{
  u8g.firstPage();
  do
  {
    // ---------- Draw the playing area box ------------
    u8g.drawFrame(box_x, box_y, box_x_length, box_y_length);
    //    u8g.setFont(u8g_font_5x7);
    //    u8g.setPrintPos(box_x_length + 1, 6);
    // -------------------------------------------------
    // ----------- Show the score ----------------------
    u8g.setFont(u8g_font_5x7);
    u8g.setPrintPos(box_x_length + 1, 6);
    u8g.print(" Score");
    u8g.setPrintPos(box_x_length + 13, 16);
    u8g.print(score);
    // --------------------------------------------------
    
    // ----------------- Draw the food -----------------
    Serial.print(food_x);
    Serial.print("-----");
    Serial.println(food_y);
    u8g.drawFrame(food_x*snake_body_width+1, food_y*snake_body_width+1, snake_body_width, snake_body_width);
    // -------------------------------------------------

    // ---------- Print food position and length of the snake -----------------
    // Snake Length
    u8g.setPrintPos(box_x_length + 1, 25);
    u8g.print("Length");
    u8g.setPrintPos(box_x_length + 13, 37);
    u8g.print(snake_length);
    // Food position
    u8g.setPrintPos(box_x_length + 1, 50);
    u8g.print(" X  Y");
    u8g.setPrintPos(box_x_length + 5, 60);
    u8g.print(food_x);
    u8g.setPrintPos(box_x_length + 17, 60);
    u8g.print(food_y);

    // -------------------------------------------------
    // ---------- Placing the snake --------------------
    for (int i = 0; i < snake_length; i++)
    {
      if (i == 0)
      {
        u8g.drawBox(snake_x[i]*snake_body_width + 1, snake_y[i]*snake_body_width + 1, snake_body_width, snake_body_width);
      }
      else
      {
        u8g.drawFrame(snake_x[i]*snake_body_width + 1, snake_y[i]*snake_body_width + 1, snake_body_width, snake_body_width);
      }
    }
    // -------------------------------------------------
    snakeEatFood();
  } while (u8g.nextPage());
  return eatYourSelf();
}

/***********************************************/
void food()
{
   int flag = 1;
   while (flag)
   {
     food_x = random(0,(box_x_length-2)/3);
     food_y = random(0,(box_y_length-2)/3);
     
     for (int i = 0; i < snake_length; i++)
     {
         if((food_x == snake_x[i])&&(food_y == snake_y[i]))
         {
          Serial.println("Food overlap");
          break;
         }
         flag = 0;
     }
   }
}
/***********************************************/
void moveSnake () {
  int temp_x[snake_length];
  int temp_y[snake_length];
  for (int i = 0; i < snake_length - 1; i++) //Old coordinate data stored in temp array, last bit data invalid
  {
    temp_x[i] = snake_x[i];
    temp_y[i] = snake_y[i];
  }
  switch (snake_dir)
  {
    case UP: 
      if ((snake_y[0] == 0)) 
      { 
        snake_y[0] = 19; 
        touch_wall = 1;
        break;
      }
      else
      {
        snake_y[0] -= 1; 
        touch_wall = 1;
        break;
      }
    case DOWN: 
      if (snake_y[0] == 19) 
      { 
        snake_y[0] = 0; 
        touch_wall = 1;
        break;
      }
      else
      {
        snake_y[0] += 1; 
        break;
      }
    case RIGHT:
      if (snake_x[0] == 31) 
      {
        snake_x[0] = 0;   
        touch_wall = 1; 
        break; 
      } else {
        snake_x[0] += 1; 
        break;  
      }
    case LEFT: 
    if (snake_x[0] == 0) 
    {
      snake_x[0] = 31;   
      touch_wall = 1; 
      break; 
    } else {
      snake_x[0] -= 1; 
      break;  
    }
    default: break;
  }
  // The body follows the head position
  for (int i = 1; i < snake_length; i++)
  {
    snake_x[i] = temp_x[i - 1];
    snake_y[i] = temp_y[i - 1];
  }
}


/***********************************************/
int snakeEatFood()
{
  if((snake_x[0] == food_x)&&(snake_y[0] == food_y))
  {
     Serial.println("Snake eat food");
     snake_length += 1;
     score += 5;
     food();
     return 1;
  }
  return 0;
}

/***********************************************/
int eatYourSelf() 
{
  for (int i = 1; i < snake_length; i++)
  {
    if((snake_x[0] == snake_x[i])&&(snake_y[0] == snake_y[i]))
      return 0;
  }
  return 1;
}


/***********************************************/
void loop(void)
{
  while (1)
  {        
    Serial.println("In the while loop");
    chose_game_control();
    Serial.println("Next to the snake");
    snake();
  }
}

/***********************************************/
