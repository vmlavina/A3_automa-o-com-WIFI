#include <WiFi.h>                         // Inclui biblioteca WiFi

// ============================================================================
// --- Mapeamento de Hardware ---
#define relay1 25                    // Pino para controle do relé 1 (tomada)
#define relay2 26                    // Pino para controle do relé 2 (lâmpada)
#define ldrPin 34                    // Pino analógico para o sensor LDR

// ============================================================================
// --- Constantes Auxiliares ---
const char *ssid     = "01.12.2017";      // Nome da rede WiFi
const char *password = "20171201vija";    // Senha da rede

// ============================================================================
// --- Variáveis Globais ---
int ldrValue = 0;
bool automaticMode = false;
unsigned long previousMillis = 0;
const long interval = 10000;              // Intervalo de 10 segundos

// ============================================================================
// --- Objetos ---
WiFiServer server(80);                    // Define a porta que o servidor irá utilizar (80)

// ============================================================================
// --- Protótipo das Funções ---
void relay_wifi();                        // Função para gerar web server e controlar os relés

// ============================================================================
// --- Configurações Iniciais ---
void setup() 
{
  Serial.begin(115200);                  // Inicializa Serial em 115200 baud rate
  pinMode(relay1, OUTPUT);               // Configura saída para relé 1
  pinMode(relay2, OUTPUT);               // Configura saída para relé 2
  digitalWrite(relay1, HIGH);            // Desliga o relé 1 (tomada)
  digitalWrite(relay2, HIGH);            // Desliga o relé 2 (lâmpada)
  pinMode(ldrPin, INPUT);                // Configura o pino do LDR como entrada

  Serial.println();                      
  Serial.print("Conectando-se a ");      
  Serial.println(ssid);                  
  WiFi.begin(ssid, password);            // Inicializa WiFi, passando o nome da rede e a senha

  while(WiFi.status() != WL_CONNECTED)   // Aguarda conexão
  {
    delay(741);                          
    Serial.print(".");                   // Vai imprimindo pontos até realizar a conexão...
  }

  Serial.println("");                    // Mostra WiFi conectada
  Serial.println("WiFi conectada");      
  Serial.println("Endereço de IP: ");    
  Serial.println(WiFi.localIP());        // Mostra o endereço IP

  server.begin();                        // Inicializa o servidor web
}

// ============================================================================
// --- Loop Infinito ---
void loop() 
{
  while(WiFi.status() != WL_CONNECTED)   // Aguarda conexão
  {
    WiFi.begin(ssid, password);          // Inicializa WiFi
    Serial.print(".");                   // Vai imprimindo pontos até realizar a conexão...
    delay(741);                          // 741ms
  }
  
  relay_wifi();                          // Chama função para controle dos relés por wifi
  
  if (automaticMode) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      ldrValue = analogRead(ldrPin);
      if (ldrValue < 2000) {
        digitalWrite(relay2, LOW);       // Liga a lâmpada (relé 2)
      } else {
        digitalWrite(relay2, HIGH);      // Desliga a lâmpada (relé 2)
      }
    }
  }
}

// ============================================================================
// --- relay_wifi ---
void relay_wifi()
{
  WiFiClient client = server.available();      // Verifica se existe um cliente conectado com dados a serem transmitidos
  
  if (client)                                  // Existe um cliente?
  {                                            // Sim
    Serial.println("Novo Cliente Definido");   // Informa por serial
    String currentLine = "";                   // String para armazenar a mensagem do cliente
    
    while (client.connected())                 // Repete enquanto o cliente estiver conectado
    {
      if (client.available())                  // Existem dados a serem lidos?
      {                                        // Sim
        char c = client.read();                // Salva em c
        Serial.write(c);                       // Mostra na Serial
        
        if (c == '\n')                         // É um caractere de nova linha?
        {                                      // Sim
          if (currentLine.length() == 0)       // A mensagem terminou?
          {                                    // Sim
            // Gera a página HTML
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            client.print("<!DOCTYPE html><html>");
            client.print("<head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.print("<style>body { font-family: Arial, sans-serif; margin: 0; padding: 20px; text-align: center; }");
            client.print(".button { background-color: #4CAF50; border: none; color: white; padding: 15px 32px;");
            client.print("text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer; }");
            client.print(".button2 {background-color: #f44336;}</style></head>");
            client.print("<body><h1>Automação Residencial</h1>");
            client.print("<div style=\"margin: 20px 0;\"><h2>Tomada</h2>");
            client.print("<button class=\"button\" onclick=\"location.href='/H1'\">Ligar</button>");
            client.print("<button class=\"button button2\" onclick=\"location.href='/L1'\">Desligar</button></div>");
            client.print("<div style=\"margin: 20px 0;\"><h2>Lâmpada</h2>");
            client.print("<button class=\"button\" onclick=\"location.href='/H2'\">Ligar</button>");
            client.print("<button class=\"button button2\" onclick=\"location.href='/L2'\">Desligar</button>");
            client.print("<div style=\"margin: 20px 0;\"><h3>Modo de Operação</h3>");
            client.print("<button class=\"button\" onclick=\"location.href='/toggleMode'\">");
            if (automaticMode) {
              client.print("Automático");
            } else {
              client.print("Manual");
            }
            client.print("</button></div></div>");
            client.print("</body></html>");
            client.println();
            break;                             
          } 
          else currentLine = "";              
        } 
        else if (c != '\r') currentLine += c;  

        // Verifica para ligar ou desligar os relés
        if (currentLine.endsWith("GET /H1")) digitalWrite(relay1, LOW);
        if (currentLine.endsWith("GET /L1")) digitalWrite(relay1, HIGH);
        if (currentLine.endsWith("GET /H2")) {
          if (!automaticMode) digitalWrite(relay2, LOW);
        }
        if (currentLine.endsWith("GET /L2")) {
          if (!automaticMode) digitalWrite(relay2, HIGH);
        }
        if (currentLine.endsWith("GET /toggleMode")) {
          automaticMode = !automaticMode;
        }
      } 
    } 

    client.stop();                          
    Serial.println("Client Disconnected.");
  }  
}