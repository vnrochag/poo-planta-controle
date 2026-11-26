#include "PoliticaOnOff.hpp"

namespace {

double calcularComHisterese(double medicao, double limiteInferior,
                            double limiteSuperior, double potenciaAplicada) {
    if (medicao < limiteInferior) {
        return 100.0;
    }
    if (medicao >= limiteSuperior) {
        return 0.0;
    }
    // Na banda, qualquer potencia positiva observada representa ligado.
    return potenciaAplicada > 0.0 ? 100.0 : 0.0;
}

}

Atuacao PoliticaOnOff::calcular(double nivel, double temperatura,
                               const Atuacao& aplicada) const {
    return Atuacao(
        calcularComHisterese(nivel, 65.0, 75.0, aplicada.bomba()),
        calcularComHisterese(temperatura, 28.0, 32.0, aplicada.aquecedor()));
}
