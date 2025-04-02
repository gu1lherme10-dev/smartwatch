#include "physicalActivityService.h"

#define MAX_PACKET_SIZE 240                               // Tamanho fixo do pacote BLE
#define EVENT_SIZE sizeof(ActivityEvent)                 // Cada evento ocupa 8 bytes
#define EVENTS_PER_PACKET (MAX_PACKET_SIZE / EVENT_SIZE) // 30 eventos por pacote

PhysicalActivityService *PhysicalActivityService::instance = nullptr;

// Construtor
PhysicalActivityService::PhysicalActivityService(PhysicalActivity* physicalActivity)
    : physicalActivity(physicalActivity),
      physicalActivityService("0x183E"),
      readActivitySummary("0x2B3D", BLERead, MAX_PACKET_SIZE, true),
      bufferOverflowNotify("46acd1c8-4caf-4330-8205-0c0743c8bfd4", BLENotify),
      notifyNewActivity("0x2B3C", BLENotify, sizeof(ActivityEvent)),
      inactiveStatus("d2d818ac-448f-4891-97ac-b9715aa44a1d", BLEWrite | BLERead)
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
    uint8_t emptyBuffer[MAX_PACKET_SIZE] = {0};
    inactiveStatus.setValue(0);
    inactiveStatus.setEventHandler(BLEWritten, updateInactiveStatusStatic);
    readActivitySummary.setValue(emptyBuffer, MAX_PACKET_SIZE);    
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

    const std::vector<ActivityEvent> &events = physicalActivity->getEventsForDay();
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
    Serial.println("Limpeza do buffer de eventos...");
    //physicalActivity->deleteAllEvents();
}

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
    Serial.println("updateInactiveStatusStatic chamada!");
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

    // Log detalhado do evento
    Serial.print("Notificando nova atividade - Timestamp: ");
    Serial.print(event.timestamp);
    Serial.print(" | Passos: ");
    Serial.println(event.data);

    // Log do buffer em formato binário
    Serial.println("Dados enviados (binário):");
    for (size_t i = 0; i < buffer.size(); i++) {
        Serial.print("Byte ");
        Serial.print(i);
        Serial.print(": ");
        Serial.println(buffer[i], BIN); // Imprime o byte em binário
    }

    // Log do buffer em formato hexadecimal (opcional)
    Serial.println("Dados enviados (hexadecimal):");
    for (size_t i = 0; i < buffer.size(); i++) {
        Serial.print("0x");
        if (buffer[i] < 0x10) Serial.print("0"); // Adiciona zero à esquerda para bytes menores que 0x10
        Serial.print(buffer[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

std::function<void(ActivityEvent)> PhysicalActivityService::notifyNewActivityCallback() {
    return [this](ActivityEvent event) {
        notifyNewActivityEvent(event);
    };
}
