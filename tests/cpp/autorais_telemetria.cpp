
#include "Telemetria.hpp"

#include <nlohmann/json.hpp>

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

// Le o arquivo de telemetria produzido.
nlohmann::json lerTelemetria(
    const std::filesystem::path& caminho) {

    std::ifstream arquivo(caminho);

    assert(arquivo.is_open());

    return nlohmann::json::parse(arquivo);
}

int main() {

    const std::filesystem::path caminho =
        "build/teste_telemetria.json";

    std::filesystem::path temporario = caminho;
    temporario += ".tmp";

    // Remove arquivos de testes anteriores.
    std::filesystem::remove(caminho);
    std::filesystem::remove(temporario);

    // Cria uma aquisicao valida.
    DadosPlanta dados{
        1,
        "sessao-teste",
        12,
        "2026-11-18T20:00:00Z",

        Medicao(
            "LT-01",
            "nivel",
            48.5,
            "%",
            "operando"),

        Medicao(
            "TT-01",
            "temperatura",
            26.2,
            "C",
            "operando"),

        75.0,
        20.0,
        1,
        std::nullopt,
        std::nullopt
    };

    // TESTE 1: publicar telemetria valida.
    Telemetria::publicar(dados, caminho);

    assert(std::filesystem::exists(caminho));

    // Verifica se o arquivo temporario foi removido.
    assert(!std::filesystem::exists(temporario));

    // TESTE 2: preservar os dados da aquisicao.
    nlohmann::json documento = lerTelemetria(caminho);

    assert(documento.at("versao") == 1);
    assert(documento.at("sessao") == "sessao-teste");
    assert(documento.at("ciclo") == 12);

    assert(
        documento.at("timestamp") ==
        "2026-11-18T20:00:00Z");

    // Verifica os sensores.
    assert(documento.at("leituras").is_array());
    assert(documento.at("leituras").size() == 2);

    const auto& nivel = documento.at("leituras").at(0);

    assert(nivel.at("tag") == "LT-01");
    assert(nivel.at("tipo") == "nivel");
    assert(nivel.at("valor") == 48.5);
    assert(nivel.at("unidade") == "%");
    assert(nivel.at("status") == "operando");

    const auto& temperatura =
        documento.at("leituras").at(1);

    assert(temperatura.at("tag") == "TT-01");
    assert(temperatura.at("tipo") == "temperatura");
    assert(temperatura.at("valor") == 26.2);
    assert(temperatura.at("unidade") == "C");
    assert(temperatura.at("status") == "operando");

    // Verifica os atuadores.
    assert(
        documento.at("atuadores").at("bomba")
        == 75.0);

    assert(
        documento.at("atuadores").at("aquecedor")
        == 20.0);

    // Verifica os campos de comando.
    assert(documento.at("ultimo_comando") == 1);
    assert(documento.at("erro_comando").is_null());
    assert(documento.at("motivo_seguro").is_null());

    // Verifica a lista inicial de alarmes.
    assert(documento.at("alarmes").is_array());
    assert(documento.at("alarmes").empty());

    // TESTE 3: substituir a telemetria anterior.
    dados.ciclo = 13;
    dados.timestamp = "2026-11-18T20:00:01Z";
    dados.bombaAplicada = 0.0;
    dados.aquecedorAplicado = 0.0;

    dados.erroComando = "Comando invalido";
    dados.motivoSeguro = "comando_invalido";

    Telemetria::publicar(dados, caminho);

    documento = lerTelemetria(caminho);

    assert(documento.at("ciclo") == 13);

    assert(
        documento.at("timestamp") ==
        "2026-11-18T20:00:01Z");

    assert(
        documento.at("atuadores").at("bomba")
        == 0.0);

    assert(
        documento.at("atuadores").at("aquecedor")
        == 0.0);

    assert(
        documento.at("erro_comando") ==
        "Comando invalido");

    assert(
        documento.at("motivo_seguro") ==
        "comando_invalido");

    assert(!std::filesystem::exists(temporario));

    // TESTE 4: falha na gravacao.
    {
        const std::filesystem::path caminhoInvalido =
            "build/teste_telemetria_diretorio_inexistente/"
            "telemetria.json";

        assert(
            !std::filesystem::exists(
                caminhoInvalido.parent_path()));

        bool encontrouErro = false;

        try {

            Telemetria::publicar(
                dados,
                caminhoInvalido);

        } catch (const std::runtime_error&) {

            encontrouErro = true;
        }

        assert(encontrouErro);

        assert(
            !std::filesystem::exists(caminhoInvalido));
    }

    // Remove o arquivo utilizado nos testes.
    std::filesystem::remove(caminho);

    std::cout
        << "4 cenarios de telemetria aprovados!\n";

    return 0;
}
