
#pragma once

#include <ctime>
#include <string>

enum class ResultadoTemporal {
    Valido,
    Expirado,
    Futuro
};

class ValidadeTemporal {

public:

    // Tempo maximo de validade: 3 minutos.
    static constexpr int LIMITE_SEGUNDOS = 180;

    static ResultadoTemporal avaliar(
        const std::string& timestamp,
        std::time_t agora);
};

