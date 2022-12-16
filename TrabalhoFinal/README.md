# Trabalho Final

## Introdução
<p align="justify"> A fim de aplicar o conhecimento adquirido na disciplina de sistemas embarcado foi desenvolvido um trabalho cujo o objetivo é controlar a velocidade de um motor brushless através de um smartphone via protocolo MQTT. Além disso, poder limitar a velocidade máxima do motor por meio de um potenciômetro de forma que a escala do cursor em um dashboard MQTT no smartphone seja completamente aproveitada. A escolha do trabalho foi feita visando a oportunidade de aplicar os seguintes tópicos aprendidos no decorrer do curso: Periféricos (I/O, PWM, ADC); Sistema de comunicação (MQTT, WIFI); Tasks (x2); Interrupção; fila. Dessa forma, o trabalho escolhido contempla boa parte do que foi lecionado durante o semestre. </p>

## Materiais
* 1x ESP32-V3
* 1x Potenciômetro de 20k
* 1x Transistor 2N2222A
* 1x Resistor de 1k
* 1x Resistor de 10k
* 1x Protoboard
* ESC EFLA1060
* Turbo fan EFLDF15
* Jumpers
* Fonte DC
* 2x Conector banana - jacaré
* Testador de servo motor genérico
* Osciloscópio
* Protoboard

## Descritivo do funcionamento
<p align="justify"> Para compreender o desenvolvimento do trabalho é preciso conhecer brevemente sobre o funcionamento dos equipamentos. 
A velocidade motor escolhido é controlada por um dispositivo chamado ESC (Eletronic Speed Controller), todo o chaveamento e potência necessários são fornecidos por ele. Para isso, o ESC precisa receber um sinal PWM que será interpretado como a velocidade desejada, esse é o sinal de interesse do nosso trabalho. </p>


1. Receber um valor de duty: dashboard MQTT -> ESP32
2. (ESP32) fazer a tratativa do valor recebido
3.
4.
5.

## Metodologia
<p align="justify"> Inicialmente, utilizando o osciloscópio a fonte e o testador de servo motor, estudou-se as características do sinal que deve ser aplicado no ESC para que ele atue no controle da velocidade do motor. Conhecendo esse sinal PWM a frequência  foi definida como 50 Hz e a resolução escolhida para o PWM foi de 12 bits. Como a largura do pulso do sinal do testador de servo varia entre 1,2 ms e 1,8 ms, para a resolução escolhida foi necessário descobrir o valor do duty equivalente. Isso poderia ser feito por cálculos, porém optou-se por criar um pequeno algoritimo para ler o valor de um ADC de 12 bits controlado por um potênciometro e escrever esse valor no duty de um PWM de mesma resolução. Dessa forma, com o auxílio do osciloscópio, foi possível descobrir os valores máximos e mínimos de duty que serão utilizados. </p>

<p align="justify"> Em seguida, para as configurações do ADC, como a tensão escolhida para ser aplicada no potenciômetro foi de 3.3 V, uma atenuação de 11 dB será utilizada para o ADC. Além da atenuação, é necessário definir a resolução do ADC. Para essa aplicação qualquer resolução do ADC fornecida pelo ESP32 atenderia, apenas a do PWM faria diferença já que a resolução mínima do pwm é de 1 bit, portanto a resolução escolhida para o ADC foi de 12 bits. Sabendo disso, foi calculado uma equação de reta que associasse a leitura do ADC com os valores máximos e mínimos do duty a ser aplicado no PWM. </p>

Para finalizar o sinal de controle, tendo em vista o objetivo apresentado na introdução, de poder aproveitar toda a escala do dashboard MQTT, uma segunda equação de reta foi elaborada. Essa segunda equação de reta recebe o duty do dashboard (que foi descoberto no experimento descrito acima. Entre 200 e 370) e associa com o valor limite lido do ADC e convertido pela primeira equação de reta. 

## Resultados
[![Watch the video](https://img.youtube.com/vi/T-D1KVIuvjA/maxresdefault.jpg)](https://youtu.be/T-D1KVIuvjA)

## Conclusões
