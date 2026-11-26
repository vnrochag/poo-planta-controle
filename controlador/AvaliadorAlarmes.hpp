#pragma once

#include "Medicao.hpp"

#include <string>
#include <vector>

struct Alarme {
    std::string codigo;
    std::string mensagem;
    std::string severidade;
};

class AvaliadorAlarmes {
public:
    static std::vector<Alarme> avaliar(const Medicao& nivel,
                                      const Medicao& temperatura);
};
