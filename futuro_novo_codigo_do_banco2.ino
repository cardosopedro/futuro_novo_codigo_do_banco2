//Código Banco Estático - atualizado em 30-09
//VERIFICAR CÓDIGO SEM BOTÃO: OK
//VERIFICAR CÓDIGO COM BOTÃO: PROBLEMAS COM SD
//VERIFICAR NOVO CÓDIGO SDFAT: OK (não adiantou - mesma velocidade de gravação)
//VERIFICAR BOTÃO: OK
//VERIFICAR BOTÃO + MÓDULO SD:

#include <SD.h>
#include <SPI.h>
#include <HX711.h>
#include <Bounce2.h>

#define TEMPO_ATUALIZACAO 10 //em milisegundos

#define RLED 2
#define YLED 3
#define GLED 4
#define HX_DOUT 8
#define HX_CLK 7
#define CHAVE 5
#define CHIP_SELECT 10

//definições de erros
#define ERRO_SD 's'

//definição de estados
#define ESTADO_GRAVANDO 'g'
#define ESTADO_FINALIZADO 'f'
#define ESTADO_ESPERA 'e'

//variaveis do SD
File file;
char nomeBase[] = "dataLog";
char nomeConcat[12];

//variaveis do HX711
HX711 scale(HX_DOUT, HX_CLK);
float calibration_factor = 2.00;
float dado;

//variaveis da chave do código (c/ debounce)
Bounce debouncer = Bounce();

//Variáveis de timing
unsigned long millisAtual  = 0;
unsigned long atualizaMillis = 0;
unsigned long millisInicial = 0;
unsigned long millisUltimo = 0;

//variáveis de contagem
int ndados = 0;
int contInicial = 0;

//variáveis de controle
char erro = false;
char statusAtual;
bool estado;


void setup() {

  inicializa();

}

void inicializa() {

  //Inicializando as portas
  pinMode(CHAVE, INPUT_PULLUP);
  pinMode(RLED, OUTPUT);
  pinMode(YLED, OUTPUT);
  pinMode(GLED, OUTPUT);

  Serial.begin(9600);
  debouncer.attach(CHAVE); // Informa que o tratamento de debounce será feito no pino CHAVE
  debouncer.interval(10); // Seta o intervalo de trepidação;

  //  //inicializar o cartão SD
  //  if (!SD.begin(CHIP_SELECT)) {
  //
  //    erro = ERRO_SD;
  //
  //    return;
  //  }
  //  else if (!erro) {
  //    int n = 1;
  //    bool parar = false;
  //
  //
  //    while (!parar)
  //    {
  //      sprintf(nomeConcat, "teste %d", n);
  //      if (SD.exists(nomeConcat))
  //        n++;
  //      else
  //        parar = true;
  //    }
  //
  //    file = SD.open(nomeConcat, FILE_WRITE);
  //  }


  if (!erro)
  {
    statusAtual = ESTADO_ESPERA;
  }

  else {
    statusAtual = erro;
    atualizaMillis = millis();
  }

  scale.set_scale();
  scale.tare();

}

void loop() {

  //Recebendo o tempo atual de maneira a ter uma base de tempo
  //para uma taxa de atualização
  millisAtual = millis();

  if ((millisAtual - atualizaMillis) >= TEMPO_ATUALIZACAO) {

    //verifica se existem erros e mantém tentando inicializar
    if (erro) {
      inicializa();
      notifica(erro);
    }

    //Se não existem erros no sistema relacionados a inicialização
    //dos dispositivos, fazer:
    if (!erro) {

      //Verifica os botão que inicia/termina a gravação
      leChave();

      //Obtem o tempo em que inicia-se a gravação
      conta();

      //Recebe os dados do HX711 e os deixam salvos na variavel dado
      adquireDados();

      //Se a gravação estiver ligada, grava os dados.
    //gravaDados();

      //Imprime o fim da gravação
      finaliza();
    }

    //Notifica via LEDs os possíveis estados do projeto

    notifica(statusAtual);

    atualizaMillis = millisAtual;
    if (statusAtual == ESTADO_FINALIZADO) {
      inicializa();
    }
  }

}

void leChave() {

  debouncer.update(); // Executa o algorítimo de tratamento;
  estado = debouncer.read(); // Lê o valor tratado do botão;
  millisAtual = millis();
  //Liga a gravação se em espera
  if (!estado) {
    if (statusAtual == ESTADO_ESPERA) {
      statusAtual = ESTADO_GRAVANDO;
    }
  }
  else { //Para a gravação se desligar a chave
    if (statusAtual == ESTADO_GRAVANDO) {
      statusAtual = ESTADO_FINALIZADO;
    }
  }
}

void conta() {

  if ((statusAtual == ESTADO_GRAVANDO) && contInicial == 0) {
    millisInicial = millis();
    contInicial = 1;
  }

}

void adquireDados() {
  if (statusAtual == ESTADO_GRAVANDO) { //p/ testar a chave
    scale.set_scale(calibration_factor);
    dado = scale.get_units(), 10;
    if (dado < 0.00)
    {
      dado = 0.00;
    }
    Serial.println(dado);
    millisUltimo = millis();
  }
}

void gravaDados() {

  if (statusAtual == ESTADO_GRAVANDO)
  {
    file = SD.open(nomeConcat, FILE_WRITE);
    if (file)
    {
      file.println(dado);
      file.close();
      ndados++;
    }
    else
    {
      statusAtual = ERRO_SD;
    }
  }

}


void finaliza() {

  if (statusAtual == ESTADO_FINALIZADO)
  {
    file = SD.open(nomeConcat, FILE_WRITE);
    if (file)
    {
      float tempoms = millisUltimo - millisInicial;
      int tempo = tempoms / 1000.00;
      file.println();
      file.print("Fim! Dados obtidos: ");
      file.println(ndados);
      file.print("   Tempo de gravação (s): ");
      file.print(tempo);
      file.close();
    }
    //    else      p/ testar a chave
    //    {
    //      statusAtual = ERRO_SD;
    //    }
  }

}


void notifica (char codigo) {

  switch (codigo) {

    case ERRO_SD:

      digitalWrite(RLED, HIGH);
      digitalWrite(YLED, LOW);
      digitalWrite(GLED, LOW);

      break;

    case ESTADO_GRAVANDO:

      digitalWrite(RLED, LOW);
      digitalWrite(YLED, LOW);
      digitalWrite(GLED, HIGH);

      break;

    case ESTADO_ESPERA:

      digitalWrite(RLED, LOW);
      digitalWrite(YLED, HIGH);
      digitalWrite(GLED, LOW);

      break;

    case ESTADO_FINALIZADO:

      digitalWrite(RLED, LOW);
      digitalWrite(YLED, LOW);
      digitalWrite(GLED, LOW);

      break;

  }
}
