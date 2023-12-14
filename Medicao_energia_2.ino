#include <Wire.h>
#include <TimeLib.h>
#include <DS1307.h>
#include <SPI.h>
#include <Ethernet.h>
#include "EmonLib.h"
#include <LiquidCrystal.h> 
 
EnergyMonitor SCT013;

LiquidCrystal lcd(9, 8, 7, 6, 5, 3);//Definições do display

int pinSCT = A1;   
int pinTensao = A2;


unsigned long tempo_corr;
unsigned long tempo_ddp;
unsigned long defasagem;
unsigned long defasagem_medida;

unsigned long tempo_energia_eletrica_inicio;
unsigned long tempo_energia_eletrica_fim;
unsigned long tempo_total_energia_eletrica;

float Energia_eletrica_consumida;
float Energia_eletrica_consumida_acumulada;
float Energia_eletrica_relogio;

float angulo;
int Valor_analogico_Lido;
int valor_analogico_ddp;
int valor_analogico_ddp_maior;
int tensao_eficaz;
float corrente_eficaz;

int contagem = 0;
float fator_de_potencia;
int Potencia_ativa_instantanea;
int Potencia_ativa_media;
float consumo = 0;
float tarifa = 0.78651;
float energia_eletrica_tarifa;

int segundo=0;     
int minuto=0;      
int hora=0;       
int dia=0;         

//DS1307 rtc(A4, A5);

void setup() {
  
  SCT013.current(pinSCT, 6.0606);
  Serial.begin(9600);
  tempo_energia_eletrica_inicio = millis();
  lcd.begin(16,4);  

}


void loop() {

  lcd.setCursor(0,0);
 
   Valor_analogico_Lido = analogRead(pinTensao);
  
  if(Valor_analogico_Lido>560 && Valor_analogico_Lido<570) {
  tempo_ddp = micros();
  
   Valor_analogico_Lido = analogRead(pinSCT);// 112 us 
    
  while(Valor_analogico_Lido<510 || Valor_analogico_Lido>530){
   Valor_analogico_Lido = analogRead(pinSCT);   
   }
  
  tempo_corr = micros(); 
  
  defasagem_medida = tempo_corr - tempo_ddp - 224;// O tempo de 224 microsegundos é o tempo que demora para a leitura da condicional do while
  if(defasagem_medida<=3000 && defasagem_medida >224 ) {
    defasagem= defasagem_medida;
  }
  


  angulo = map(defasagem,0,4166,0,90); 

  fator_de_potencia = cos(3.14*(angulo/180)); // transforma o angulo em radiano.  

   valor_analogico_ddp_maior = analogRead(A2);
   for(int i=0;i<1000;i++){
   valor_analogico_ddp = analogRead(A2);

    if(valor_analogico_ddp>valor_analogico_ddp_maior){
      valor_analogico_ddp_maior = valor_analogico_ddp;  
    }
     
   }

    tensao_eficaz = valor_analogico_ddp_maior*0.1963; 
    if(tensao_eficaz<3){tensao_eficaz=0;}
    
    lcd.setCursor(0,1);  //Seta o cursor na segunda linha
     lcd.print("T:");     // Mostra o valor da tensaão   
     lcd.print(tensao_eficaz);
     lcd.print("V");

double corrente_eficaz = SCT013.calcIrms(1480);   // Calcula o valor da Corrente
    if(corrente_eficaz<0.03){corrente_eficaz=0;}
    
    lcd.setCursor(9,0);  // Mostra o valor da corrente 
     lcd.print("I:");
     lcd.print(corrente_eficaz);
     lcd.print("A");


// Calcular potência ativa media

Potencia_ativa_instantanea = tensao_eficaz*corrente_eficaz*fator_de_potencia;

if(Potencia_ativa_instantanea>0){
  if(Potencia_ativa_media==0){
    Potencia_ativa_media = Potencia_ativa_instantanea;
  }
  Potencia_ativa_media = (Potencia_ativa_instantanea+Potencia_ativa_media)/2;
}
if(Potencia_ativa_media<5){Potencia_ativa_media=0;}

if(corrente_eficaz<0.009){Potencia_ativa_media=0;}


    lcd.setCursor(0,0);   //seta o cursor na primeira linha e primeira coluna
     lcd.print("P:");     //Mostra o valor da potencia 
     lcd.print(Potencia_ativa_media);
     lcd.print(" W");

      contagem = contagem +1;

  }
  

// Calcula a Energia Eletrica consumida no periodo  
      if(contagem >=5){   // Linha que serve para definir o tamanho da amostra
       
      tempo_energia_eletrica_fim = millis();  // marca o fim do periodo de medicao
      
      contagem = 0;
      
      tempo_total_energia_eletrica = tempo_energia_eletrica_fim - tempo_energia_eletrica_inicio;
      
      tempo_energia_eletrica_inicio = millis(); // marca o inicio do proximo ciclo para nao perder a referencia do tempo

      tempo_total_energia_eletrica = (tempo_total_energia_eletrica/3600); // transforma de milissegundos para mili horas (10^-3 h)
      
      Energia_eletrica_consumida = Potencia_ativa_media*tempo_total_energia_eletrica;
    
      
      Potencia_ativa_media = 0;

      Energia_eletrica_consumida_acumulada = Energia_eletrica_consumida + Energia_eletrica_consumida_acumulada;

      
      if(Energia_eletrica_consumida_acumulada<10000){
      Energia_eletrica_relogio = Energia_eletrica_consumida_acumulada;  
      
      lcd.setCursor(0,2);
      lcd.print("E:");
     lcd.print(Energia_eletrica_relogio,2);
     lcd.print(" mWh");
      }

      if(Energia_eletrica_consumida_acumulada>=10000 && Energia_eletrica_consumida_acumulada <10000000){
      Energia_eletrica_relogio =  Energia_eletrica_consumida/1000; 
      lcd.setCursor(0,2);
      lcd.print("E:");
     lcd.print(Energia_eletrica_relogio,2);
     lcd.print(" Wh");
      }


      if(Energia_eletrica_consumida_acumulada>=10000000){
      Energia_eletrica_relogio =  Energia_eletrica_consumida/1000000; 
      lcd.setCursor(0,2);
      lcd.print("E:");
     lcd.print(Energia_eletrica_relogio,2);
     lcd.print(" KWh");
     lcd.print("    ");
      }

      energia_eletrica_tarifa = Energia_eletrica_consumida/1000000;
      lcd.setCursor(0,3);
      consumo = (energia_eletrica_tarifa*tarifa)+(energia_eletrica_tarifa*0.172648)+(energia_eletrica_tarifa*0.005146)+(energia_eletrica_tarifa*0.024325);
      lcd.print("R$: ");
      lcd.print(consumo,12);
  
  }


     if(hora<10){lcd.setCursor(8,1);lcd.print("0");} // Se a hora for menor que 10, coloca um 0 na frente
     lcd.setCursor(9,1);
     lcd.print(hora);
     lcd.print(":");
     if(minuto<10){lcd.setCursor(11,1);lcd.print("0");}// Se a o minuto for menor que 10, coloca um 0 na frente
     lcd.print(minuto);
     lcd.print(":");
     if(segundo<10){lcd.setCursor(14,1);lcd.print("0");} // Se o segundo for menor que 10, coloca um 0 na frente
     lcd.print(segundo);

    segundo=segundo+1;
       if (segundo >=60) {segundo = 0; minuto = minuto+1;}
       if (minuto >=60){segundo =0;minuto=0;hora = hora+1;}
       if (hora >23){segundo=0;minuto=0;hora =0; dia+1;}



       delay(1000);
  }

