Adaptador Teclado PS2 a USB con un Digispark (ATTiny85)

La idea del adaptador de teclado viene de este circuito, las conexiones son las mismas que para el Digispark.
![](http://mario.mtechcreations.com/wp-content/uploads/2013/11/PS2-USB-Trinket-Wireing.jpg)

Teclado        a          Digispark.




Rojo       >>>>>>>>>>       5v Arduino.

Negro      >>>>>>>>>>       GND

Blanco     >>>>>>>>>>       P0    Pin 0 → I2C SDA, PWM (LED on Model B)

Amarillo   >>>>>>>>>>       P2    Pin 2 → I2C SCK, Analog In


 Pin outs:

    All pins can be used as Digital I/O
    Pin 0 → I2C SDA, PWM (LED on Model B)
    Pin 1 → PWM (LED on Model A)
    Pin 2 → I2C SCK, Analog In
    Pin 3 → Analog In (also used for USB+ when USB is in use)
    Pin 4 → PWM, Analog (also used for USB- when USB is in use)
    Pin 5 → Analog In


https://es.wikipedia.org/wiki/PS/2

Pin 1 	+DATA 	Datos salida
Pin 2 	Reservado 	Reservadoa
Pin 3 	GND 	Masa
Pin 4 	VCC 	+5 V CC a 100 mA
Pin 5 	+CLK 	Reloj salida
Pin 6 	Reservado 	Reservadob


Los LED de estado del teclado no funcionan ,sí alguien es capaz de ayudarme con eso, estaría genial.
