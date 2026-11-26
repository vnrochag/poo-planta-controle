#include "AvaliadorAlarmes.hpp"

#include <cassert>
#include <initializer_list>
#include <iostream>
#include <optional>

namespace {

void verificar(std::optional<double> nivel, std::optional<double> temperatura,
               std::initializer_list<Alarme> esperados) {
    const Medicao leituraNivel("LT-01", "nivel", nivel, "%", nivel ? "operando" : "falha");
    const Medicao leituraTemperatura("TT-01", "temperatura", temperatura, "C",
                                    temperatura ? "operando" : "falha");
    const auto alarmes = AvaliadorAlarmes::avaliar(leituraNivel, leituraTemperatura);

    assert(alarmes.size() == esperados.size());
    auto alarme = alarmes.begin();
    for (const auto& esperado : esperados) {
        assert(alarme->codigo == esperado.codigo);
        assert(alarme->mensagem == esperado.mensagem);
        assert(alarme->severidade == esperado.severidade);
        ++alarme;
    }
}

}

int main() {
    const Alarme nivelCritico{"NIVEL_CRITICO", "Nivel abaixo de 5%", "critico"};
    const Alarme nivelBaixo{"NIVEL_BAIXO", "Nivel entre 5% e abaixo de 20%", "aviso"};
    const Alarme nivelAlto{"NIVEL_ALTO", "Nivel acima de 90%", "aviso"};
    const Alarme tempBaixa{"TEMP_BAIXA", "Temperatura abaixo de 22 C", "aviso"};
    const Alarme tempAlta{"TEMP_ALTA", "Temperatura acima de 40 C", "aviso"};

    verificar(70.0, 30.0, {});
    verificar(4.999, 30.0, {nivelCritico});
    verificar(5.0, 30.0, {nivelBaixo});
    verificar(19.999, 30.0, {nivelBaixo});
    verificar(20.0, 30.0, {});
    verificar(90.0, 30.0, {});
    verificar(90.001, 30.0, {nivelAlto});
    verificar(70.0, 21.999, {tempBaixa});
    verificar(70.0, 22.0, {});
    verificar(70.0, 40.0, {});
    verificar(70.0, 40.001, {tempAlta});
    verificar(0.0, 45.0, {nivelCritico, tempAlta});
    verificar(std::nullopt, 18.0, {tempBaixa});
    verificar(95.0, std::nullopt, {nivelAlto});
    verificar(std::nullopt, std::nullopt, {});

    std::cout << "15 cenarios do avaliador de alarmes aprovados!\n";
}
