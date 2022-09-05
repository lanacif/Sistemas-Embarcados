# Questão 1: 
ok

# Questão 2: 
- Pra GPIO4 subida e descida. Para GPIO4 subida.
- O programa tem 2 taks a app_main e a gpio_task_example.
- A gpio_task_example é pioritária em relação à app_main, ele é bloqueada quando os argumentos da fila do tipo de comunicação interrupt-to-task chega ao final. A app_main entra em bloqueio pelo vTaskDelay.
- O programa configura as IO's, interrupções, fila, e as tasks faladas acima. Quando uma interrupção é gerada a fila é incrementada com o numero do pino que gerou a interrupção. A task de exemplo esvazia a fila printando o pino que gerou a interrupção e o valor na porta.

Exemplo da saída:

GPIO[4] intr, val: 0 <br />
cnt: 1489 <br />
GPIO[4] intr, val: 1 <br />
GPIO[5] intr, val: 1 <br />
cnt: 1490 <br />
GPIO[4] intr, val: 0 <br />
cnt: 1491 <br />
GPIO[4] intr, val: 1 <br />
GPIO[5] intr, val: 1 <br />
cnt: 1492 <br />
GPIO[4] intr, val: 0 <br />

# Questão 3:
Feita no algoritimo p2.c <br />
OBS: O kit utilizado foi o V4, portanto por não ter um led fisicamente conectado no IO2 escolheu-se o IO4 para conectar um led externamente.
