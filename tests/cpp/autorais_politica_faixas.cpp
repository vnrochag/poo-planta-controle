#include "PoliticaFaixas.hpp"
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
    const PoliticaFaixas faixas;
    const PoliticaOnOff onOff;
    const Atuacao desligados(0.0, 0.0);

    // Interior de cada faixa dos dois atuadores.
    verificar(faixas, 2.0, 20.0, desligados, 100.0, 100.0);
    verificar(faixas, 30.0, 24.0, desligados, 70.0, 70.0);
    verificar(faixas, 60.0, 28.0, desligados, 50.0, 50.0);
    verificar(faixas, 80.0, 35.0, desligados, 0.0, 0.0);

    // Logo abaixo e no valor exato de cada transicao.
    verificar(faixas, 4.999, 21.999, desligados, 100.0, 100.0);
    verificar(faixas, 5.0, 22.0, desligados, 70.0, 70.0);
    verificar(faixas, 49.999, 25.999, desligados, 70.0, 70.0);
    verificar(faixas, 50.0, 26.0, desligados, 50.0, 50.0);
    verificar(faixas, 69.999, 29.999, desligados, 50.0, 50.0);
    verificar(faixas, 70.0, 30.0, desligados, 0.0, 0.0);

    // Mesma entrada e mesma interface, com decisoes diferentes por estrategia.
    const Atuacao aplicada(70.0, 0.0);
    verificar(faixas, 68.0, 31.0, aplicada, 50.0, 0.0);
    verificar(onOff, 68.0, 31.0, aplicada, 100.0, 0.0);

    // Por faixas, alterar a atuacao observada nao muda o resultado.
    verificar(faixas, 68.0, 31.0, Atuacao(0.0, 100.0), 50.0, 0.0);

    std::cout << "13 cenarios de faixas e interface comum aprovados!\n";
    return 0;
}
