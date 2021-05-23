#include <A2a.h>
#define endereco 0x08

A2a arduinoMaster;

void setup() {
  arduinoMaster.begin(endereco);
  arduinoMaster.onReceive(receberDados);
  arduinoMaster.onRequest(enviarDados);
}

void loop() {

}

void receberDados() {
  arduinoMaster.receiveData(); 
}

void enviarDados() {
  arduinoMaster.sendData(); 
}
