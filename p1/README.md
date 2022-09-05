# Questão 1: 
ok

# Questão 2: 
ok

# Questão 3:
O simples programa, utiliza algumas estruturas e funções para printar informações sobre o ESP32 utilizado e logo em seguida printa uma contagem regressiva para a reinicialização do mesmo.

Saída do programa:

Hello world! <br />
This is esp32 chip with 2 CPU core(s), WiFi/BT/BLE, silicon revision 1, 4MB external flash
Minimum free heap size: 295180 bytes <br />
Restarting in 10 seconds... <br />
Restarting in 9 seconds... <br />
Restarting in 8 seconds... <br />
Restarting in 7 seconds... <br />
Restarting in 6 seconds... <br />
Restarting in 5 seconds... <br />
Restarting in 4 seconds... <br />
Restarting in 3 seconds... <br />
Restarting in 2 seconds... <br />
Restarting in 1 seconds... <br />
Restarting in 0 seconds... <br />
Restarting now. <br />

# Questão 4:
Revisão feita <br />
A função printf permite apresentar na tela os valores de qualquer tipo  de dado. Para tanto, ela utiliza um mecanismo de formatação que permite traduzir a representação interna em variáveis para a representação ASCII que pode ser mostrada na tela.

# Questão 5 e 6:
Desenvolvidas no código p1.c

Exemplo de saída do programa:

Software reset via esp_restart. <br />
IDF Version: v4.4.2 <br />
Restarting in 3 seconds... <br />
Restarting in 2 seconds... <br />
Restarting in 1 seconds... <br />
Restarting in 0 seconds... <br />
Restarting now. <br />

# Questão 7:

A vTaslDelay tem a função de atrasar uma tarefa para um determinado número de ticks. O tempo real que a tarefa permanece bloqueada depende da taxa de tiques.

Configuração: <br />
void vTaskDelay( const TickType_t xTicksToDelay ); <br />
Onde: <br />
xTicksToDelay É a quantidade de tempo, em períodos de tique, que a tarefa de chamada deve bloquear.
