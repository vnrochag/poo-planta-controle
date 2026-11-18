
#include "EstadoAquisicao.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

// Verifica se um arquivo de estado invalido
// e rejeitado corretamente.
void deveFalharAoCarregar(
    EstadoAquisicao& estado,
    const std::filesystem::path& caminho,
    const std::string& mensagemEsperada) {

    bool encontrouErro = false;

    try {
        estado.carregar(caminho);
    } catch (const std::runtime_error& erro) {
        encontrouErro =
            std::string(erro.what()).find(mensagemEsperada)
            != std::string::npos;
    }

    assert(encontrouErro);
}

int main() {

    const std::filesystem::path caminho =
        "build/teste_estado_persistencia.json";

    std::filesystem::remove(caminho);

    EstadoAquisicao estado;

    // TESTE 1: arquivo de estado inexistente.
    estado.carregar(caminho);

    assert(
        estado.avaliar("sessao-A", 1) ==
        ResultadoCiclo::PrimeiraAquisicao);

    // TESTE 2: salvar estado valido.
    estado.registrar("sessao-A", 3);

    estado.salvar(caminho);

    assert(std::filesystem::exists(caminho));

    // TESTE 3: recuperar estado salvo.
    EstadoAquisicao estadoRecuperado;

    estadoRecuperado.carregar(caminho);

    assert(
        estadoRecuperado.avaliar("sessao-A", 3) ==
        ResultadoCiclo::CicloRepetido);

    assert(
        estadoRecuperado.avaliar("sessao-A", 2) ==
        ResultadoCiclo::CicloAntigo);

    assert(
        estadoRecuperado.avaliar("sessao-A", 4) ==
        ResultadoCiclo::NovoCiclo);

    // TESTE 4: atualizar o estado salvo.
    estadoRecuperado.registrar("sessao-A", 5);

    estadoRecuperado.salvar(caminho);

    EstadoAquisicao estadoAtualizado;

    estadoAtualizado.carregar(caminho);

    assert(
        estadoAtualizado.avaliar("sessao-A", 5) ==
        ResultadoCiclo::CicloRepetido);

    assert(
        estadoAtualizado.avaliar("sessao-A", 4) ==
        ResultadoCiclo::CicloAntigo);

    // Verifica se o arquivo temporario foi removido.
    std::filesystem::path temporario = caminho;
    temporario += ".tmp";

    assert(!std::filesystem::exists(temporario));

    // TESTE 5: JSON corrompido.
    {
        std::ofstream arquivo(caminho);
        assert(arquivo.is_open());

        arquivo << R"({"sessao": )";
    }

    deveFalharAoCarregar(
        estadoAtualizado,
        caminho,
        "Arquivo de estado JSON invalido");

    // TESTE 6: sessao vazia.
    {
        std::ofstream arquivo(caminho);
        assert(arquivo.is_open());

        arquivo << R"({
            "sessao": "",
            "ciclo": 5
        })";
    }

    deveFalharAoCarregar(
        estadoAtualizado,
        caminho,
        "Sessao do estado nao pode ser vazia");

    // TESTE 7: ciclo negativo.
    {
        std::ofstream arquivo(caminho);
        assert(arquivo.is_open());

        arquivo << R"({
            "sessao": "sessao-A",
            "ciclo": -1
        })";
    }

    deveFalharAoCarregar(
        estadoAtualizado,
        caminho,
        "Ciclo do estado deve ser positivo");

    // Confirma que as falhas de carregamento
    // nao alteraram o ultimo estado valido.
    assert(
        estadoAtualizado.avaliar("sessao-A", 5) ==
        ResultadoCiclo::CicloRepetido);

    // TESTE 8: salvar objeto sem registro.
    {
        EstadoAquisicao estadoVazio;

        bool encontrouErro = false;

        try {
            estadoVazio.salvar(caminho);
        } catch (const std::runtime_error& erro) {
            encontrouErro =
                std::string(erro.what()).find(
                    "Nao existe estado de aquisicao para salvar")
                != std::string::npos;
        }

        assert(encontrouErro);
    }

    // Remove os arquivos utilizados nos testes.
    std::filesystem::remove(caminho);
    std::filesystem::remove(temporario);

    std::cout
        << "8 cenarios de persistencia aprovados!\n";

    return 0;
}
