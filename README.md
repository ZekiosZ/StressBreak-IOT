# StressBreak IoT -- Sistema de Monitoramento de Estresse em Home Office

MQTT + Node-RED + ESP32 + Wokwi

------------------------------------------------------------------------

## Link para acesso ao projeto pelo wokwi
[Clique Aqui](https://wokwi.com/projects/447535451473761281)

------------------------------------------------------------------------

## 📌 Resumo do Projeto

O **StressBreak IoT** é um sistema que monitora sinais ambientais e
comportamentais durante o trabalho em home office e calcula um **Índice
de Estresse**.\
Com base nesse índice, o sistema sugere pausas automaticamente e envia
comandos ao dispositivo via MQTT.

O projeto funciona integrado a: - ESP32 (Wokwi ou físico) - Sensores
(LDR, DHT11, potenciômetro) - Servidor IoT FIWARE/Helix MQTT - Node-RED
(dashboard e lógica de decisão) - LCD + buzzer para feedback local

------------------------------------------------------------------------

## 📡 Fluxo Geral do Sistema

    ESP32 → MQTT (FIWARE/Helix) → Node-RED → /cmd → ESP32

------------------------------------------------------------------------

## 📦 Componentes Usados

### 1. ESP32 DevKit V1

### 2. LDR (KY-018)

• VCC → 3V3\
• GND → GND\
• AO → GPIO 34

### 3. Potenciômetro (nível de estresse manual)

• VCC → 3V3\
• GND → GND\
• SIG → GPIO 35

### 4. Sensor DHT11

• DATA → GPIO 4\
• VCC → 3V3\
• GND → GND

### 5. Buzzer

• + → GPIO 27\
• - → GND

### 6. LCD 16x2 I2C

• SDA → GPIO 21\
• SCL → GPIO 22

------------------------------------------------------------------------

## 📡 MQTT -- Tópicos Utilizados

  Função             Tópico                     Exemplo
  ------------------ -------------------------- -----------------
  StressIndex        `/TEF/device070/attrs`     `s|85`
  Luminosidade       `/TEF/device070/attrs/p`   `58`
  Umidade            `/TEF/device070/attrs/h`   `45.3`
  Temperatura        `/TEF/device070/attrs/t`   `23.1`
  Comandos → ESP32   `/TEF/device070/cmd`       `device070@on|`

------------------------------------------------------------------------

## 🧠 Lógica do StressIndex (no ESP32)

    StressIndex = 0.4 * potStress
                + 0.3 * desconfortoTermico
                + 0.3 * (100 - luminosidade)

------------------------------------------------------------------------

## 🟦 Node-RED -- Automação da Pausa

### Regras:

-   **Stress ≥ 70** → envia pausa → `device070@on|`
-   **Stress ≤ 40** → retoma fluxo normal → `device070@off|`
-   Dashboard exibe:
    -   Stress (gauge + gráfico histórico)
    -   Luminosidade
    -   Temperatura
    -   Umidade
    -   Indicador de Pausa

A dashboard fica disponível em:

    http://localhost:1880/ui

------------------------------------------------------------------------
## 🚀 Como Rodar o Projeto

### 1. Wokwi / ESP32

-   Abra o projeto no Wokwi\
-   Configure o WiFi e broker MQTT no código\
-   Rode a simulação\
-   Verifique no log que está enviando os tópicos

### 2. Node-RED

-   Importe `node-red-flow.json`\
-   Configure broker MQTT\
-   Acesse o dashboard: digite `/ui` depois do endereço que esta no seu navegador no "site" gerado pelo node-red

### 3. FIWARE / Helix MQTT

Certifique-se que os tópicos estão liberados conforme:

    /TEF/device070/attrs
    /TEF/device070/attrs/p
    /TEF/device070/attrs/h
    /TEF/device070/attrs/t
    /TEF/device070/cmd

Conecte no aplicativo MyMqtt com a ip do seu servidor fiware, usando os topicos acima, e vc recebera os dados do mesmo que foi escolhido
------------------------------------------------------------------------

Desenvolvido como Entrega para a Global Solution Fiap 2025 para a materia de Edge Computing sistems.
