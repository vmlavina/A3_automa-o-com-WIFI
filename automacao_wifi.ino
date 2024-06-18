#include <WiFi.h> // Inclui biblioteca WiFi

// ============================================================================
// --- Mapeamento de Hardware ---
#define relay1 25 // Pino para controle da Tomada
#define relay2 26 // Pino para controle da Lâmpada
#define ldrPin 34 // Pino para leitura do sensor LDR

// ============================================================================
// --- Constantes Auxiliares ---
const char *ssid = "01.12.2017"; // Nome da rede WiFi
const char *password = "20171201vija"; // Senha da rede

// ============================================================================
// --- Objetos ---
WiFiServer server(80); // Define a porta que o servidor irá utilizar (padrão 80)

// ============================================================================
// --- Variáveis ---
String mode = "Manual"; // Variável para armazenar o modo selecionado

// ============================================================================
// --- Protótipo das Funções ---
void handleClient(); // Função para gerar web server e controlar os relés
void controlLampAutomatically(); // Função para controlar a lâmpada automaticamente

// ============================================================================
// --- Configurações Iniciais ---
void setup() {
  Serial.begin(115200); // Inicializa Serial em 115200 baud rate
  pinMode(relay1, OUTPUT); // Configura saída para Tomada
  pinMode(relay2, OUTPUT); // Configura saída para Lâmpada
  pinMode(ldrPin, INPUT); // Configura entrada para o sensor LDR

  // Desliga os relés inicialmente
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);

  Serial.println();
  Serial.print("Conectando-se a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password); // Inicializa WiFi, passando o nome da rede e a senha

  while (WiFi.status() != WL_CONNECTED) { // Aguarda conexão
    delay(741);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi conectada");
  Serial.print("Endereço de IP: ");
  Serial.println(WiFi.localIP()); // Mostra o endereço IP

  server.begin(); // Inicializa o servidor web
}

// ============================================================================
// --- Loop Infinito ---
void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    handleClient(); // Chama função para controle dos relés por WiFi
  } else {
    // Tenta reconectar caso a conexão WiFi seja perdida
    WiFi.begin(ssid, password);
    Serial.print(".");
    delay(741);
  }

  // Controle automático da lâmpada
  if (mode == "Automático") {
    controlLampAutomatically();
  }
}

// ============================================================================
// --- handleClient ---
void handleClient() {
  WiFiClient client = server.available(); // Verifica se existe um cliente conectado com dados a serem transmitidos

  if (client) { // Existe um cliente?
    Serial.println("Novo Cliente Definido");
    String currentLine = ""; // String para armazenar a mensagem do cliente

    while (client.connected()) { // Repete enquanto o cliente estiver conectado
      if (client.available()) { // Existem dados a serem lidos?
        char c = client.read(); // Salva em c
        Serial.write(c); // Mostra na Serial

        if (c == '\n') { // É um caractere de nova linha?
          if (currentLine.length() == 0) { // A mensagem terminou?
            // Gera a página HTML
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            client.println("<!DOCTYPE html>");
            client.println("<html>");
            client.println("<head>");
            client.println("<title>Controle de Dispositivos</title>");
            client.println("<style>");
            client.println("body { font-family: Arial, sans-serif; text-align: center; background-color: #f2f2f2; padding-top: 50px; }");
            client.println("h2 { color: #333; }");
            client.println(".device { margin-bottom: 40px; }");
            client.println(".button { display: inline-block; padding: 20px 30px; font-size: 24px; margin: 10px; text-decoration: none; color: white; border-radius: 5px; }");
            client.println(".on { background-color: #4CAF50; }"); // Cor verde
            client.println(".off { background-color: #f44336; }"); // Cor vermelha
            client.println(".selector { margin-bottom: 20px; }");
            client.println(".selector button { background-color: #008CBA; border: none; color: white; padding: 20px 40px; text-align: center; text-decoration: none; display: inline-block; font-size: 24px; margin: 4px 2px; cursor: pointer; border-radius: 5px; }");
            client.println(".selected { background-color: #4CAF50; }"); // Cor verde
            client.println("</style>");
            client.println("</head>");
            client.println("<body>");
            client.println("<h2>Controle de Dispositivos</h2>");
            client.println("<div class=\"device\">");
            client.println("<h3>Tomada</h3>");
            client.println("<a href=\"/H1\" class=\"button on\">Ligar Tomada</a>");
            client.println("<a href=\"/L1\" class=\"button off\">Desligar Tomada</a>");
            client.println("</div>");
            client.println("<div class=\"device\">");
            client.println("<h3>L&acirc;mpada</h3>"); // Corrigido "Lâmpada"
            client.println("<a href=\"/H2\" class=\"button on\">Ligar L&acirc;mpada</a>"); // Corrigido "Lâmpada"
            client.println("<a href=\"/L2\" class=\"button off\">Desligar L&acirc;mpada</a>"); // Corrigido "Lâmpada"
            client.println("</div>");
            client.println("<div class=\"selector\">");
            client.print("<button id=\"mode\" class=\"button ");
            client.print(mode == "Manual" ? "selected" : "");
            client.print("\" onclick=\"toggleMode()\">");
            client.print(mode);
            client.println("</button>");
            client.println("</div>");
            client.println("<script>");
            client.println("function toggleMode() {");
            client.println("  var xhr = new XMLHttpRequest();");
            client.println("  xhr.open('GET', '/toggleMode', true);");
            client.println("  xhr.send();");
            client.println("  setTimeout(function() { location.reload(); }, 500);");
            client.println("}");
            client.println("</script>");
            client.println("</body>");
            client.println("</html>");
            break; // Encerra o laço
          } else {
            currentLine = ""; // Reseta a string
          }
        } else if (c != '\r') {
          currentLine += c; // Adiciona o caractere à mensagem
        }

        // Verifica para ligar ou desligar os relés
        if (currentLine.endsWith("GET /H1")) digitalWrite(relay1, LOW); // Ligar relé 1
        if (currentLine.endsWith("GET /L1")) digitalWrite(relay1, HIGH); // Desligar relé 1
        if (currentLine.endsWith("GET /H2")) {
          if (mode == "Manual") digitalWrite(relay2, LOW); // Ligar relé 2
        }
        if (currentLine.endsWith("GET /L2")) {
          if (mode == "Manual") digitalWrite(relay2, HIGH); // Desligar relé 2
        }
        if (currentLine.endsWith("GET /toggleMode")) mode = (mode == "Manual") ? "Automático" : "Manual";
      }
    }
    client.stop(); // Finaliza conexão
    Serial.println("Client Disconnected.");
  }
}

// ============================================================================
// --- controlLampAutomatically ---
void controlLampAutomatically() {
  int ldrValue = analogRead(ldrPin); // Lê o valor do sensor LDR
  Serial.print("LDR Value: ");
  Serial.println(ldrValue);

  if (ldrValue < 2000) {
    digitalWrite(relay2, LOW); // Liga a lâmpada (nível lógico baixo) se o valor for menor que 2000
  } else {
    digitalWrite(relay2, HIGH); // Desliga a lâmpada (nível lógico alto) se o valor for maior ou igual a 2000
  }
}
