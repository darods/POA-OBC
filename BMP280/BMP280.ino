//VERSION 1.02
//hola que hace
//LIBRERIAS :
   //BMP 280
#include<Wire.h>

#include <Adafruit_BMP280.h>
#include <Adafruit_Sensor.h>


//VARIABLES GLOBALES :

float valores_n = 0;  //Es la altura que mide el sensor en un instante k, valores sin refinar
float altura_ref = 0; //se hace una medicion y se pone como valor negativo, de tal manera que la diferencia entre los futuros datos
                      // y la altura actual sea la correcta, si no se mueve se mantiene en cero

float valores_s = 0; //Son los valores corregidos
float altura_maxima = 0;
 
float presion=560; //la presion local, bogota esta fucking arriba
Adafruit_BMP280 bme;//I2C


//FUNCIONES A USAR:
void analizar_altura();//amplia la señal inicial para que las opperaciones sean mas sencillas
void tierra();//corrige los datos negativos igualandolos a cero
float correccion();//devuelve los datos corregidos, util para hacer operaciones

void setup(){
  Serial.begin(9600);//Es lo que inicia el BMP280
  if(!bme.begin()){
    Serial.println("error");
  }//fin de la iniciacion del sensor
  
  delay(100);//una pequeña espera para darle tiempo de iniciar
  altura_ref=bme.readAltitude(presion);//la diferencia para que el valor inicial sea cero
  

}

void loop(){
  if(!bme.begin()){
    Serial.print("error");
    while(!bme.begin());
  }//fin de la iniciacion del sensor
  
  analizar_altura();//toma el dato de la altura actual
  tierra(); //verifica que el dato no sea menor que cero
  correccion();//devuelve los datos corregidos
  
  if(correccion()>altura_maxima){
    altura_maxima = correccion();
  }



  
     
  Serial.println(valores_s); //imprimimos en pantalla las mediciones corregidas  
  delay(8);//Es lo maximo que puedo usar como tiempo de muestreo, ademas tambien es mas sencillo que el P del PID y da los mismos resultados
}

void analizar_altura(){//amplia la señal inicial para que las opperaciones sean mas sencillas
   valores_n= int((bme.readAltitude(presion)-altura_ref)*100);
}

void tierra(){ //la gracia de la funcion es que las mediciones, por logica
  if(valores_n<0){//no pueden ser menores que cero, esto es porque la referencia, osea el suelo
      valores_n = 0;//siempre sera el cero desde donde se lanza el cohete, no puede ir mas abajo
    }
}

float correccion(){ //Es una funcion que devuelve los datos corregidos, bestante util para poder hacer operaciones lugeo
  int pasado_1 = valores_n;//crea una variable que guarda la altura inmediatamente anterior
  int correcto_anterior = 0;
  
  if(valores_n-pasado_1>30){//si la diferencia entre la altura actual y la anterior es superior a 30
      valores_s = (valores_n+pasado_1)/2;//se promedia para equilibrar los datos
      valores_s = correcto_anterior+ (0.1)*(pasado_1-correcto_anterior);
      correcto_anterior = valores_s;
  }else{//si no lo es, los valores correctos corresponderan a los valores medidos
      valores_s = correcto_anterior+ (0.1)*(pasado_1-correcto_anterior);
      correcto_anterior = valores_s;
    }
}

