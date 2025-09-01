/**
 * @file ObjectPool.h
 * @brief Sistema de pooling de objetos para optimización de memoria.
 * 
 * **CONCEPTO EDUCATIVO - PATRÓN OBJECT POOL**:
 * Este patrón permite reutilizar objetos en lugar de crearlos y destruirlos
 * constantemente, reduciendo la fragmentación de memoria y mejorando el rendimiento.
 * 
 * @author Sistema de Riego Inteligente
 * @version 4.0 - Optimización de Memoria
 * @date 2025
 */

#ifndef __OBJECT_POOL_H__
#define __OBJECT_POOL_H__

#include <stdint.h>
#include <vector>
#include <queue>

/**
 * @brief Plantilla de pool de objetos para reutilización eficiente.
 * @tparam T Tipo de objeto a almacenar en el pool
 * @tparam MAX_SIZE Tamaño máximo del pool
 */
template<typename T, size_t MAX_SIZE>
class ObjectPool {
private:
    std::queue<T*> availableObjects;
    std::vector<T*> allObjects;
    
public:
    ObjectPool() {
        // Pre-asignar todos los objetos al crear el pool
        for (size_t i = 0; i < MAX_SIZE; ++i) {
            T* obj = new T();
            availableObjects.push(obj);
            allObjects.push_back(obj);
        }
    }
    
    ~ObjectPool() {
        // Liberar todos los objetos al destruir el pool
        while (!availableObjects.empty()) {
            T* obj = availableObjects.front();
            availableObjects.pop();
            delete obj;
        }
        allObjects.clear();
    }
    
    /**
     * @brief Obtiene un objeto del pool
     * @return Puntero al objeto o nullptr si el pool está vacío
     */
    T* acquire() {
        if (availableObjects.empty()) {
            return nullptr;
        }
        
        T* obj = availableObjects.front();
        availableObjects.pop();
        return obj;
    }
    
    /**
     * @brief Libera un objeto de vuelta al pool
     * @param obj Objeto a liberar
     */
    void release(T* obj) {
        if (obj) {
            availableObjects.push(obj);
        }
    }
    
    /**
     * @brief Obtiene el número de objetos disponibles
     * @return Cantidad de objetos disponibles
     */
    size_t availableCount() const {
        return availableObjects.size();
    }
    
    /**
     * @brief Obtiene el número total de objetos en el pool
     * @return Capacidad total del pool
     */
    size_t totalCount() const {
        return MAX_SIZE;
    }
    
    /**
     * @brief Obtiene el porcentaje de uso del pool
     * @return Porcentaje de objetos en uso (0-100)
     */
    uint8_t usagePercentage() const {
        return static_cast<uint8_t>((MAX_SIZE - availableObjects.size()) * 100 / MAX_SIZE);
    }
    
    // Eliminar copia y asignación
    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;
};

#endif // __OBJECT_POOL_H__
