# Trabalho Final

## Introdução
A fim de aplicar o conhecimento adquirido na disciplina de sistemas embarcado foi desenvolvido um trabalho cujo o objetivo é controlar a velocidade de um motor brushless através de um smartphone, via protocolo MQTT. Além disso, poder limitar a velocidade máxima do motor por meio de um potenciômetro de forma que a escala do cursor em um dashboard MQTT no smartphone seja completamente aproveitada. A escolha do trabalho foi feita visando a oportunidade de aplicar os seguintes tópicos aprendidos no decorrer do curso: Periféricos (I/O, PWM, ADC); Sistema de comunicação (MQTT, WIFI); Tasks (x2); Interrupção; fila. Dessa forma, o trabalho escolhido contempla boa parte do que foi lecionado durante o semestre.

# Materiais

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

## Metodologia
Inicialmente, utilizando o osciloscópio a fonte e o testador de servo motor, estudou-se as características do sinal que deve ser aplicado no ESC para que ele atue no controle da velocidade do motor. Feito isso, foi calculado uma equação de reta que associasse a leitura do ADC com os valores máximos e mínimos  


## Resultados
## Conclusões
