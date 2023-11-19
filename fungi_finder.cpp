/*
---------------------------INFO------------------------------

Every image needs a little tweeking with AVD_DIA to function 
properly. Here are some examples:
originalsrhoom1 needs 1200 to find all the mushrooms
originalshroom2 needs 1000 
originalshroom_overlap needs 800

If the mushrooms are too dirty this algorithm doenst work. 
See originalshroom_dirty.

The stars that were drawn are too small for this large-sized images
If anyone uses this algorithm with a camrea for Arduino (640*480)
they will be more visible.
"shrooms_found" image is where I show the result of "originalshroom1"
mushrooms centers.

The images I provided algonside the algorithm are my own 
and I wanted to prove that this algorithm works with real life
example images.

This algorithm can find the centers of the mushrooms on the image 
within very little error-margin

---------------------------INFO-----------------------------
*/



#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "./stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "./stb/stb_image_write.h"

#define GRAYSCALE     (1u)
#define JPG_QUALITY   (100u)
#define BLACK         (0u)
#define WHITE         (255u)
#define DARK          (50u)
#define THRESHOLD     (100u)
#define DEBUG_LEVEL   (0u)
#define LINE          (150)


using namespace std;

class Image{

  public:
    /*First argument is the want to be processed image and second is the name of the result image*/
    Image(const char *image_path,const char *resultimagename)
    {
      resultimage = resultimagename;

      image = stbi_load(image_path, &width, &height, &components, GRAYSCALE);
      if(image == NULL){
        cout << "Image cannot be loaded!" << endl;
        exit(1);
      } 

      image2d = (uint8_t **)malloc(height * sizeof(uint8_t *)); 
      for (int i = 0; i < height; i++)  image2d[i] = (uint8_t *)malloc(width * sizeof(uint8_t));

      for (int i = 0; i < height; ++i)
      {
        for (int ii = 0; ii < width; ++ii) {
          image2d[i][ii] = image[ii+(i*(width))];
        }
      }
      ToBlackAndWhite(THRESHOLD); //Threshold tells the B&W function wheter a pixel should be black ro white
      filter_image_columns(image2d);
      filter_image_rows(image2d);
      find_mushroom_3(image2d, 10);  //2nd argument determines how many rows to jump over to process a row
    }


    typedef struct
    {
      uint16_t x;
      uint16_t y;
    }Point;

    typedef struct
    {
      Point coordinate;
    }Mushroom;
    /*Draw lines where the algorithm processes hence make it easier to debug*/
    void draw_horizontal_line(uint8_t **image, uint16_t row, uint16_t from, uint16_t to)
    {
      for (uint16_t i = from; i <= to; i++) image[row][i] = LINE;
    }

    void draw_vertical_line(uint8_t **image, uint16_t column, uint16_t from, uint16_t to)
    {
      for (uint16_t i = from; i <= to; i++) image[i][column] = LINE;
    }
    /*Drawing a star where the final calculated center is*/
    void draw_star(uint8_t **image_p, Point p)
    {
      image_p[p.y][p.x] = DARK; 
      image_p[p.y+1][p.x+1] = DARK; 
      image_p[p.y+2][p.x+2] = DARK;  
      image_p[p.y-1][p.x+1] = DARK; 
      image_p[p.y-2][p.x+2] = DARK;  
      image_p[p.y+1][p.x-1] = DARK; 
      image_p[p.y+2][p.x-2] = DARK;  
      image_p[p.y-1][p.x-1] = DARK; 
      image_p[p.y-2][p.x-2] = DARK;  
    }
  /*These functions blur the image a bit so small imperfections like dirt can be reduced*/
  void filter_image_columns(uint8_t **image)
  {
	uint8_t left, middle = 0;
	for(int row = 0; row < height; ++row){
		for(int column = 0; column < width; ++column){	
			left = middle; 
			middle = image[row][column]; 
			image[row][column] = (uint8_t)((left + middle + image[row][column+1])/3);
		  }
	  }
  }

  void filter_image_rows(uint8_t **image)
{
	uint8_t left, middle;
	for(int column = 0; column < width; ++column){
		for(int row = 0; row < height-1; ++row){	
			left = middle; 
			middle = image[row][column]; 
			image[row][column] = (uint8_t)((left + middle + image[row][column+1])/3);
		}
	}
}

    uint16_t find_mushroom_3(uint8_t **image_p, uint16_t dy)
    {

    #define AVG_DIA (1200)
    #define DELTA_X (AVG_DIA/2)
    #define DELTA_Y (AVG_DIA/2)
    #define MAX_HITS (10000u)

      Mushroom mushroom[MAX_HITS] = {0};
      uint16_t hit_counter = 0;
      uint16_t mushroom_counter = 0;
      uint8_t potential_shroom_found = false;
      /*Making a black frame for the algorithm*/
     for(int x = 0; x < width; ++x){
        image_p[0][x] = BLACK;
        image_p[height-1][x] = BLACK;
      }

     for(int y = 1; y < height; ++y){
        image_p[y][0] = BLACK;
        image_p[y][width-1] = BLACK; 
      }

      for (uint16_t row = 1; row < height; row += dy){
        for (uint16_t column = 1; column < width; ++column){
          if(image_p[row][column] == BLACK){ 
            while((image_p[row][column] == BLACK) && (column < width)) ++column; //Go through black pixel until a white appears
            if(column != width){ 
              potential_shroom_found = process_white(image_p, &column, row, &mushroom[hit_counter]);
              if(potential_shroom_found == true && (hit_counter < MAX_HITS-1)) ++hit_counter;
             }
          }
        }
      }
      if(hit_counter == 0) return hit_counter;
      hit_counter -= 1; 
      average_hitpoints(hit_counter, mushroom);
      /*Iterating through hitpoints and draw stars where the calculated centers are*/
      for(uint16_t i = 0; i <= hit_counter; ++i){
        if(mushroom[i].coordinate.x != 0){
          draw_star(image_p, (Point){.x=mushroom[i].coordinate.x, .y=mushroom[i].coordinate.y});
          ++mushroom_counter;
        }
      }
      mushroomsfound = mushroom_counter;
      return mushroom_counter;
    }

    uint8_t process_white(uint8_t **image_p, uint16_t *column, uint16_t row, Mushroom *mushroom)
    {
      uint16_t white_start = 0;
      uint16_t white_end = 0;
      uint16_t top = 0;
      uint16_t bottom = 0;
      uint16_t dx = 0;
      uint16_t dy = 0;
      uint16_t y_base = 0;
      uint8_t mushroom_found = false;

      /*Go through white pixels until a black appears*/
      white_start = *column;	
      while(image_p[row][*column] != BLACK) (*column)++;

      white_end = (*column)-1; 
      *column -= 1;
      dx = white_end-white_start; //width of white pixels

      if(dx > (AVG_DIA-DELTA_X) && dx < (AVG_DIA+DELTA_X)){
        y_base = row; 

        while(image_p[row][*column] != BLACK) --row; 

        top = row+1; 
        row = y_base;	

        while(image_p[row][*column] != BLACK) ++row;

        bottom = row-1; 
        dy = bottom - top; //height of white pixels
        
       
        if(dy > (AVG_DIA-DELTA_X) && dy < (AVG_DIA+DELTA_X)){
          /*Calculating a potential center based on previous values*/
          row = y_base; 
          mushroom->coordinate.y = bottom - (bottom-top)/2; 
          mushroom->coordinate.x = (white_end + white_start)/2; 
    #if DEBUG_LEVEL >= 1u
          /*Developers can see that way the potential centers and the processed rows and columns*/
          make_cross(image2d, mushroom->coordinate.x, mushroom->coordinate.y); 
          draw_horizontal_line(image2d, row, white_start, white_end);
          draw_vertical_line(image2d, mushroom->coordinate.x, top, bottom);
    #endif
          mushroom_found = true;
        }
      }
      return mushroom_found;
    }

    void average_hitpoints(uint16_t hit_count, Mushroom *mushroom)
    {
      /*Iterate through the potential centers*/
      for(uint16_t current_point = 0; current_point < hit_count; ++current_point){
        uint16_t n = 0;
        Mushroom summ = {0};
        if(mushroom[current_point].coordinate.x == 0) continue; 
          for(uint16_t compare_point = current_point + 1; compare_point <= hit_count; ++compare_point){
            uint32_t dx = labs( mushroom[current_point].coordinate.x - mushroom[compare_point].coordinate.x);
            uint32_t dy = labs( mushroom[current_point].coordinate.y - mushroom[compare_point].coordinate.y);
            if(AVG_DIA/2 > (dx) && AVG_DIA/2 > (dy)){ //We don't want to process hitpoints far away from where we are currently, For example another mushroom's potential hitpoints can ruin this averaging
              summ.coordinate.x += mushroom[compare_point].coordinate.x;
              summ.coordinate.y += mushroom[compare_point].coordinate.y;
              ++n;
              mushroom[compare_point].coordinate.x = 0; //Cast it into the "graveyard" because we don't want to process it in the next iteration
            }
          }
          if(n != 0){
            summ.coordinate.x /= n; //Calculated center from real averages of potential centers
            summ.coordinate.y /= n;
            mushroom[current_point] = summ;

          }
        }
      }

/*Making little gray crosses for potential centers*/
void make_cross(uint8_t **image, uint16_t x, uint16_t y)
{
	image[y-1][x] = LINE;
	image[y][x-1] = LINE;
	image[y][x] = LINE;
	image[y][x+1] = LINE;
	image[y+1][x] = LINE;
}


uint16_t get_abs(int32_t a)
{
	if(a < 0){
		a = -a;	
		if(a < 0) a = 0x7fff;	
	}
	return a;
}

  /*Altering the image to black&white pixels for further process*/
    void ToBlackAndWhite(uint8_t treshold)
    {
      for (int  row = 0; row < height; ++row){
        for (int  column = 0; column < width; ++column)
        {
          if(image2d[row][column] <= treshold) image2d[row][column] = BLACK;
          else image2d[row][column] = WHITE;
        }
      }
    }

    uint8_t *GetImage(void)
    {
      for (int i = 0; i < height; ++i)
      {
        for (int ii = 0; ii < width; ++ii) {
          image[ii+(i*width)] = image2d[i][ii];
        }
      }
      return image;
    }

    void ShowImageInfo()
    {

      cout << "Image width: " << width << endl << "Image height: " << height << endl << "Mushrooms found: " << mushroomsfound << endl;
    }

    ~Image()
    {
      /*creating the resulting image and free the memory*/
      stbi_write_jpg(resultimage, width, height, 1, GetImage(), JPG_QUALITY);
      stbi_image_free(image);
      for (int i = 0; i < height; i++) free(image2d[i]);
      free(image2d);
    }

  private:
    uint8_t *image;
    uint8_t **image2d;
    const char *resultimage;
    int width;
    int height;
    int components;
    uint16_t mushroomsfound;
};



int main()
{

  Image example1("./originalshroom1.jpg", "ResultImage.jpg");
  example1.ShowImageInfo();

  


  return 0;
}
