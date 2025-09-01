/**
 * @file ejemplo_servo_simple.ino
 * @brief Ejemplo simple del nuevo controlador de servo
 * 
 * Este ejemplo demuestra cómo usar el nuevo controlador de servo
 * que implementa el patrón específico proporcionado por el usuario.
 * 
 * @author Sistema de Riego Inteligente
 * @version 3.0
 * @date 2025
 */

#include <Arduino.h>
#include <Servo.h>

// =============================================================================
// Configuración del Servo
// =============================================================================

Servo servoMotor;
int pinServo = 9;  // Pin PWM conectado al servo

// Configuración de tiempos
const unsigned long MOVEMENT_DELAY = 500;   // 0.5 segundos
const unsigned long FREE_DELAY = 3000;      // 3 segundos

// Estados del servo
enum ServoState {
    IDLE,
    MOVING_TO_OPEN,
    OPEN,
    MOVING_TO_CLOSE,
    CLOSED
};

ServoState currentState = IDLE;
unsigned long stateStartTime = 0;
unsigned long cycleCount = 0;
bool autoCycle = false;

// =============================================================================
// Setup
// =============================================================================

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("=== EJEMPLO SERVO SIMPLE ===");
    Serial.println("Inicializando servomotor...");
    
    // Arranca en 0 grados
    servoMotor.attach(pinServo);
    servoMotor.write(0);
    delay(MOVEMENT_DELAY);          // Espera 0.5 segundos
    servoMotor.detach(); // Desenergiza
    delay(FREE_DELAY);         // Espera 3 segundos
    
    currentState = CLOSED;
    stateStartTime = millis();
    
    Serial.println("Servomotor inicializado correctamente");
    Serial.println("Posición inicial: 0°");
    Serial.println("Iniciando ciclo automático...");
    
    // Iniciar ciclo automático
    startCycle(true);
}

// =============================================================================
// Loop
// =============================================================================

void loop() {
    updateServo();
    
    // Imprimir estado cada 5 segundos
    static unsigned long lastReport = 0;
    if (millis() - lastReport > 5000) {
        printStatus();
        lastReport = millis();
    }
    
    // Permitir que el ESP32 maneje otras tareas
    yield();
}

// =============================================================================
// Funciones del Controlador
// =============================================================================

void startCycle(bool enableAutoCycle) {
    autoCycle = enableAutoCycle;
    currentState = MOVING_TO_OPEN;
    stateStartTime = millis();
    
    Serial.println("Iniciando ciclo de movimiento");
    Serial.println("Auto-ciclo: " + String(autoCycle ? "Habilitado" : "Deshabilitado"));
}

void updateServo() {
    if (currentState == IDLE) {
        return;
    }

    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - stateStartTime;

    switch (currentState) {
        case MOVING_TO_OPEN:
            if (elapsedTime >= MOVEMENT_DELAY) {
                // Servo ya está en posición abierta, desenergizar
                servoMotor.detach();
                currentState = OPEN;
                stateStartTime = currentTime;
                
                Serial.println("Movido a 85° - Desenergizado");
            }
            break;

        case OPEN:
            if (elapsedTime >= FREE_DELAY) {
                // Iniciar movimiento a posición cerrada
                servoMotor.attach(pinServo);
                servoMotor.write(0);
                currentState = MOVING_TO_CLOSE;
                stateStartTime = currentTime;
                
                Serial.println("Moviendo a 0°");
            }
            break;

        case MOVING_TO_CLOSE:
            if (elapsedTime >= MOVEMENT_DELAY) {
                // Servo ya está en posición cerrada, desenergizar
                servoMotor.detach();
                currentState = CLOSED;
                stateStartTime = currentTime;
                cycleCount++;
                
                Serial.println("Movido a 0° - Desenergizado");
                Serial.println("Ciclo completado. Total: " + String(cycleCount));
                
                if (autoCycle) {
                    // Esperar tiempo libre antes del siguiente ciclo
                    delay(FREE_DELAY);
                    // Reiniciar ciclo
                    startCycle(true);
                }
            }
            break;

        case CLOSED:
            // En posición cerrada, esperar comandos
            break;

        default:
            break;
    }
}

void openServo() {
    servoMotor.attach(pinServo);
    servoMotor.write(85);
    currentState = OPEN;
    stateStartTime = millis();
    
    Serial.println("Abierto manualmente a 85°");
}

void closeServo() {
    servoMotor.attach(pinServo);
    servoMotor.write(0);
    currentState = CLOSED;
    stateStartTime = millis();
    
    Serial.println("Cerrado manualmente a 0°");
}

void stopCycle() {
    if (currentState != IDLE && currentState != CLOSED) {
        closeServo();
    }
    
    currentState = IDLE;
    autoCycle = false;
    
    Serial.println("Ciclo detenido");
}

// =============================================================================
// Funciones de Utilidad
// =============================================================================

const char* stateToString(ServoState state) {
    switch (state) {
        case IDLE: return "IDLE";
        case MOVING_TO_OPEN: return "MOVING_TO_OPEN";
        case OPEN: return "OPEN";
        case MOVING_TO_CLOSE: return "MOVING_TO_CLOSE";
        case CLOSED: return "CLOSED";
        default: return "UNKNOWN";
    }
}

void printStatus() {
    Serial.println("\n=== ESTADO DEL SERVOMOTOR ===");
    Serial.println("Estado: " + String(stateToString(currentState)));
    Serial.println("Pin: " + String(pinServo));
    Serial.println("Auto-ciclo: " + String(autoCycle ? "Habilitado" : "Deshabilitado"));
    Serial.println("Ciclos completados: " + String(cycleCount));
    Serial.println("Tiempo en estado actual: " + String(millis() - stateStartTime) + "ms");
    Serial.println("===============================\n");
}

// =============================================================================
// Comandos por Serial (Opcional)
// =============================================================================

void handleSerialCommands() {
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        
        if (command == "open") {
            openServo();
        } else if (command == "close") {
            closeServo();
        } else if (command == "start") {
            startCycle(true);
        } else if (command == "stop") {
            stopCycle();
        } else if (command == "status") {
            printStatus();
        } else if (command == "help") {
            Serial.println("Comandos disponibles:");
            Serial.println("  open   - Abrir servo manualmente");
            Serial.println("  close  - Cerrar servo manualmente");
            Serial.println("  start  - Iniciar ciclo automático");
            Serial.println("  stop   - Detener ciclo");
            Serial.println("  status - Mostrar estado");
            Serial.println("  help   - Mostrar esta ayuda");
        }
    }
}

// Para usar comandos serial, descomenta esta línea en loop():
// handleSerialCommands();
