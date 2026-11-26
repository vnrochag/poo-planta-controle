#pragma once

#include "PoliticaControle.hpp"

class PoliticaFaixas : public PoliticaControle {
public:
    Atuacao calcular(double nivel, double temperatura,
                     const Atuacao& aplicada) const override;
};
