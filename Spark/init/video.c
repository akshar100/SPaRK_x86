/* video.c 
 *
 * Written by Vicente Esteve LLoret <viesllo@inf.upv.es>
 * Copyright (C) Feb, 2003 OCERA Consortium.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation version 2.
 *
 * Some Visualization Functions.
 */




/*****************************************************************

  
  		  This is the module to write Video Memory
		  and functions to manage strings.

		  It must be implemented the API video interface
		  defined in Documentation directory
		  
		  
                  
Note:    Actually only works fine with direccion 0xB8000 and 80x25 visualizacion
	 But we could code others modules to make differents kinds 
	 of video access (i.e. Frame buffer access).
	 
	 The implementation of swapping between debugger -> aplication
	 is carry using 0x0c and 0x0d indexs from 0x3d4 VGA port

******************************************************************/

#define BASEKERNEL 0x00000000  
#define MEMVIDEO   0x000B8000

#define COLORSET1  0x12      /* Color por defecto del Deblin */ 
#define COLORSET2  0x50      /* Color de la linea en la que está
			        la instrucción en uso */

char COLOR = COLORSET1;


                             /* En estos 2 bytes se guarda el valor 
				de la dirección base de la memoria de video 
                                La memoria que hay en pantalla estará
                                en la dirección fisica 0xB8000+videobase */  
 
          /* mete un caracter car en la memoria de video , en las
          cordenadas (x,y) con atributos attrib */

void PutChar(unsigned long x,unsigned long y,char car,char attrib)
 {
 char *ptr=(char *) MEMVIDEO;
 
 ptr=ptr+(2*80*x);     
 ptr=ptr+(y*2);    /*calculo la direccion para visualizar el caracter */
                   /* Supongo temporalmente 80*25                     */
 *ptr=car;
 ptr++;
 *ptr=attrib;
 }

                 /* Visualiza un String por pantalla en las cordenadas
                    (x,y) y con los atributos de color attrib */

void PutString(unsigned long x,unsigned long y,char *ptr,char attrib)
 {
 
 while (*ptr!=0)
  {
  PutChar(x,y,*ptr,attrib);
  y++;
  ptr++;
  };

}
void DebugString(char *str)
{
  int i;
	
  PutString(1,1,str,0x12);
  for (i=0;i<0x1FFFFFFF;i++){__asm("nop\n");};
	
};

void DebugString2(char *str)
{
  PutString(2,1,str,0x12);
};


void ClearVHAL(void)
{
 int x,y;
 for (x=0;x<27;x++)
 for (y=0;y<80;y++)
 PutChar(x,y,' ',COLORSET1);
}

