#include <EasyNextionLibrary.h>
#include <trigger.h>

#include <Servo.h> // Biblioteca de controle dos servo-motores
#include <EEPROM.h> // Biblioteca de memória EEPROM


// VARIÁVEIS GLOBAIS
Servo motorEntrada, motorDivisao;
int tempoAmassadorIda = 1700; // Tempo que o motor amassador leva para ir
int tempoAmassadorVolta = 1500; // Tempo que o motor amassador leva para voltar
int vezesAmassador = 1; // Quantas vezes o amassador repete o processo
const int velocidadeGlobal = 1; // Fator de velocidade do processo como um todo
const int pagamentoIFCoins = 50; // Quanto o estudante recebe por material reciclado
const int saldoMaximo = 32256; // Maior número possível para o saldo de IF Coins
const int anguloInicialEntrada = 170, anguloFinalEntrada = 0;
const int anguloPlasticoDivisao = 10, anguloMetalDivisao = 100;
int posDivisao = 0;
// VARIÁVEIS DA TELA NEXTION
int paginaAtual = 0; // Em qual tela a Nextion se encontra no momento (0 = TELA INICIAL)
int estudanteLogado = 1; // Número do estudante que está usando o sistema atualmente
int saldosMemoria[6] = {0, 0, 0, 0, 0, 0}; // Máximo de 32256 segundo o Arduino
EasyNex myNex(Serial); // Conexão da Tela Nextion ao Arduino
// PINOS DOS COMPONENTES
const int motorEntradaSinal = 6; // Pino PWM do motor-entrada
const int motorDivisaoSinal = 5; // Pino PWM do motor-divisão
const int indutivoSinal = 13; // Pino digital do sensor indutivo
const int capacitivoSinal = 12; // Pino digital do sensor capacitivo
const int rele1 = 8; // Pino digital do relé 1
const int rele2 = 9; // Pino digital do relé 2

// FUNÇÕES DA TELA NEXTION
// Função de mudar a página da Tela Nextion
void mudarPagina(int ID = 0) {
  Serial.print("page " + String(ID)); // Envia o comando à tela
  // Finaliza o comando com 0xFF
  for (int i = 0; i < 3; i ++) {
    Serial.write(0xFF);
  }
  paginaAtual = ID;
}

// Atualiza as variáveis internas de saldo da Tela Nextion
void atualizarSaldos() {
  // Altera cada variável em sequência
  for (int i = 0; i < 6; i ++) {
    Serial.print("telaInicio.Saldo" + String(i + 1) + ".val=" + String(saldosMemoria[i])); // Envia o comando à tela
    // Finaliza o comando com 0xFF
    for (int j = 0; j < 3; j ++) {
      Serial.write(0xFF);
    }
  }
}

// Recebe e interpreta mensagens da Tela Nextion
// Mensagem: Página 1
void trigger0() {
  paginaAtual = 1;
}

// Mensagem: Página 2
void trigger1() {
  paginaAtual = 2;
}

// Mensagem: Estudante 1: "Erick Madrona"
void trigger2() {
  estudanteLogado = 1;
}

// Mensagem: Estudante 2: "Carlos Eduardo"
void trigger3() {
  estudanteLogado = 2;
}

// Mensagem: Estudante 3: "Yuri Mendonça"
void trigger4() {
  estudanteLogado = 3;
}

// Mensagem: Estudante 4: "Samuel Henrique"
void trigger5() {
  estudanteLogado = 4;
}

// Mensagem: Estudante 5: "Rafael Ferreira"
void trigger6() {
  estudanteLogado = 5;
}

// Mensagem: Estudante 6: "Bernardo Rosa"
void trigger7() {
  estudanteLogado = 6;
}

// FUNÇÕES DA MEMÓRIA EEPROM
// Carrega os saldos da memória EEPROM
void carregarSaldos() {
  EEPROM.get(0, saldosMemoria);
}

// Salva os saldos na memória EEPROM
void salvarSaldos() {
  EEPROM.put(0, saldosMemoria);
}


// FUNÇÕES DO ARDUINO
void setup() {
  // Inicia e aguarda a conexão da Tela Nextion
  myNex.begin(9600);
  // Conecta os SERVO MOTORES aos seus pinos
  motorEntrada.attach(motorEntradaSinal); // Motor-entrada como saída do Arduino
  motorDivisao.attach(motorDivisaoSinal); // Motor-divisão como saída do Arduino
  // Define os pinos de ENTRADA
  pinMode(indutivoSinal, INPUT); // Sensor indutivo como entrada do Arduino
  pinMode(capacitivoSinal, INPUT); // Sensor capacitivo como entrada do Arduino
  // Define os pinos de SAIDA
  pinMode(rele1, OUTPUT); // Relé 1 como saída do Arduino
  pinMode(rele2, OUTPUT); // Relé 2 como saída do Arduino
  // Redefine todas as variáveis
  digitalWrite(rele1, LOW); // Relé 1 desligado
  digitalWrite(rele2, LOW); // Relé 2 desligado
  motorEntrada.write(anguloInicialEntrada); // Motor-entrada no ângulo 170
  motorDivisao.write(anguloPlasticoDivisao); // Motor-divisão no ângulo 10
  posDivisao = anguloPlasticoDivisao;
  // Carrega os saldos guardados na memória EEPROM
  carregarSaldos();
  // Envia os saldos guardados na memória EEPROM
  atualizarSaldos();
}

void loop() {
  myNex.NextionListen(); // Recebe mensagens da Tela Nextion

  int estadoIndutivo = digitalRead(indutivoSinal); // Estado atual do sensor indutivo
  int estadoCapacitivo = digitalRead(capacitivoSinal); // Estado atual do sensor capacitivo
  // Caso o SENSOR INDUTIVO ou CAPACITIVO seja ativado
  if (estadoIndutivo == LOW || estadoCapacitivo == HIGH) {
    // Caso a Tela Nextion esteja na TELA DE INÍCIO DO PROCESSO
    if (paginaAtual == 2) {
      // Muda para a TELA DE PROCESSAMENTO
      mudarPagina(3);

      // REDIRECIONAMENTO da porta da divisão plástico/metal
      // Material detectado: PLÁSTICO
      if (estadoIndutivo == HIGH && posDivisao > anguloPlasticoDivisao) {
        for (int pos = posDivisao; pos > anguloPlasticoDivisao; pos --) {
          motorDivisao.write(pos); // Porta inclinada para a direita 
          posDivisao = pos;
          delay(10/velocidadeGlobal);
        }
        myNex.writeStr("processamento.Detectado.txt", "Detectado: Plastico");
      // Material detectado: METAL
      } else {
        for (int pos = posDivisao; pos < anguloMetalDivisao; pos ++) {
          motorDivisao.write(pos); // Porta inclinada para a esquerda 
          posDivisao = pos;
          delay(10/velocidadeGlobal);
        }
        myNex.writeStr("processamento.Detectado.txt", "Detectado: Metal");
      }

      delay(500/velocidadeGlobal);
          
      // Processo de AMASSAMENTO do material reciclável
      for (int i = 0; i < vezesAmassador; i ++) {
        // Gira no sentido anti-horário
        digitalWrite(rele1, HIGH);
        digitalWrite(rele2, LOW);
        delay(tempoAmassadorIda); // Espera o amassador ir
        // Gira no sentido horário  
        digitalWrite(rele1, LOW);
        digitalWrite(rele2, HIGH);
        delay(tempoAmassadorVolta); // Espera o amassador voltar
      }
      digitalWrite(rele1, HIGH); // Relé 1 desligado
      digitalWrite(rele2, HIGH); // Relé 2 desligado

      delay(500/velocidadeGlobal);

      // ABERTURA & FECHAMENTO da porta de entrada
      // A porta começa fechada e se abre
      for (int pos = anguloInicialEntrada; pos > anguloFinalEntrada; pos --) {
        motorEntrada.write(pos);
        delay(10/velocidadeGlobal);
      }

      delay(1000/velocidadeGlobal); // Espera para que o material reciclável consiga cair

      // A porta está aberta e se fecha
      for (int pos = anguloFinalEntrada; pos < anguloInicialEntrada; pos ++) {
        motorEntrada.write(pos);
        delay(10/velocidadeGlobal);
      }

      // FINALIZAÇÃO do processo de reciclagem
      mudarPagina(4); // Muda para a TELA DE FIM DE PROCESSO
      if (saldosMemoria[estudanteLogado - 1] + pagamentoIFCoins <= saldoMaximo) { // Precaução contra ultrapasso do saldo máximo
        saldosMemoria[estudanteLogado - 1] += pagamentoIFCoins; // Aumenta o saldo do estudante
      } else {
        saldosMemoria[estudanteLogado - 1] = 0; // Esvazia o saldo do estudante
      }
      atualizarSaldos(); // Atualiza novamente as variáveis da Tela Nextion
      salvarSaldos(); // Agora que os saldos foram alterados, salvá-los na memória EEPROM
    }
  }
}
