#pragma once

#include "Atuacao.hpp"

class PoliticaControle {
public:
    virtual ~PoliticaControle() = default;

    // Recebe medicoes operacionais ja validadas: nivel em % e temperatura em C.
    virtual Atuacao calcular(double nivel, double temperatura,
                             const Atuacao& aplicada) const = 0;
};
