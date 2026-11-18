
#include "ValidadeTemporal.hpp"

#include <cassert>
#include <ctime>
#include <iostream>
#include <stdexcept>
#include <string>

int main() {

    // Timestamp fixo utilizado nos testes.
    const std::string timestamp =
        "2026-01-01T00:00:00Z";

    // Representacao do timestamp em segundos UTC.
    const std::time_t instante = 1767225600;

    // Confirma o limite de validade escolhido.
    assert(ValidadeTemporal::LIMITE_SEGUNDOS == 180);

    // TESTE 1: aquisicao instantanea.
    assert(
        ValidadeTemporal::avaliar(timestamp, instante) ==
        ResultadoTemporal::Valido);

    // TESTE 2: aquisicao com 90 segundos.
    assert(
        ValidadeTemporal::avaliar(timestamp, instante + 90) ==
        ResultadoTemporal::Valido);

    // TESTE 3: aquisicao com 179 segundos.
    assert(
        ValidadeTemporal::avaliar(timestamp, instante + 179) ==
        ResultadoTemporal::Valido);

    // TESTE 4: aquisicao no limite de 180 segundos.
    assert(
        ValidadeTemporal::avaliar(timestamp, instante + 180) ==
        ResultadoTemporal::Valido);

    // TESTE 5: aquisicao acima do limite.
    assert(
        ValidadeTemporal::avaliar(timestamp, instante + 181) ==
        ResultadoTemporal::Expirado);

    // TESTE 6: aquisicao com 300 segundos.
    assert(
        ValidadeTemporal::avaliar(timestamp, instante + 300) ==
        ResultadoTemporal::Expirado);

    // TESTE 7: timestamp no futuro.
    assert(
        ValidadeTemporal::avaliar(timestamp, instante - 10) ==
        ResultadoTemporal::Futuro);

    // TESTE 8: timestamp malformado.
    {
        bool encontrouErro = false;

        try {
            ValidadeTemporal::avaliar(
                "timestamp-invalido",
                instante);

        } catch (const std::invalid_argument&) {
            encontrouErro = true;
        }

        assert(encontrouErro);
    }

    std::cout
        << "8 cenarios de validade temporal aprovados!\n";

    return 0;
}
