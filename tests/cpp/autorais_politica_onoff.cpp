#include "PoliticaOnOff.hpp"

#include <cassert>
#include <iostream>

namespace {

void verificar(const PoliticaControle& politica, double nivel, double temperatura,
               const Atuacao& aplicada, double bomba, double aquecedor) {
    const Atuacao calculada = politica.calcular(nivel, temperatura, aplicada);
    assert(calculada.bomba() == bomba);
    assert(calculada.aquecedor() == aquecedor);
}

}

int main() {
    const PoliticaOnOff politica;
    const Atuacao desligados(0.0, 0.0);
    const Atuacao ligados(100.0, 100.0);

    // Abaixo do limite inferior, liga mesmo partindo de zero.
    verificar(politica, 64.9, 27.9, desligados, 100.0, 100.0);

    // No limite inferior e no interior da banda, preserva ligado/desligado.
    verificar(politica, 65.0, 28.0, desligados, 0.0, 0.0);
    verificar(politica, 65.0, 28.0, ligados, 100.0, 100.0);
    verificar(politica, 70.0, 30.0, desligados, 0.0, 0.0);
    verificar(politica, 74.9, 31.9, ligados, 100.0, 100.0);

    // No limite superior e acima dele, desliga mesmo partindo de 100%.
    verificar(politica, 75.0, 32.0, ligados, 0.0, 0.0);
    verificar(politica, 75.1, 32.1, ligados, 0.0, 0.0);

    // Potencia parcial conta como ligado. Cada atuador usa a propria potencia.
    verificar(politica, 70.0, 30.0, Atuacao(70.0, 0.0), 100.0, 0.0);
    verificar(politica, 70.0, 30.0, Atuacao(0.0, 50.0), 0.0, 100.0);

    // Cada medicao pode levar a uma decisao diferente.
    verificar(politica, 64.9, 32.0, Atuacao(0.0, 100.0), 100.0, 0.0);
    verificar(politica, 75.0, 27.9, Atuacao(100.0, 0.0), 0.0, 100.0);

    std::cout << "11 cenarios da politica ON/OFF aprovados!\n";
    return 0;
}
