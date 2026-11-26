
#include "Telemetria.hpp"

#include "AvaliadorAlarmes.hpp"

#include <nlohmann/json.hpp>

#include <fstream>
#include <stdexcept>
#include <system_error>

// Converte uma medicao em um objeto JSON.
namespace {

nlohmann::json converterMedicao(
    const Medicao& medicao) {

    nlohmann::json valor = nullptr;

    if (medicao.valor().has_value()) {
        valor = *medicao.valor();
    }

    return {
        {"tag", medicao.tag()},
        {"tipo", medicao.tipo()},
        {"valor", valor},
        {"unidade", medicao.unidade()},
        {"status", medicao.status()}
    };
}

}

void Telemetria::publicar(
    const DadosPlanta& dados,
    const std::filesystem::path& caminho) {

    // Preserva o diagnostico da planta.
    nlohmann::json erroComando = nullptr;

    if (dados.erroComando.has_value()) {
        erroComando = *dados.erroComando;
    }

    // Preserva o motivo do estado seguro.
    nlohmann::json motivoSeguro = nullptr;

    if (dados.motivoSeguro.has_value()) {
        motivoSeguro = *dados.motivoSeguro;
    }

    auto alarmes = nlohmann::json::array();
    for (const auto& alarme : AvaliadorAlarmes::avaliar(dados.nivel, dados.temperatura)) {
        alarmes.push_back({
            {"codigo", alarme.codigo},
            {"mensagem", alarme.mensagem},
            {"severidade", alarme.severidade}
        });
    }

    // Monta a telemetria conforme o contrato.
    const nlohmann::json documento = {
        {"versao", dados.versao},
        {"sessao", dados.sessao},
        {"ciclo", dados.ciclo},
        {"timestamp", dados.timestamp},

        {"leituras", nlohmann::json::array({
            converterMedicao(dados.nivel),
            converterMedicao(dados.temperatura)
        })},

        {"atuadores", {
            {"bomba", dados.bombaAplicada},
            {"aquecedor", dados.aquecedorAplicado}
        }},

        {"ultimo_comando", dados.ultimoComando},
        {"erro_comando", erroComando},
        {"motivo_seguro", motivoSeguro},

        {"alarmes", alarmes}
    };

    // Define o caminho do arquivo temporario.
    std::filesystem::path temporario = caminho;
    temporario += ".tmp";

    try {

        // Grava primeiro no arquivo temporario.
        {
            std::ofstream arquivo(temporario);

            if (!arquivo.is_open()) {
                throw std::runtime_error(
                    "Nao foi possivel criar a telemetria temporaria");
            }

            arquivo << documento.dump(4) << '\n';

            arquivo.close();

            if (!arquivo) {
                throw std::runtime_error(
                    "Falha ao gravar a telemetria");
            }
        }

        // Publica o documento completo.
        std::filesystem::rename(temporario, caminho);

    } catch (...) {

        // Remove o arquivo temporario em caso de falha.
        std::error_code erro;
        std::filesystem::remove(temporario, erro);

        throw;
    }
}
