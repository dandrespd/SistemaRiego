import os
import re

# Mapeo de archivos y correcciones de rutas
fix_map = {
    'src/core/ConfigManager.cpp': [
        (r'#include "../../include/core/ConfigManager.h"', 
         '#include <core/ConfigManager.h>')
    ],
    'src/drivers/IN_DIGITAL.cpp': [
        (r'#include "drivers/IN_DIGITAL.h"', 
         '#include <drivers/IN_DIGITAL.h>')
    ],
    'src/drivers/OUT_DIGITAL.cpp': [
        (r'#include "drivers/OUT_DIGITAL.h"', 
         '#include <drivers/OUT_DIGITAL.h>')
    ],
    'src/drivers/I2CManager.cpp': [
        (r'#include "../../include/drivers/I2CManager.h"', 
         '#include <drivers/I2CManager.h>')
    ],
    'src/core/SystemManager.cpp': [
        (r'#include "core/SystemManager.h"', 
         '#include <core/SystemManager.h>')
    ],
    'src/drivers/Led.cpp': [
        (r'#include "drivers/Led.h"', 
         '#include <drivers/Led.h>')
    ]
}

def fix_file(file_path, patterns):
    with open(file_path, 'r') as f:
        content = f.read()
    
    for pattern, replacement in patterns:
        content = re.sub(pattern, replacement, content)
    
    with open(file_path, 'w') as f:
        f.write(content)

# Aplicar correcciones
for file_path, patterns in fix_map.items():
    if os.path.exists(file_path):
        fix_file(file_path, patterns)
        print(f"Fixed: {file_path}")
    else:
        print(f"File not found: {file_path}")

print("Rutas actualizadas correctamente")
