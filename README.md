# Snake game Embedded project
snake game with Nu-LB-NUC140 Microcontroller

## The code structure
The main file is **snake.c**. 
To run this file you also need this drivers.






![image](https://user-images.githubusercontent.com/68822926/123509870-af4ab980-d680-11eb-9e17-64fab8ffe263.png)
you can download them from here : 

1. KeilMDK : mdk474.exe–ARM/Keilprovide free IDE tool1.Free download from https://www.keil.com/demo/eval/armv4 .Short registration, no need for license
2. Nu-Link Driver : Nu-Link_Keil_Driver.exe–Nuvotonprovide NuLinkICEdevice driver(NuLinkICE is built-in on Nu-LB-NUC140 learning board)–run it to install this NuLinkdevice driver
3. NuvotonBSP : NUC140BSP.zip–Nuvoton’sBasic Sample Package including device drivers and sample codes–Just unzip it  (usually it is put under C:\Nuvoton\)


video of the project:
https://drive.google.com/file/d/1KBBrow_GbDenI1-rcMb9Ai1QAHLm_SGy/view?usp=sharing

## The settings explained

**1. General background**
The Snake game is displayed on the LCD screen of the kit and can be controlled by one of the keyboards integrated in the kit, a joystick that connects to the kit (GPIO) and via the BT app on the smartphone via the UART-BT connector that connects to the kit (GPIO).
In the game you can choose:
1. 3 speeds of the snake
2. With / without collision with walls
3. With / without obstacles and in addition whether the obstacles will be static or dynamic.

After harming or eating food, the buzzer will beep and flash a red / green LED accordingly.
The score is displayed on top of a seven-segment view of the assessment.








![image](https://user-images.githubusercontent.com/68822926/123510069-187efc80-d682-11eb-9ea2-a60d0f6fcafe.png)


Snake vertebrae are realized by a linked list so that they are easier to manage.
The snake progresses according to the algorithm - deleting the tail vertebra and adding a vertebra in the head according to the desired direction.
(At first the logic was to move each vertebra individually, but this method proved ineffective and greatly slowed down the process as the length of the snake increased).
After each progress of the snake the head vertebra is examined for wall injury, self-harm, obstacle injury or eating food and accordingly the next action is determined whether it is the end of the game or adding a vertebra to the snake.

Due to problems using dynamic allocation to the linked list, we had to pre-allocate memory, a list of 51 cells - a higher number caused display problems, probably due to lack of memory.

![image](https://user-images.githubusercontent.com/68822926/123510094-3ba9ac00-d682-11eb-863e-3746d1ba801d.png)
![image](https://user-images.githubusercontent.com/68822926/123510097-3f3d3300-d682-11eb-9711-037b2f0acbca.png)
![image](https://user-images.githubusercontent.com/68822926/123510101-419f8d00-d682-11eb-8226-618ea61ca87b.png)
![image](https://user-images.githubusercontent.com/68822926/123510104-4401e700-d682-11eb-8b2b-7deb8c41974d.png)
![image](https://user-images.githubusercontent.com/68822926/123510106-47956e00-d682-11eb-804a-1e7bee41ef64.png)


