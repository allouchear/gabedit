/* Textures.c */
/**********************************************************************************************************
Copyright (c) 2002-2013 Abdul-Rahman Allouche. All rights reserved

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the Gabedit), to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions
  of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
************************************************************************************************************/


#include "../../Config.h"
#include "GlobalOrb.h"
#define gl_max  256

/*********************************************************************************************/
GLubyte Texture[16] =
{
	0x99,0x99,0x99,0x99,0xFF,0xFF,0xFF,0xFF,              /*Image (2x2)*/
	0xFF,0xFF,0xFF,0xFF,0x99,0x99,0x99,0x99
};

static GLuint Nom = 0;
unsigned char image[256][256][3];
unsigned char image_granite[gl_max][gl_max][3];
/*********************************************************************************************/
int texture_function(int x,int y)
{
	float dx=(128.0-(float)x)/255.0*40.0;
	float dy=(128.0-(float)y)/255.0*40.0;
	float a=cos(sqrt(dx*dx+dy*dy)+0);
	return (int)((a+1.0)/2.0*255);
}

/*********************************************************************************************/
static float Interpolate(float a,float b,float x)
{
  float ft = x * 3.1415927 ;
  float f = (1 - cos(ft)) * .5 ;
  return  (a*(1-f) + b*f) ;
}
/*********************************************************************************************/
float Noise(int x, int y)
{
  int n = x + y * 57 ;
  n = (n<<13) ^ n;
  return ( 1.0 - ( (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0); 
}
/*********************************************************************************************/
float SmoothedNoise_1(float x, float y)
{
  float corners = ( Noise((int) x-1,(int) y-1)+Noise((int) x+1,(int) y-1)+Noise((int) x-1,(int) y+1)+Noise((int) x+1,(int) y+1) ) / 16 ;
  float sides   = ( Noise((int) x-1,(int) y)  +Noise((int) x+1,(int) y)  +Noise((int) x,(int) y-1)  +Noise((int) x,(int) y+1) ) /  8 ;
  float center  =  Noise((int) x,(int) y) / 4 ;
  return (corners + sides + center) ;
}
/*********************************************************************************************/
float InterpolatedNoise_1(float x, float y)
{
  int integer_X =(int) floor(x) ;
  float fractional_X = x - integer_X ;
  int integer_Y =(int) floor(y) ;
  float fractional_Y = y - integer_Y ;
  float v1 = SmoothedNoise_1(integer_X,     integer_Y) ;
  float v2 = SmoothedNoise_1(integer_X + 1, integer_Y) ;
  float v3 = SmoothedNoise_1(integer_X,     integer_Y + 1) ;
  float v4 = SmoothedNoise_1(integer_X + 1, integer_Y + 1) ;
  float i1 = Interpolate(v1 , v2 , fractional_X) ;
  float i2 = Interpolate(v3 , v4 , fractional_X) ;
  return (Interpolate(i1 , i2 , fractional_Y)) ;
}
/*********************************************************************************************/
float PerlinNoise_2D(float x, float y)
{
	float total = 0 ;
	float p = 0.25 ;
	int n = 3 ;
	float frequency;
	float amplitude;
	int i;

	for (i = 0 ; i <= n ; i++ )
	{
		frequency = 2<<i ;
		amplitude = pow(p,i) ;
		total += InterpolatedNoise_1(x * frequency,y*frequency) * amplitude ;
	}
	return (total) ;
}  
/*********************************************************************************************/
float noise(float s,float t)
{
	return((1+PerlinNoise_2D(s,t))/2.0F) ; 
}
/*********************************************************************************************/
void granite(float s, float t, float Kd,GLdouble Col[])
{
	float sum = 0;
	float freq = 1.0;
	int i;
	for (i = 0 ; i < 6 ; i++ )
	{
		sum += fabs(.5 - noise(12*freq*s,12*freq*t))/freq*1.4 ;
		freq *= 2;
	}
        if ( sum > 1/Kd )
		sum = 1/Kd ;
	Col[0] = sum * Kd ;
	Col[1] = sum * Kd ;
	Col[2] = sum * Kd ;
}  
/*********************************************************************************************/
void create_image_granite()
{
	GLdouble Col[3];
	float t = gl_max*gl_max*3 ;
	float s;
	int i;
	int j;

	for (i=0;i<gl_max;i++)
	{
		t =(float) i/(gl_max-1) ;
		for (j=0;j<gl_max;j++)
	    	{
		s =(float) j/(gl_max-1) ;
		granite(s,t,0.6,Col);
	        image_granite[i][j][0]=(int) (Col[0]*255);
	        image_granite[i][j][1]=(int) (Col[1]*255);
	        image_granite[i][j][2]=(int) (Col[2]*255);
	    	}
	}

}
/*********************************************************************************************/
void create_image(gint type)
{
	int i;
	int j;
	switch(type)
	{
		case 1 :
		for (i=0;i<256;i++)
	    		for (j=0;j<256;j++)
	    	{
	        image[i][j][0]=texture_function(i,j);
	        image[i][j][1]=128;
	        image[i][j][2]=128;
	    	}
		break;
		case 0 :
		for (i=0;i<256;i++)
	    		for (j=0;j<256;j++)
	    	{
	        image[i][j][0]=128;
	        image[i][j][1]=128;
	        image[i][j][2]=texture_function(i,j);
	    	}
		break;
		case 2 :
		for (i=0;i<256;i++)
	    		for (j=0;j<256;j++)
	    	{
	        image[i][j][0]=128;
	        image[i][j][1]=128;
	        image[i][j][2]=texture_function(i,j);
	    	}
		break;
	}
}
/*********************************************************************************************/
void activate_texture(gint type)
{
  glEnable(GL_TEXTURE_2D);                 /*Active le texturing*/
  switch(TypeTexture)
  {
  	case GABEDIT_TYPETEXTURE_1 :
		 create_image(type);
		 glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,256,256,0,GL_RGB,GL_UNSIGNED_BYTE,image);
		 break;
  	case GABEDIT_TYPETEXTURE_2 : 
/*  if( glIsTexture(Nom) != GL_TRUE)*/
  	{
  		glGenTextures(1,&Nom);                   /*new texture #*/
  		glBindTexture(GL_TEXTURE_2D,Nom);        /*select tis #*/
  		glTexImage2D (
       		GL_TEXTURE_2D,                      /*Type : texture 2D*/
       		0,                                  /*Mipmap : nothing*/
       		4,                                  /*colors : 4*/
       		2,                                  /*width : 2*/
       		2,                                  /*height : 2*/
       		0,                                  /*width of border : 0*/
       		GL_RGBA,                            /*Format : RGBA*/
       		GL_UNSIGNED_BYTE,                   /*Type of colors*/
       		Texture                             /*Address of image*/
  		);
	}
		break;
  	case GABEDIT_TYPETEXTURE_3 :
		 create_image_granite();
		 glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,gl_max,gl_max,0,GL_RGB,GL_UNSIGNED_BYTE,image_granite);
		 break;
  	case GABEDIT_TYPETEXTURE_NONE :
		 break;
  }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
}
/*********************************************************************************************/
void disable_texture()
{
  glDisable(GL_TEXTURE_2D);                 
}
