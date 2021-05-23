// INCLUSÃO DE BIBLIOTECAS
#include <A2a.h>
#include "config.h"

// DEFINIÇÕES DE PINOS

#define pinSensor 5

// DEFINIÇÕES
#define endereco 0x08
#define tempoAtualizacao 10000
#define tempoConfirmacao 500


#define LIVRE 1
#define OCUPADO 0


// INSTANCIANDO OBJETOS
;
AdafruitIO_Feed *controle = io.feed("controle");
AdafruitIO_Feed *vaga = io.feed("vaga");

A2a arduinoSlave;

// DECLARAÇÃO DE FUNÇÕES
void configuraSlave();
void configuraMQTT();
bool monitoraVaga(byte pin);

// DECLARAÇÃO DE VARIÁVEIS
unsigned long controleTempo = 0;

bool comandoRecebido = false;
bool estadoVaga = OCUPADO;

void setup() {
  Serial.begin(9600);
  while (! Serial);

  configuraMQTT();

  arduinoSlave.begin(0, 2);
  configuraSlave();

  Serial.println("Conferindo posição do portão");
  
  portao->get();
  io.run();
  
  if(estadoPortao == FECHADO){
    if(arduinoSlave.digitalWireRead(endereco, pinPortao1)){
      fechaPortao();      
    }
  }else{
    if(arduinoSlave.digitalWireRead(endereco, pinPortao2)){
      abrePortao();      
    }
  }
  
  Serial.println("Portão posicionado");
  
  Serial.println("Fim Setup");
}

void loop() {
  io.run();

  //Monitora o sensor da vaga
  if (millis() > controleTempo + tempoAtualizacao) {
    if (monitoraVaga(pinSensor)) {
      controleTempo = millis();
      vaga->save(estadoVaga);
    }
  }

 

// IMPLEMENTO DE FUNÇÕES
void controleMQTT(AdafruitIO_Data *data) {
  Serial.print("Controle Recebido <- ");
  Serial.println(data->value());
  comandoRecebido = true;
}

void portaoMQTT(AdafruitIO_Data *estado) {
  Serial.print("Portao Recebido <- ");
  estadoPortao = estado->toBool();
  Serial.println(estado->value());
}

bool monitoraVaga(byte pin) {
  static bool leituraAnt;
  static bool mediaLeitura[5];

  bool leitura = arduinoSlave.digitalWireRead(endereco, pin);
  estadoVaga = leitura;

  if (leitura != leituraAnt) {

    //Faz cinco leituras para filtrar interferencias
    for ( byte i = 0; i < 5 ; i++) {
      mediaLeitura[i] = arduinoSlave.digitalWireRead(endereco, pin);
      delay(tempoConfirmacao);
    }

    byte qtdAnterior = 0;
    byte qtdAtual = 0;

    for ( byte i = 0; i < 5 ; i++) {
      if (mediaLeitura[i] == leituraAnt) {
        qtdAnterior++;
      }
      else {
        qtdAtual++;
      }

    }

    if ( qtdAtual > qtdAnterior) {
      leituraAnt = leitura;
      return 1 ;
    } else {
      return 0;
    }

  } else {
    return 0;
  }

}



void configuraSlave() {
  Serial.println("configurando pinMode do Arduino");
  arduinoSlave.pinWireMode(endereco, pinSensor, INPUT);
  Serial.println("Slave Configurado!");
}

void configuraMQTT() {
  Serial.print("Conectando ao Adafruit IO");
  io.connect();

  controle->onMessage(controleMQTT);
  portao->onMessage(portaoMQTT);

  while (io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.println(io.statusText());
}
