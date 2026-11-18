
#include "Medicao.hpp"

#include <cmath>
#include <stdexcept>
#include <utility>

Medicao::Medicao(std::string tag,
                 std::string tipo,
                 std::optional<double> valor,
                 std::string unidade,
                 std::string status)
    : tag_(std::move(tag)),
      tipo_(std::move(tipo)),
      valor_(valor),
      unidade_(std::move(unidade)),
      status_(std::move(status)) {

    // Verifica a identificacao e a unidade do sensor.
    if (tag_ == "LT-01") {
        if (tipo_ != "nivel" || unidade_ != "%") {
            throw std::invalid_argument("Dados invalidos do sensor LT-01");
        }
    } else if (tag_ == "TT-01") {
        if (tipo_ != "temperatura" || unidade_ != "C") {
            throw std::invalid_argument("Dados invalidos do sensor TT-01");
        }
    } else {
        throw std::invalid_argument("Sensor desconhecido");
    }

    // Verifica o estado do sensor.
    if (status_ != "operando" && status_ != "falha") {
        throw std::invalid_argument("Status de sensor invalido");
    }

    if (status_ == "operando") {
        if (!valor_.has_value()) {
            throw std::invalid_argument("Sensor operando sem valor");
        }

        if (!std::isfinite(*valor_) || *valor_ < 0.0 || *valor_ > 100.0) {
            throw std::invalid_argument("Valor da medicao fora dos limites");
        }
    } else {
        if (valor_.has_value()) {
            throw std::invalid_argument("Sensor em falha deve ter valor nulo");
        }
    }
}

const std::string& Medicao::tag() const {
    return tag_;
}

const std::string& Medicao::tipo() const {
    return tipo_;
}

const std::optional<double>& Medicao::valor() const {
    return valor_;
}

const std::string& Medicao::unidade() const {
    return unidade_;
}

const std::string& Medicao::status() const {
    return status_;
}

bool Medicao::estaOperando() const {
    return status_ == "operando";
}
