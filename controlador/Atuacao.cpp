#include "Atuacao.hpp"

#include <cmath>
#include <stdexcept>

namespace {

bool percentualValido(double percentual) {
    return std::isfinite(percentual) && percentual >= 0.0 && percentual <= 100.0;
}

}

Atuacao::Atuacao(double bomba, double aquecedor)
    : bomba_(bomba), aquecedor_(aquecedor) {
    if (!percentualValido(bomba_) || !percentualValido(aquecedor_)) {
        throw std::invalid_argument(
            "Os percentuais de bomba e aquecedor devem ser finitos e estar entre 0 e 100");
    }
}

double Atuacao::bomba() const {
    return bomba_;
}

double Atuacao::aquecedor() const {
    return aquecedor_;
}
