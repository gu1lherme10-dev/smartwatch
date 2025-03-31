#include "physicalActivityService.h"

#define MAX_PACKET_SIZE 80                               // Tamanho fixo do pacote BLE
#define EVENT_SIZE sizeof(ActivityEvent)                 // Cada evento ocupa 8 bytes
#define EVENTS_PER_PACKET (MAX_PACKET_SIZE / EVENT_SIZE) // 30 eventos por pacote

PhysicalActivityService *PhysicalActivityService::instance = nullptr;

// Construtor
PhysicalActivityService::PhysicalActivityService()
    : physicalActivityService("0x183E"),
      readActivitySummary("0x2B3D", BLERead, MAX_PACKET_SIZE, true),
      bufferOverflowNotify("46acd1c8-4caf-4330-8205-0c0743c8bfd4", BLENotify),
      notifyNewActivity("0x2B3C", BLENotify, sizeof(ActivityEvent)),
      inactiveStatus("d2d818ac-448f-4891-97ac-b9715aa44a1d", BLEWrite)
{
    instance = this;
}

// Inicializa o serviço BLE
void PhysicalActivityService::begin()
{
    physicalActivityService.addCharacteristic(readActivitySummary);
    physicalActivityService.addCharacteristic(bufferOverflowNotify);
    physicalActivityService.addCharacteristic(notifyNewActivity);
    physicalActivityService.addCharacteristic(inactiveStatus);
    BLE.addService(physicalActivityService);
    BLE.setAdvertisedService(physicalActivityService);

    bufferOverflowNotify.setValue(0);
    // Define um valor inicial para readActivitySummary
    std::vector<ActivityEvent> initialEvents = generateActivityEvents(1742947200, 1); // Gera 1 evento inicial
    std::vector<uint8_t> buffer = serializeEvents(initialEvents);
    readActivitySummary.setValue(buffer.data(), buffer.size());
    readActivitySummary.setEventHandler(BLERead, sendActivitySummary);
}

// Definição da variável estática
bool PhysicalActivityService::appIsActive = false;

void PhysicalActivityService::loop() {}

// Notifica buffer cheio
void PhysicalActivityService::notifyBufferOverflow()
{
    bufferOverflowNotify.writeValue(1);
    Serial.println("Buffer cheio! Notificando app...");
}

std::vector<PhysicalActivityService::ActivityEvent> PhysicalActivityService::generateActivityEvents(uint32_t startTimestamp, uint32_t numEvents)
{
    std::vector<ActivityEvent> events;
    uint32_t currentTimestamp = startTimestamp;

    for (uint32_t i = 0; i < numEvents; i++)
    {
        ActivityEvent event;
        event.timestamp = currentTimestamp;

        uint32_t activityType = 0;  // 0 = Inatividade, 1 = Caminhada, 2 = Corrida
        uint32_t numSteps = 0;

        // Gerar número de passos com base no tipo de atividade
        if (i % 3 == 0)  // Inatividade
        {
            activityType = 0;  // Tipo de atividade: Inatividade
            numSteps = 0;      // Passos = 0
        }
        else if (i % 3 == 1)  // Caminhada
        {
            activityType = 1;  // Tipo de atividade: Caminhada
            numSteps = 1000 + rand() % 2000; // Passos realistas para caminhada
        }
        else  // Corrida
        {
            activityType = 2;  // Tipo de atividade: Corrida
            numSteps = 3000 + rand() % 5000; // Passos realistas para corrida
        }

        // Combina o tipo de atividade (2 bits) e os passos (30 bits) no mesmo valor
        uint32_t activityData = (activityType << 30) | (numSteps & 0x3FFFFFFF); // 0x3FFFFFFF mascara os 30 bits de passos

        event.activity_steps = activityData; // Atribuindo o valor combinado

        // Logando os detalhes de cada evento gerado
        Serial.print("Evento gerado: ");
        Serial.print("Timestamp: ");
        Serial.print(event.timestamp);
        Serial.print(" | Tipo de Atividade: ");
        switch (activityType)
        {
            case 0: Serial.print("Inatividade"); break;
            case 1: Serial.print("Caminhada"); break;
            case 2: Serial.print("Corrida"); break;
        }
        Serial.print(" | Passos: ");
        Serial.print(numSteps);
        Serial.print(" | activity_steps (32 bits): ");
        Serial.println(activityData, HEX); // Exibe o valor em hexadecimal

        // Armazenar o evento gerado
        events.push_back(event);

        // Incrementar o timestamp (aqui podemos manter o intervalo de 5 minutos)
        currentTimestamp += 300000;  // Incrementa 5 minutos (300000 ms)
    }

    return events;
}



// Serializa eventos em um buffer de bytes
std::vector<uint8_t> PhysicalActivityService::serializeEvents(const std::vector<ActivityEvent> &events)
{
    std::vector<uint8_t> buffer;
    buffer.reserve(events.size() * EVENT_SIZE);

    for (const auto &event : events)
    {
        const uint8_t *ptr = reinterpret_cast<const uint8_t *>(&event);
        buffer.insert(buffer.end(), ptr, ptr + EVENT_SIZE);
    }
    return buffer;
}

void PhysicalActivityService::sendActivityEventsBLE(BLEDevice central, BLECharacteristic characteristic)
{
    Serial.println("Iniciando envio de eventos via BLE...");

    const std::vector<ActivityEvent> &events = generateActivityEvents(1742947200, 10);
    size_t totalEvents = events.size();
    Serial.print("Total de eventos gerados: ");
    Serial.println(totalEvents);

    size_t index = 0;
    while (index < totalEvents)
    {
        size_t batchSize = std::min(EVENTS_PER_PACKET, totalEvents - index);
        std::vector<ActivityEvent> batch(events.begin() + index, events.begin() + index + batchSize);
        std::vector<uint8_t> buffer = serializeEvents(batch);

        Serial.print("Pacote de ");
        Serial.print(batchSize);
        Serial.println(" eventos preparado para envio.");

        // Debug: imprimir os primeiros bytes do buffer
        Serial.print("Primeiros bytes do buffer: ");
        for (size_t i = 0; i < std::min(buffer.size(), (size_t)10); i++)
        {
            Serial.print(buffer[i], HEX);
            Serial.print(" ");
        }
        Serial.println();

        // Envio BLE
        characteristic.writeValue(buffer.data(), buffer.size());
        Serial.println("Pacote enviado via BLE!");

        index += batchSize;
        delay(10);
    }

    Serial.println("Todos os eventos foram enviados!");
}

// void PhysicalActivityService::sendActivityEventsBLE(BLEDevice central, BLECharacteristic characteristic) {
//     Serial.println("📡 sendActivitySummary chamada! Enviando 1 evento de teste...");

//     ActivityEvent event;
//     event.timestamp = 1742947200;  // Timestamp fixo para debug
//     event.activity_steps = 1234;   // Número fixo de passos para testar

//     std::vector<uint8_t> buffer(sizeof(ActivityEvent));
//     memcpy(buffer.data(), &event, sizeof(ActivityEvent));

//     Serial.print("🔹 Timestamp: ");
//     Serial.print(event.timestamp);
//     Serial.print(" | Passos: ");
//     Serial.println(event.activity_steps);

//     Serial.print("🔹 Buffer size: ");
//     Serial.println(buffer.size());

//     characteristic.writeValue(buffer.data(), buffer.size());
//     Serial.println("🚀 Evento enviado via BLE!");
// }

// Envia resumo da atividade
void PhysicalActivityService::sendActivitySummary(BLEDevice central, BLECharacteristic characteristic)
{
    Serial.println("sendActivitySummary chamada!");

    if (instance != nullptr)
    {
        instance->sendActivityEventsBLE(central, characteristic);
    }
    else
    {
        Serial.println("⚠️ Instância da classe não inicializada!");
    }
}

// Atualiza status de atividade do app
void PhysicalActivityService::updateInactiveStatus(BLEDevice central, BLECharacteristic characteristic)
{
    uint8_t status = inactiveStatus.value();
    appIsActive = (status == 1);
    Serial.print("App está ativo: ");
    Serial.println(appIsActive ? "SIM" : "NÃO");
}

// Método estático para atualizar status de atividade
void PhysicalActivityService::updateInactiveStatusStatic(BLEDevice central, BLECharacteristic characteristic)
{
    if (instance != nullptr)
    {
        instance->updateInactiveStatus(central, characteristic);
    }
}

// Notifica um novo evento de atividade
void PhysicalActivityService::notifyNewActivityEvent(ActivityEvent event)
{
    std::vector<uint8_t> buffer(sizeof(ActivityEvent));
    memcpy(buffer.data(), &event, sizeof(ActivityEvent));

    notifyNewActivity.writeValue(buffer.data(), buffer.size());

    Serial.print("Notificando nova atividade - Timestamp: ");
    Serial.print(event.timestamp);
    Serial.print(" | Passos: ");
    Serial.println(event.activity_steps);
}
