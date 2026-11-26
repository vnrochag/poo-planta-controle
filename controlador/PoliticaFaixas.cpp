#include "PoliticaFaixas.hpp"

namespace {

double calcularPorFaixas(double medicao, double limitePotenciaMaxima,
                         double limitePotenciaAlta, double limiteDesligamento) {
    if (medicao < limitePotenciaMaxima) {
        return 100.0;
    }
    if (medicao < limitePotenciaAlta) {
        return 70.0;
    }
    if (medicao < limiteDesligamento) {
        return 50.0;
    }
    return 0.0;
}

}

Atuacao PoliticaFaixas::calcular(double nivel, double temperatura,
                                const Atuacao& /* aplicada */) const {
    return Atuacao(
        calcularPorFaixas(nivel, 5.0, 50.0, 70.0),
        calcularPorFaixas(temperatura, 22.0, 26.0, 30.0));
}
