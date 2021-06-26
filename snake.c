///////////////////////////////////
//       --Final Project--       //
//         *Snake Game*          //
//               By              //
//  Arazi David & Shmueli Muli   //
///////////////////////////////////

#include <stdio.h>	
#include <string.h>
#include "NUC1xx.h"
#include "DrvSYS.h"
#include "DrvGPIO.h"
#include "NUC1xx-LB_002\LCD_Driver.h"
#include "2D_Graphic_Driver.h"
#include "Driver\DrvSYS.h"
#include "Driver\DrvGPIO.h"
#include "Driver\DrvADC.h"
#include <stdlib.h>		
#include "scankey.h"
#include "Seven_Segment.h"
#include <stdlib.h>
#include <time.h>
#include "Driver\DrvUART.h"

#define pixel_size 4

static char TEXT1[16]="        ";
volatile char  bt_command;
unsigned char DisplayBuf [128*8];

typedef struct obstacle
{
	int8_t x0;
	int8_t y0;
	int8_t direction;
}obstacle;

typedef struct food
{
	int8_t x0;
	int8_t y0;
}food;

typedef struct snake
{
	int8_t x0;
	int8_t y0;
	int8_t num_part;
	struct snake* next;
	struct snake* prev;
}snake;

typedef struct list_snake
{
	snake* head;
	snake* tail;
}list_snake;

void initList_snake(list_snake* snake_list)
{
	snake_list->head = snake_list->tail = NULL;
}


int8_t isEmpty_snake(list_snake* snake_list)
{
	return snake_list->head == NULL;
}

int8_t length_snake(list_snake* lst)
{
	int8_t length ;
	length=0;
	snake* current;
	for (current = lst->head; current != NULL; current = current->next)
		length++;
  	return length;
}

snake allocItem_snake(int8_t x0, int8_t y0,int8_t num_part)
{
	snake item  ;
	item.x0 = x0;
	item.y0 = y0;
	item.num_part=num_part;
	return item;
}

int8_t insertFirst_part(list_snake* snake_list, snake* item)
{
	if (item != NULL)
	{
		//reset item pointers
		item->next = item->prev = NULL;

		if (isEmpty_snake(snake_list))
		{
			//make head & tail pointing to item
			snake_list->head = snake_list->tail = item;
		}
		else
		{
			//update first prev link
			snake_list->head->prev = item;
			//point item to old first link
			item->next = snake_list->head;
			//point head to new first user
			snake_list->head = item;
		}
		return 1;
	}
	return 0;
}

int8_t deleteLast_part(list_snake* snake_list)
{
	//if list is empty there is nothing to delete
	if (!isEmpty_snake(snake_list))
	{
		//save reference to last link
		snake* temp = snake_list->tail;
		//if only one link
		if (temp->prev == NULL)
		{
			initList_snake(snake_list);
		}
		else
		{
			snake_list->tail->prev->next = NULL;
			snake_list->tail = snake_list->tail->prev;
		}
		return 1;
	}
	return 0;
}

void UART_INT_HANDLE(void)
{
	bt_command='0';
	while(UART0->ISR.RDA_IF==1) 
	{
		 bt_command=UART0->DATA;
	}
}

void Init_RGBLED(void)
{
	// initialize GPIO pins
	DrvGPIO_Open(E_GPA, 12, E_IO_OUTPUT); // GPA12 pin set to output mode
	DrvGPIO_Open(E_GPA, 13, E_IO_OUTPUT); // GPA13 pin set to output mode
	DrvGPIO_Open(E_GPA, 14, E_IO_OUTPUT); // GPA14 pin set to output mode
	// set GPIO pins output Hi to disable LEDs
	DrvGPIO_SetBit(E_GPA, 12); // GPA12 pin output Hi to turn off Blue  LED
	DrvGPIO_SetBit(E_GPA, 13); // GPA13 pin output Hi to turn off Green LED
	DrvGPIO_SetBit(E_GPA, 14); // GPA14 pin output Hi to turn off Red   LED
}

void seg_display(int16_t value)
{
  	int8_t digit;
	digit = value / 1000;
	close_seven_segment();
	if(digit)
	{
		show_seven_segment(3,digit);		
		value = value - digit * 1000;
		digit = value / 100;
		close_seven_segment();
	}
	if(digit)
	{
		show_seven_segment(2,digit);
		value = value - digit * 100;
		digit = value / 10;
		close_seven_segment();
	}
	if(digit)
	{
		show_seven_segment(1,digit);
		DrvSYS_Delay(800);
	}
	value = value - digit * 10;
	digit = value;
	close_seven_segment();
	show_seven_segment(0,digit);
}

int8_t search_part(food*  current_food, list_snake* snake_list)
{
	snake* b = snake_list->head;
	
	while (b != NULL)
	{
		if ( (b->x0== current_food->x0)&(b->y0== current_food->y0)	)
		{
			return 1;
			break;
		}
		else
			b = b->next;
	}
	return 0;
}

int8_t check_self_collision(list_snake* snake_list)
{
	int8_t x,y;
	snake* b = snake_list->head;
	x=snake_list->head->x0;
	y=snake_list->head->y0;
	b = b->next;
	while (b != NULL)
	{
		if ( (b->x0== x)&(b->y0== y)	)
		{
			return 1;
			break;
		}
		else
			b = b->next;
	}
	return 0;
}

int8_t move(list_snake* snake_list,snake* a,int8_t direction)
{
	int8_t num_part_tail;
	if((((snake_list->head->x0)+pixel_size)>127  | snake_list->head->x0<0 | snake_list->head->y0<0 | ((snake_list->head->y0)+pixel_size)>63))
		return 1;
	num_part_tail= snake_list->tail->num_part;
	RectangleFill(snake_list->tail->x0,snake_list->tail->y0,(snake_list->tail->x0)+pixel_size,(snake_list->tail->y0)+pixel_size,0);
	RectangleDraw(1,1,126,62,1) ;
	deleteLast_part(snake_list);
	switch (direction)
      	{
		case 2 : //up
			a[num_part_tail] = allocItem_snake(snake_list->head->x0,snake_list->head->y0-pixel_size,num_part_tail);
			break;
		case 6 : //right	
		  	a[num_part_tail] = allocItem_snake(snake_list->head->x0+pixel_size,snake_list->head->y0,num_part_tail);
			break;
		case 8 : //down
        		a[num_part_tail] = allocItem_snake(snake_list->head->x0,snake_list->head->y0+pixel_size,num_part_tail);
			break;
		case 4 : //left
			a[num_part_tail] = allocItem_snake(snake_list->head->x0-pixel_size,snake_list->head->y0,num_part_tail);	
			break;
		default:
			NULL;				
	}
   	insertFirst_part(snake_list, &a[num_part_tail]);
	RectangleFill( a[num_part_tail].x0,a[num_part_tail].y0,(a[num_part_tail].x0)+pixel_size,(a[num_part_tail].y0)+pixel_size,1)	; 
	draw_LCD(DisplayBuf);		     // from display buffer to LCD
	return 0;
}

int8_t check_obstacle_collide(int8_t obstacle_on,list_snake* snake_list,obstacle* obstacle)
{
	snake* b;
     	b= snake_list->head;
	if(obstacle_on==1)
	{
		if( (((snake_list->head->x0+4)>=obstacle->x0)&(snake_list->head->x0<=(obstacle->x0+10)) &((snake_list->head->y0+4)>=obstacle->y0) &(snake_list->head->y0<=(obstacle->y0+10))) |
            	((snake_list->head->x0<=(obstacle->x0+10))&((snake_list->head->x0+4)>=obstacle->x0) &((snake_list->head->y0+4)>=obstacle->y0) &(snake_list->head->y0<=(obstacle->y0+10))) |
            	(((snake_list->head->x0+4)>=obstacle->x0)&(snake_list->head->x0<=(obstacle->x0+10)) &(snake_list->head->y0<=(obstacle->y0+10)) &((snake_list->head->y0+4)>=obstacle->y0))	)
			return 1;
	}
	else
	{
		while(b!=NULL)
		{
			if( (((b->x0+4)>=obstacle->x0)&(b->x0<=(obstacle->x0+10)) &((b->y0+4)>=obstacle->y0) &(b->y0<=(obstacle->y0+10))) |
            		((b->x0<=(obstacle->x0+10))&((b->x0+4)>=obstacle->x0) &((b->y0+4)>=obstacle->y0) &(b->y0<=(obstacle->y0+10))) |
            		(((b->x0+4)>=obstacle->x0)&(b->x0<=(obstacle->x0+10)) &(b->y0<=(obstacle->y0+10)) &((b->y0+4)>=obstacle->y0))	)
				return 1;
			b = b->next;
		}		
	}	
}

int8_t move_obstacle(int8_t obstacle_on, obstacle* obstacle1,obstacle* obstacle2 ,obstacle* obstacle3,obstacle* obstacle4 ,obstacle* obstacle5,list_snake* snake_list)
{
	snake* b;
   	b= snake_list->head;
  	RectangleDraw(obstacle1->x0,obstacle1->y0,obstacle1->x0+10,obstacle1->y0+10,0);
   	RectangleDraw(obstacle4->x0,obstacle4->y0,obstacle4->x0+10,obstacle4->y0+10,0);
   	if(obstacle_on==1)
   	{	 
		if(check_obstacle_collide(obstacle_on,snake_list,obstacle1))
			return 1;
		if(check_obstacle_collide(obstacle_on,snake_list,obstacle2))
			return 1;
		if(check_obstacle_collide(obstacle_on,snake_list,obstacle3))
			return 1;
		if(check_obstacle_collide(obstacle_on,snake_list,obstacle4))
			return 1;
		if(check_obstacle_collide(obstacle_on,snake_list,obstacle5))
			return 1;
   	}
	if(obstacle_on==2)
	{	 
		switch (obstacle1->direction)
		{
			case 6 : //right	
			{
				obstacle1->x0 = obstacle1->x0+2;
				if(obstacle1->x0==118)
				{
					obstacle1->direction=4;
					obstacle1->x0 = obstacle1->x0-2;	
				}
			break;
			}
			case		4 : //left
			{
				obstacle1->x0 = obstacle1->x0-2;
				if(obstacle1->x0==0)
				{
					obstacle1->direction=6;
					obstacle1->x0 = obstacle1->x0+2;
				}
				break;
		  	}	
			default:
				NULL;			
		  }	
			switch (obstacle4->direction)
     			 {
				case		6 : //right	
				{
					obstacle4->x0 = obstacle4->x0+2;
					if(obstacle4->x0==118)
					{
						obstacle4->direction=4;
						obstacle4->x0 = obstacle4->x0+2;	
					}
					break;
			  	}
				
				case		4 : //left
				{
					obstacle4->x0 = obstacle4->x0-2;
					if(obstacle4->x0==0)
					{
						obstacle4->direction=6;
						obstacle4->x0 = obstacle4->x0+2;	
					}
					break;
			 	 }
				default:
					NULL;			
		  	}	
	   		if(check_obstacle_collide(obstacle_on,snake_list,obstacle1))
				return 1;		
		 	if(check_obstacle_collide(obstacle_on,snake_list,obstacle4))
		   		return 1;
	  	}	
		if(obstacle_on==1)
		{
			RectangleDraw(obstacle2->x0,obstacle2->y0,obstacle2->x0+10,obstacle2->y0+10,1);		
	    		RectangleDraw(obstacle3->x0,obstacle3->y0,obstacle3->x0+10,obstacle3->y0+10,1);
			RectangleDraw(obstacle5->x0,obstacle5->y0,obstacle5->x0+10,obstacle5->y0+10,1);
		}		
	 	RectangleDraw(obstacle1->x0,obstacle1->y0,obstacle1->x0+10,obstacle1->y0+10,1);
		RectangleDraw(obstacle4->x0,obstacle4->y0,obstacle4->x0+10,obstacle4->y0+10,1);	
	 	draw_LCD(DisplayBuf);		     // from display buffer to LCD
	  	return 0;
}

void no_walls_func(list_snake* snake_list,snake* a)
{
	int8_t num_part_tail;
	num_part_tail= snake_list->tail->num_part;
	RectangleFill(snake_list->tail->x0,snake_list->tail->y0,(snake_list->tail->x0)+pixel_size,(snake_list->tail->y0)+pixel_size,0);
	RectangleDraw(1,1,126,62,1) ;
	deleteLast_part(snake_list);
	if((snake_list->head->x0)+pixel_size>127)
		a[num_part_tail] = allocItem_snake(2,snake_list->head->y0,num_part_tail);
	if((snake_list->head->x0)<0)
		a[num_part_tail] = allocItem_snake(122,snake_list->head->y0,num_part_tail);
	if((snake_list->head->y0)+pixel_size>63)
		a[num_part_tail] = allocItem_snake(snake_list->head->x0,2,num_part_tail);
	if((snake_list->head->y0)<0)
		a[num_part_tail] = allocItem_snake(snake_list->head->x0,58,num_part_tail);
	insertFirst_part(snake_list, &a[num_part_tail]);
	RectangleFill( a[num_part_tail].x0,a[num_part_tail].y0,(a[num_part_tail].x0)+pixel_size,(a[num_part_tail].y0)+pixel_size,1)	; 
	draw_LCD(DisplayBuf);		     // from display buffer to LCD
}

food new_food(list_snake* snake_list,int8_t random_seed,int8_t obstacle_on)
{
	snake* x;
	x = snake_list->head;
	food current_food ;
	int8_t c=1;
	srand(random_seed);
	while(c)
	{
		current_food.x0=rand()%118 ;
	  	current_food.y0=rand()%54 ;
		if(!(current_food.x0%4) & !(current_food.y0%4) )
		{
			current_food.x0=current_food.x0+2;
			current_food.y0=current_food.y0+2;
	    		c =  search_part(&current_food,snake_list);
			if(obstacle_on==1)
			{
				if((current_food.x0>49)&(current_food.x0<70)&(current_food.y0>17)&(current_food.y0<38))
					c=1;
				if(((current_food.x0>18)&(current_food.x0<39))&(((current_food.y0>2)&(current_food.y0<23))|((current_food.y0>32)&(current_food.y0<53))))
					c=1;
				if(((current_food.x0>80)&(current_food.x0<101))&(((current_food.y0>2)&(current_food.y0<23))|((current_food.y0>32)&(current_food.y0<53))))
					c=1;
			}
		}
	}
	RectangleFill(current_food.x0,current_food.y0,(current_food.x0)+pixel_size,(current_food.y0)+pixel_size,1)	; 
	return current_food;
}

int32_t main (void)
{
	int8_t direction=6; //2-up,6-right,8-down,4-left
	int8_t last_direction=6;
	int8_t score=0;
	int8_t score1;
	uint16_t Vx, Vy;
 	uint8_t  SW;
	int8_t random_seed=0;
	int8_t num;
	int8_t num_part=0;
	int8_t no_walls=-1;
  	list_snake snake_list;
	snake a[50]; //>50 wil be get problems on lcd ,probably memory..
	food current_food;
	int8_t yummy=0;
	int8_t device=-1; // 0 -keypad,1 joystick,2-bluetooth
	int8_t speed=-1; // 0-easy, 1-medium 2-hard
	int8_t state=-1;
	int8_t wall_collision=0;
	int8_t obstacle_collision=0;
	int8_t del_snake;
	int8_t obstacle_on=-1;
	obstacle obstacle1;
	obstacle obstacle2;
	obstacle obstacle3 ;
	obstacle obstacle4;
	obstacle obstacle5;
	STR_UART_T sParam;
	DrvGPIO_InitFunction(E_FUNC_UART0);	// Set UART pins
	/* UART Setting */
   	sParam.u32BaudRate 		  = 9600;
    	sParam.u8cDataBits 		  = DRVUART_DATABITS_8;
    	sParam.u8cStopBits 		  = DRVUART_STOPBITS_1;
    	sParam.u8cParity 		    = DRVUART_PARITY_NONE;
    	sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;
	/* Set UART Configuration */
 	if(DrvUART_Open(UART_PORT0,&sParam) != E_SUCCESS);
	DrvUART_EnableInt(UART_PORT0, DRVUART_RDAINT, UART_INT_HANDLE);
	UNLOCKREG();
	DrvSYS_Open(48000000);
	SYSCLK->PWRCON.XTL12M_EN = 1; // enable external clock (12MHz)
	SYSCLK->CLKSEL0.HCLK_S = 0;	  // select external clock (12MHz)
	LOCKREG();
	Initial_panel();  
	clr_all_panel();	                        
	DrvADC_Open(ADC_SINGLE_END, ADC_SINGLE_CYCLE_OP, 0x03, INTERNAL_HCLK, 1); // ADC1 & ADC0	
  	DrvGPIO_Open(E_GPB, 0, E_IO_INPUT); // SW	
	DrvGPIO_Open(E_GPB, 11, E_IO_OUTPUT); // initial GPIO pin GPB11 for controlling Buzzer	 
	sprintf(TEXT1,"   Snake Game");
	print_lcd(0, TEXT1);
	sprintf(TEXT1,"       by   ");
	print_lcd(1, TEXT1); 
	sprintf(TEXT1,"  David & Muli");
	print_lcd(2, TEXT1);
	DrvSYS_Delay(650000);
	while(1) 
 	{ 
	 	device = Scankey();
	 	if(device)
	 	{
			clr_all_panel();
			sprintf(TEXT1," Keypad detected");
			print_lcd(0, TEXT1);
			DrvSYS_Delay(650000);
			device=0;
			break;
		} 		
		if( bt_command=='2'| bt_command=='4'| bt_command=='6'| bt_command=='8'| bt_command=='5')
		{
			clr_all_panel();
			sprintf(TEXT1," Bluetooth");
			print_lcd(0, TEXT1);
			sprintf(TEXT1," detected");
			print_lcd(1, TEXT1);
			DrvSYS_Delay(650000);
			device=2;
      			bt_command=0;			
			break;
		}	
		DrvADC_StartConvert();                   // start A/D conversion
    		while(DrvADC_IsConversionDone()==FALSE); // wait till conversion is done
		Vx = ADC->ADDR[0].RSLT & 0xFFF;
		Vy = ADC->ADDR[1].RSLT & 0xFFF;
		SW = DrvGPIO_GetBit(E_GPB,0);
		if(SW==0 & Vx!=0 )
		{
			clr_all_panel();
			sprintf(TEXT1," Joystick detected");
			print_lcd(0, TEXT1);
			DrvSYS_Delay(650000);
			device=1;	
			break;
		}
 	}
  	state=0;
	while(speed==-1) 
 	{
		num=0;
	 	if (state==0)
	 	{
	 		clr_all_panel();
			sprintf(TEXT1,"    Speed");
			print_lcd(0, TEXT1);
	 		sprintf(TEXT1,"Slow <==");
			print_lcd(1, TEXT1);
	 		sprintf(TEXT1,"Medium");
			print_lcd(2, TEXT1);
	 		sprintf(TEXT1,"Fast");
			print_lcd(3, TEXT1); 
	 	}
		 else if(state==1)
	 	{
			clr_all_panel();
			sprintf(TEXT1,"    Speed");
			print_lcd(0, TEXT1);
	 		sprintf(TEXT1,"Slow ");
			print_lcd(1, TEXT1);
	 		sprintf(TEXT1,"Medium <==");
			print_lcd(2, TEXT1);
	 		sprintf(TEXT1,"Fast");
			print_lcd(3, TEXT1);  
	 	}
		 else if(state==2)
	 	{
			clr_all_panel();
			sprintf(TEXT1,"    Speed");
			print_lcd(0, TEXT1);
	 		sprintf(TEXT1,"Slow ");
			print_lcd(1, TEXT1);
	 		sprintf(TEXT1,"Medium ");
			print_lcd(2, TEXT1);
	 		sprintf(TEXT1,"Fast <==");
			print_lcd(3, TEXT1); 	 
	 	} 
	 	DrvSYS_Delay(100000);
	 	if(device==0)
	  	{
			while(num!=2 & num!=8 & num!=5)
			{
	     		 num=Scankey();
		   	}
			switch(num)
      			{
				case 2 : //up
			 	{
					if(state==0)
						state=2;
					else if(state==1) 
						state=0;
					else if(state==2) 
						state=1;				
			 	}
					break;
				case 8 : //down	
			 	{
					if(state==0) 
						state=1;
					else if(state==1) 
						state=2;
					else if(state==2) 
						state=0;				
			 	}
			 		break;
				case 5 : //choose
			 	{
					if(state==0) 
						speed=0;
					else if(state==1) 
						speed=1;
					else if(state==2) 
						speed=2;				
			 	}
					break;	
			 	default:
					break;;
			}
		}
		if(device==1)
		{
			SW=1;
			Vy=1000;	
			while(SW!=0 & Vy>=100 & Vy<=4000)	
			{
				DrvADC_StartConvert();                   // start A/D conversion
				while(DrvADC_IsConversionDone()==FALSE); // wait till conversion is done
				Vy = ADC->ADDR[1].RSLT & 0xFFF;
				SW = DrvGPIO_GetBit(E_GPB,0);
			}	
			if(Vy<100)	//up
		 	{
				if(state==0)
					state=2;
				else if(state==1) 
					state=0;
				else if(state==2) 
					state=1;	
		 	}
			if(Vy>4000)	//down
	  		{
				if(state==0)
					state=1;
				else if(state==1) 
					state=2;
				else if(state==2) 
					state=0;	
		 	}
			if(SW==0) //choose
		 	{
				if(state==0)
					speed=0;
				else if(state==1) 
					speed=1;
				else if(state==2) 
					speed=2;	
		 	}
		}
		if(device==2)
	   	{
	      		bt_command=0;
		    	while(bt_command!='2' & bt_command!='8' & bt_command!='5')
        		{}
	 		switch(bt_command)
      			{
				case '2' : //up
			 	{
					if(state==0) 
						state=2;
					else if(state==1) 
						state=0;
					else if(state==2) 
						state=1;				
			 	}
					break;
				case '8' : //down	
			 	{
				 	if(state==0) 
				 		state=1;
				 	else if(state==1) 
						state=2;
				 	else if(state==2) 
						state=0;				
			 	}
			 		break;
				case '5' : //choose
			 	{
					if(state==0) 
						speed=0;
					else if(state==1) 
						speed=1;
					else if(state==2) 
						speed=2;				
			 	}
			 		break;	
				default:
					break;
			}
			bt_command=0;
		}
 	}
	state=0;
	while(no_walls==-1) 
 	{
		num=0;
	 	if (state==0)
	 	{
	 		clr_all_panel();
			sprintf(TEXT1,"    Walls");
			print_lcd(0, TEXT1);
	 		sprintf(TEXT1,"Yes <==");
			print_lcd(1, TEXT1);
	 		sprintf(TEXT1,"No");
			print_lcd(2, TEXT1); 
	 	}
	 	else if(state==1)
	 	{
			clr_all_panel();
			sprintf(TEXT1,"    Walls");
			print_lcd(0, TEXT1);
	 		sprintf(TEXT1,"Yes ");
			print_lcd(1, TEXT1);
	 		sprintf(TEXT1,"No <==");
			print_lcd(2, TEXT1);
  
	 	}
	 	DrvSYS_Delay(100000);
	 	if(device==0)
	 	{
		 	while(num!=2 & num!=8 & num!=5)
			{
	      			num=Scankey();
			}
	 		switch(num)
      			{
				case 2 : //up
			 	{
					if(state==0) 
						state=1;
					else if(state==1) 
						state=0;				
			 	}
			 		break;
				case 8 : //down	
			 	{
					if(state==0) 
						state=1;
					else if(state==1) 
						state=0;				
			 	}
			 		break;
				case 5 : //choose
			 	{
					if(state==0) 
						no_walls=0;
					else if(state==1) 
						no_walls=1;			
			 	}
			 		break;	
				default:
					break;;
			}
		}
		if(device==1)
		{
			SW=1;
			Vy=1000;	
			while(SW!=0 & Vy>=100 & Vy<=4000)	
			{
				DrvADC_StartConvert();                   // start A/D conversion
    				while(DrvADC_IsConversionDone()==FALSE); // wait till conversion is done
				Vy = ADC->ADDR[1].RSLT & 0xFFF;
				SW = DrvGPIO_GetBit(E_GPB,0);
			}
			if(Vy<100)	//up
			{
				if(state==0) 
					state=1;
				else if(state==1) 
					state=0;
			}
			if(Vy>4000)	//down
			{
				if(state==0) 
					state=1;
				else if(state==1) 
					state=0;
			}
			if(SW==0) //choose
		 	{
				if(state==0) 
					no_walls=0;
				else if(state==1) 
					no_walls=1;		
		 	}
		}
		if(device==2)
	   	{
	      		bt_command=0;
		    	while(bt_command!='2' & bt_command!='8' & bt_command!='5')
        		{}
	 		switch(bt_command)
      			{
				case '2' : //up
			 	{
					if(state==0) 
						state=1;
					else if(state==1) 
						state=0;				
			 	}
				 	break;
				case '8' : //down	
			 	{
					if(state==0) 
						state=1;
					else if(state==1) 
						state=0;				
			 	}
			 		break;
				case '5' : //choose
			 	{
					if(state==0) 
						no_walls=0;
					else if(state==1) 
						no_walls=1;			
			 	}
			 		break;	
				default:
					break;;
			}
			bt_command=0;
		}
	}
	state=0;
	while(obstacle_on==-1) 
	{
		num=0;
		if (state==0)
		{
			clr_all_panel();
			sprintf(TEXT1,"    Obstacle");
			print_lcd(0, TEXT1);
			sprintf(TEXT1,"Off <==");
			print_lcd(1, TEXT1);
			sprintf(TEXT1,"On-static");
			print_lcd(2, TEXT1);
			sprintf(TEXT1,"On-dynamic");
			print_lcd(3, TEXT1);	 
		}
		else if(state==1)
		{
			clr_all_panel();
			sprintf(TEXT1,"    Obstacle");
			print_lcd(0, TEXT1);
			sprintf(TEXT1,"Off ");
			print_lcd(1, TEXT1);
			sprintf(TEXT1,"On-static <==");
			print_lcd(2, TEXT1);
			sprintf(TEXT1,"On-dynamic");
			print_lcd(3, TEXT1); 	 
		}
		else if(state==2)
		{
			clr_all_panel();
			sprintf(TEXT1,"    Obstacle");
			print_lcd(0, TEXT1);
			sprintf(TEXT1,"Off ");
			print_lcd(1, TEXT1);
			sprintf(TEXT1,"On-static ");
			print_lcd(2, TEXT1);
			sprintf(TEXT1,"On-dynamic <==");
			print_lcd(3, TEXT1); 	 
		} 
		DrvSYS_Delay(100000); 
		if(device==0)
		{
			while(num!=2 & num!=8 & num!=5)
			{
				num=Scankey();
			}
			switch(num)
      			{
				case 2 : //up
				{
					if(state==0)
						state=2;
					else if(state==1) 
						state=0;
					else if(state==2) 
						state=1;				
				}
			 		break;	
				case 8 : //down	
				{
					if(state==0) 
						state=1;
					else if(state==1) 
						state=2;
					else if(state==2) 
						state=0;				
				}
					break;		
				case 5 : //choose
				{
					if(state==0) 
						obstacle_on=0;
					else if(state==1) 
						obstacle_on=1;
					else if(state==2) 
						obstacle_on=2;				
				}
			 		break;	
				default:
					break;;
			}
		}
		if(device==1)
		{
			SW=1;
			Vy=1000;	
			while(SW!=0 & Vy>=100 & Vy<=4000)	
			{
				DrvADC_StartConvert();                   // start A/D conversion
    				while(DrvADC_IsConversionDone()==FALSE); // wait till conversion is done
				Vy = ADC->ADDR[1].RSLT & 0xFFF;
				SW = DrvGPIO_GetBit(E_GPB,0);
			}
			if(Vy<100)	//up
			{
				if(state==0) 
			 		state=2;
				else if(state==1) 
					state=0;
				else if(state==2) 
					state=1;	
			}
			if(Vy>4000)	//down
		 	{
				 if(state==0) 
					state=1;
				else if(state==1) 
					state=2;
				else if(state==2) 
					state=0;	
	  		}
			if(SW==0) //choose
			{
				if(state==0) 
					obstacle_on=0;
				else if(state==1) 
					obstacle_on=1;
				else if(state==2) 
					obstacle_on=2;	
		 	}
		}
		if(device==2)
	   	{
	      		bt_command=0;
		    	while(bt_command!='2' & bt_command!='8' & bt_command!='5')
        		{}
	 		switch(bt_command)
      			{
				case '2' : //up
			 	{
					if(state==0) 
						state=2;
					else if(state==1) 
						state=0;
					else if(state==2) 
						state=1;				
			 	}
			 		break;
				case '8' : //down	
			 	{
					if(state==0) 
						state=1;
					else if(state==1) 
						state=2;
					else if(state==2) 
						state=0;				
			 	}
					break;
				
				case '5' : //choose
			 	{
					if(state==0) 
						obstacle_on=0;
					else if(state==1) 
						obstacle_on=1;
					else if(state==2) 
						obstacle_on=2;				
			 	}
			 		break;	
				default:
					break;
			}
			bt_command=0;
		}
 	}
 	clr_all_panel();
	sprintf(TEXT1,"  You choose ");
	print_lcd(0, TEXT1);
      	if (no_walls==0)
	{
	 	sprintf(TEXT1,"*With Walls* ");
		print_lcd(1, TEXT1);
	}
	if (no_walls==1)
	{
	 	sprintf(TEXT1,"*No Walls* ");
		print_lcd(1, TEXT1);
	}
	if (speed==0)
	{
	 	sprintf(TEXT1,"*Slow Speed* ");
		print_lcd(2, TEXT1);
	}
	if (speed==1)
	{
	 	sprintf(TEXT1,"*Medium Speed* ");
		print_lcd(2, TEXT1);
	}
	if (speed==2)
	{
	 	sprintf(TEXT1,"*Fast Speed* ");
		print_lcd(2, TEXT1); 
	}
	if (obstacle_on==0)
	{
	 	sprintf(TEXT1,"*Obstacle-off* ");
		print_lcd(3, TEXT1); 
	}
	if (obstacle_on==1)
	{
	 	sprintf(TEXT1,"*Obstacle-static* ");
		print_lcd(3, TEXT1); 
	}
	if (obstacle_on==2)
	{
	 	sprintf(TEXT1,"*Obstacle-dynamic* ");
		print_lcd(3, TEXT1); 
	}
	DrvSYS_Delay(650000);	
	DrvSYS_Delay(450000);						
 	while(1) 
 	{
		initList_snake(&snake_list);
		a[num_part] = allocItem_snake(2,58,num_part);//initial position ,direction right
		insertFirst_part(&snake_list, &a[num_part]);
		RectangleFill( a[num_part].x0,a[num_part].y0,(a[num_part].x0)+pixel_size,(a[num_part].y0)+pixel_size,1)	; 
		num_part++;
		a[num_part] = allocItem_snake(2+pixel_size,58,num_part);
		insertFirst_part(&snake_list, &a[num_part]);
		RectangleFill( a[num_part].x0,a[num_part].y0,(a[num_part].x0)+pixel_size,(a[num_part].y0)+pixel_size,1)	; 	
	  	current_food=	new_food(&snake_list,random_seed,obstacle_on);
	  	obstacle1.x0=28;
	  	obstacle1.y0=12;
	  	obstacle1.direction=6;
	 	obstacle2.x0=90;
	  	obstacle2.y0=12;
	  	obstacle2.direction=0;
	 	obstacle3.x0=28;
	  	obstacle3.y0=42;
	  	obstacle3.direction=0;
	 	obstacle4.x0=90;
	  	obstacle4.y0=42;
	  	obstacle4.direction=4;
		obstacle5.x0=59;
	  	obstacle5.y0=27;
	  	obstacle5.direction=0;	 
    		while(1)
		{
			if(obstacle_on==1)
			{
				RectangleDraw(obstacle2.x0,obstacle2.y0,obstacle2.x0+10,obstacle2.y0+10,1);		
	    			RectangleDraw(obstacle3.x0,obstacle3.y0,obstacle3.x0+10,obstacle3.y0+10,1);
				RectangleDraw(obstacle5.x0,obstacle5.y0,obstacle5.x0+10,obstacle5.y0+10,1);
			}
			if(obstacle_on==1|obstacle_on==2)
			{
		  		RectangleDraw(obstacle1.x0,obstacle1.y0,obstacle1.x0+10,obstacle1.y0+10,1);
	   			RectangleDraw(obstacle4.x0,obstacle4.y0,obstacle4.x0+10,obstacle4.y0+10,1);
			}	
			if (device==0)
		 	{
				num = Scankey();
				if(num)
			 		direction=num;
	   		}	
			if (device==1)
		 	{
				DrvADC_StartConvert();                   // start A/D conversion	
				while(DrvADC_IsConversionDone()==FALSE); // wait till conversion is done
				Vx = ADC->ADDR[0].RSLT & 0xFFF;
				Vy = ADC->ADDR[1].RSLT & 0xFFF;
				SW = DrvGPIO_GetBit(E_GPB,0);
				if(Vy<100) //up
					direction=2;	
				if(Vx>200) //right
					direction=6;
				if(Vy>4000) //down
					direction=8;
				if(Vx<10) //left
					direction=4;  
	   		}
			if (device==2)
			{
				direction= bt_command-'0';
				bt_command=0;
			}
			switch (direction)
      			{
				case 2 : //up
				{
					if(last_direction==8)
						wall_collision=move(&snake_list,a,8);
					else
					{
						last_direction=2;
				   		wall_collision= move(&snake_list,a,2);
					}
				}
					break;
				case 6 : //right	
				{
					if(last_direction==4)
						wall_collision= move(&snake_list,a,4);
					else
					{
						last_direction=6;
				   		wall_collision= move(&snake_list,a,6);	
					}
				}					
					break;
				case 8 : //down
				{
					if(last_direction==2)
						wall_collision=	move(&snake_list,a,2);
					else
					{
						last_direction=8;
						   wall_collision=move(&snake_list,a,8);	
					}
				}		
					break;
				case 4 : //left
				{
					if(last_direction==6)
						wall_collision= move(&snake_list,a,6);
					else
					{
						last_direction=4;
				   		wall_collision=	move(&snake_list,a,4);	
					}
				}						
					break;
				default:
					wall_collision=move(&snake_list,a,last_direction);				
		  	}		
			switch (speed)
      			{
				case 0 : //easy
				{
					if(obstacle_on)	
						DrvSYS_Delay(6000);         // adjustable delay for vision
					DrvSYS_Delay(20000);         // adjustable delay for vision	
				}	
					break;
				case 1 : //medium	
				{
					if(obstacle_on)	
						DrvSYS_Delay(2000);         // adjustable delay for vision
					DrvSYS_Delay(6000);         // adjustable delay for vision	
				}	
					break;
				case 2 : //hard
					break;
				default:
        				break;					
		  	}			
			RectangleFill(current_food.x0,current_food.y0,(current_food.x0)+pixel_size,(current_food.y0)+pixel_size,1)	;	
			if(obstacle_on==1|obstacle_on==2)	
		  		obstacle_collision = move_obstacle(obstacle_on,&obstacle1,&obstacle2,&obstacle3,&obstacle4,&obstacle5,&snake_list);		
			if( ( (wall_collision | check_self_collision(&snake_list)| obstacle_collision)  & !no_walls ) | ( (check_self_collision(&snake_list)|obstacle_collision) & no_walls ) )
	   		{
				clr_all_panel();                  // clear LCD display
				DrvGPIO_ClrBit(E_GPB,11); // GPB11 = 0 to turn on Buzzer
				DrvSYS_Delay(5000);	    // Delay 
				DrvGPIO_SetBit(E_GPB,11); // GPB11 = 1 to turn off Buzzer	
				DrvSYS_Delay(2000);	    // Delay 
				DrvGPIO_ClrBit(E_GPB,11); // GPB11 = 0 to turn on Buzzer
				DrvGPIO_SetBit(E_GPA,12);
				DrvGPIO_SetBit(E_GPA,13);
				DrvGPIO_ClrBit(E_GPA,14); // Red   LED on
				DrvSYS_Delay(5000);	    // Delay 
				DrvGPIO_SetBit(E_GPB,11); // GPB11 = 1 to turn off Buzzer	
				DrvGPIO_SetBit(E_GPA,12);
				DrvGPIO_SetBit(E_GPA,13);
				DrvGPIO_SetBit(E_GPA,14);
				DrvGPIO_SetBit(E_GPB,11); // GPB11 = 1 to turn off Buzzer	
				DrvSYS_Delay(2000);	    // Delay 
				sprintf(TEXT1,"your score is:%d",score);
				print_lcd(2, TEXT1);
				print_lcd(0, " **GAME OVER** "); // Line 0 display
				num_part=0;
				del_snake=1;
				while(del_snake)
				{
					del_snake=deleteLast_part(&snake_list);
				}
				direction=6; //2-up,6-right,8-down,4-left
				last_direction=6;
				RectangleFill(0,0,128,128,0)	;	
				initList_snake(&snake_list);
				a[num_part] = allocItem_snake(2,58,num_part);//initial position ,direction right
				insertFirst_part(&snake_list, &a[num_part]);
				RectangleFill( a[num_part].x0,a[num_part].y0,(a[num_part].x0)+pixel_size,(a[num_part].y0)+pixel_size,1)	; 
				num_part++;
				a[num_part] = allocItem_snake(2+pixel_size,58,num_part);
				insertFirst_part(&snake_list, &a[num_part]);
				RectangleFill( a[num_part].x0,a[num_part].y0,(a[num_part].x0)+pixel_size,(a[num_part].y0)+pixel_size,1)	; 
				
				current_food=new_food(&snake_list,random_seed,obstacle_on);
				score1=score;
				score=0;
				num=0;
				SW=-1;
				bt_command=0;
				while(1)
				{
				 	if(device==0) 
				 	{
				 		num = Scankey();
					 	if(num==5)
							break;
				 	}
				if(device==1)
				{
					DrvADC_StartConvert();                   // start A/D conversion
				   	while(DrvADC_IsConversionDone()==FALSE); // wait till conversion is done
				   	SW = DrvGPIO_GetBit(E_GPB,0);
					if(SW==0)
						break;
				}
				if(device==2)
				{
				 	if(bt_command=='5')
						break;
				}
        			seg_display(score1);				 
			}
		}	
  		else if(no_walls & wall_collision )
		no_walls_func(&snake_list,a) ;		
  		yummy= search_part(&current_food,&snake_list);
		if (yummy)
		{
			DrvGPIO_ClrBit(E_GPB,11); // GPB11 = 0 to turn on Buzzer
			DrvGPIO_SetBit(E_GPA,12);
			DrvGPIO_ClrBit(E_GPA,13); // Green LED on
			DrvGPIO_SetBit(E_GPA,14);
			DrvSYS_Delay(2000);	    // Delay 
			DrvGPIO_SetBit(E_GPB,11); // GPB11 = 1 to turn off Buzzer	
			DrvGPIO_SetBit(E_GPA,12);
	  		DrvGPIO_SetBit(E_GPA,13);
	  		DrvGPIO_SetBit(E_GPA,14);
    			DrvSYS_Delay(2000);	    // Delay 
			score++;
			num_part= length_snake(&snake_list);
	  		a[num_part] = allocItem_snake(current_food.x0,current_food.y0,num_part);
			insertFirst_part(&snake_list, &a[num_part] );
	  		current_food=new_food(&snake_list,random_seed,obstacle_on);
		}
		seg_display(score);
		random_seed++;
 	}				    		
}
}
	   
		
		 		
		
