#include "Atuacao.hpp"

#include <cassert>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace {

void deveRejeitar(double bomba, double aquecedor) {
    bool rejeitou = false;
    try {
        Atuacao atuacao(bomba, aquecedor);
    } catch (const std::invalid_argument&) {
        rejeitou = true;
    }
    assert(rejeitou);
}

}

int main() {
    const Atuacao parcial(70.0, 42.5);
    assert(parcial.bomba() == 70.0);
    assert(parcial.aquecedor() == 42.5);

    const Atuacao bombaNoMinimo(0.0, 100.0);
    assert(bombaNoMinimo.bomba() == 0.0);
    assert(bombaNoMinimo.aquecedor() == 100.0);

    const Atuacao bombaNoMaximo(100.0, 0.0);
    assert(bombaNoMaximo.bomba() == 100.0);
    assert(bombaNoMaximo.aquecedor() == 0.0);

    const double infinito = std::numeric_limits<double>::infinity();
    const double invalidos[] = {
        -0.1, 100.1, std::numeric_limits<double>::quiet_NaN(), infinito, -infinito
    };

    for (double invalido : invalidos) {
        deveRejeitar(invalido, 50.0);
        deveRejeitar(50.0, invalido);
    }

    std::cout << "13 cenarios de Atuacao aprovados!\n";
    return 0;
}
