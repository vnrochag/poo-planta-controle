
#include "Medicao.hpp"

#include <cassert>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

// Verifica se a classe rejeita uma medicao invalida.
void deveRejeitar(const std::string& tag,
                  const std::string& tipo,
                  std::optional<double> valor,
                  const std::string& unidade,
                  const std::string& status) {
    bool rejeitou = false;

    try {
        Medicao medicao(tag, tipo, valor, unidade, status);
    } catch (const std::invalid_argument&) {
        rejeitou = true;
    }

    assert(rejeitou);
}

int main() {
    // Teste 1: medicao valida de nivel.
    Medicao nivel("LT-01", "nivel", 48.5, "%", "operando");

    assert(nivel.tag() == "LT-01");
    assert(nivel.tipo() == "nivel");
    assert(nivel.valor().has_value());
    assert(nivel.valor().value() == 48.5);
    assert(nivel.unidade() == "%");
    assert(nivel.estaOperando());

    // Teste 2: medicao valida de temperatura.
    Medicao temperatura("TT-01", "temperatura", 26.2, "C", "operando");

    assert(temperatura.valor().value() == 26.2);
    assert(temperatura.estaOperando());

    // Teste 3: valores de fronteira permitidos.
    Medicao minimo("LT-01", "nivel", 0.0, "%", "operando");
    Medicao maximo("LT-01", "nivel", 100.0, "%", "operando");

    assert(minimo.valor().value() == 0.0);
    assert(maximo.valor().value() == 100.0);

    // Teste 4: valores fora dos limites.
    deveRejeitar("LT-01", "nivel", -1.0, "%", "operando");
    deveRejeitar("LT-01", "nivel", 101.0, "%", "operando");

    // Teste 5: unidade incorreta.
    deveRejeitar("TT-01", "temperatura", 26.2, "%", "operando");

    // Teste 6: sensor desconhecido.
    deveRejeitar("XX-01", "nivel", 50.0, "%", "operando");

    // Teste 7: sensor operando sem valor.
    deveRejeitar("LT-01", "nivel", std::nullopt, "%", "operando");

    // Teste 8: sensor em falha sem valor.
    Medicao falha("LT-01", "nivel", std::nullopt, "%", "falha");

    assert(!falha.estaOperando());
    assert(!falha.valor().has_value());

    // Teste 9: sensor em falha com valor indevido.
    deveRejeitar("LT-01", "nivel", 30.0, "%", "falha");

    std::cout << "Testes autorais de Medicao aprovados!\n";

    return 0;
}