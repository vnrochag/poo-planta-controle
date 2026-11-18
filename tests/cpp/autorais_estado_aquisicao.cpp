
#include "EstadoAquisicao.hpp"

#include <cassert>
#include <iostream>
#include <stdexcept>

int main() {

    EstadoAquisicao estado;

    // TESTE 1: primeira aquisicao.
    assert(
        estado.avaliar("sessao-A", 1) ==
        ResultadoCiclo::PrimeiraAquisicao);

    estado.registrar("sessao-A", 1);

    // TESTE 2: novo ciclo.
    assert(
        estado.avaliar("sessao-A", 3) ==
        ResultadoCiclo::NovoCiclo);

    estado.registrar("sessao-A", 3);

    // TESTE 3: ciclo repetido.
    assert(
        estado.avaliar("sessao-A", 3) ==
        ResultadoCiclo::CicloRepetido);

    // TESTE 4: ciclo antigo.
    assert(
        estado.avaliar("sessao-A", 2) ==
        ResultadoCiclo::CicloAntigo);

    // TESTE 5: nova sessao.
    assert(
        estado.avaliar("sessao-B", 1) ==
        ResultadoCiclo::NovaSessao);

    estado.registrar("sessao-B", 1);

    // Confirma que a nova sessao foi registrada.
    assert(
        estado.avaliar("sessao-B", 1) ==
        ResultadoCiclo::CicloRepetido);

    // TESTE 6: sessao vazia.
    {
        bool encontrouErro = false;

        try {
            estado.avaliar("", 1);
        } catch (const std::invalid_argument&) {
            encontrouErro = true;
        }

        assert(encontrouErro);
    }

    // TESTE 7: ciclo igual a zero.
    {
        bool encontrouErro = false;

        try {
            estado.avaliar("sessao-B", 0);
        } catch (const std::invalid_argument&) {
            encontrouErro = true;
        }

        assert(encontrouErro);
    }

    // Atualiza o estado para o ciclo 3.
    estado.registrar("sessao-B", 3);

    // TESTE 8: tentativa de registrar ciclo repetido.
    {
        bool encontrouErro = false;

        try {
            estado.registrar("sessao-B", 3);
        } catch (const std::invalid_argument&) {
            encontrouErro = true;
        }

        assert(encontrouErro);
    }

    // TESTE 9: tentativa de registrar ciclo antigo.
    {
        bool encontrouErro = false;

        try {
            estado.registrar("sessao-B", 2);
        } catch (const std::invalid_argument&) {
            encontrouErro = true;
        }

        assert(encontrouErro);
    }

    // Confirma que as falhas nao alteraram
    // o ultimo registro valido.
    assert(
        estado.avaliar("sessao-B", 3) ==
        ResultadoCiclo::CicloRepetido);

    assert(
        estado.avaliar("sessao-B", 4) ==
        ResultadoCiclo::NovoCiclo);

    std::cout
        << "9 cenarios de estado da aquisicao aprovados!\n";

    return 0;
}
