
#include "Aquisicao.hpp"
#include "EstadoAquisicao.hpp"
#include "PoliticaFaixas.hpp"
#include "PoliticaOnOff.hpp"
#include "Telemetria.hpp"
#include "ValidadeTemporal.hpp"

#include <nlohmann/json.hpp>

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

// Exibe uma medicao recebida da planta.
void exibirMedicao(const Medicao& medicao) {

    std::cout << medicao.tag() << ": ";

    if (medicao.estaOperando()) {
        std::cout
            << *medicao.valor()
            << " "
            << medicao.unidade();
    } else {
        std::cout << "FALHA";
    }

    std::cout << '\n';
}

// Verifica se a solicitacao recuperada ja esta disponivel para a planta.
bool comandoPublicado(const Comando& comando, const std::filesystem::path& caminho) {
    std::ifstream arquivo(caminho);
    if (!arquivo.is_open()) {
        return false;
    }

    const auto documento = nlohmann::json::parse(arquivo, nullptr, false);
    return documento.is_object() &&
           documento.contains("versao") && documento.at("versao").is_number_integer() &&
           documento.contains("id") && documento.at("id").is_number_integer() &&
           documento == nlohmann::json{
               {"versao", 1},
               {"sessao", comando.sessao()},
               {"id", comando.id()},
               {"atuadores", {
                   {"bomba", comando.atuacao().bomba()},
                   {"aquecedor", comando.atuacao().aquecedor()}
               }}
           };
}

// Processa uma aquisicao da planta.
int processarAquisicao(
    const std::filesystem::path& diretorio,
    bool umaVez,
    const PoliticaControle& politica) {

    try {

        const std::filesystem::path caminho =
            diretorio / "planta.json";

        const std::filesystem::path caminhoEstado =
            diretorio / "controlador_estado.json";

        const std::filesystem::path caminhoTelemetria =
            diretorio / "telemetria.json";

        // Le e valida os dados da planta.
        DadosPlanta dados =
            Aquisicao::lerArquivo(caminho);

        // Verifica a validade temporal.
        ResultadoTemporal temporal =
            ValidadeTemporal::avaliar(
                dados.timestamp,
                std::time(nullptr));

        if (temporal == ResultadoTemporal::Expirado) {

            std::cerr
                << "Aquisicao expirada. Dados antigos.\n";

            return EXIT_FAILURE;
        }

        if (temporal == ResultadoTemporal::Futuro) {

            std::cerr
                << "Timestamp no futuro. Aquisicao rejeitada.\n";

            return EXIT_FAILURE;
        }

        // Recupera o ultimo registro salvo.
        EstadoAquisicao estado;

        estado.carregar(caminhoEstado);

        ResultadoCiclo resultado =
            estado.avaliar(dados.sessao, dados.ciclo);

        // Classifica a aquisicao.
        switch (resultado) {

            case ResultadoCiclo::PrimeiraAquisicao:

                std::cout
                    << "Primeira aquisicao identificada.\n";

                break;

            case ResultadoCiclo::NovoCiclo:

                std::cout
                    << "Novo ciclo identificado.\n";

                break;

            case ResultadoCiclo::NovaSessao:

                std::cout
                    << "Nova sessao da planta identificada.\n";

                break;

            case ResultadoCiclo::CicloRepetido:

                if (umaVez) {
                    std::cout
                        << "Ciclo repetido. "
                        << "Aquisicao ignorada.\n";
                }

                return EXIT_SUCCESS;

            case ResultadoCiclo::CicloAntigo:

                std::cerr
                    << "Ciclo antigo. "
                    << "Aquisicao rejeitada.\n";

                return EXIT_FAILURE;
        }

        const auto pendenteAnterior = estado.pendente();
        estado.reconciliar(dados.sessao, dados.ciclo, dados.ultimoComando);
        if (pendenteAnterior && pendenteAnterior->comando.sessao() == dados.sessao) {
            const auto id = static_cast<std::uint64_t>(pendenteAnterior->comando.id());
            if (dados.ultimoComando == id) {
                std::cout << "Comando confirmado pela planta: " << id << '\n';
            } else if (dados.ultimoComando > id) {
                std::cout << "Comando superado sem confirmacao observada: " << id << '\n';
            }
        }

        // A telemetria mostra o snapshot da planta, inclusive uma medicao em falha.
        Telemetria::publicar(dados, caminhoTelemetria);

        const auto caminhoComando = diretorio / "comando_planta.json";
        if (dados.nivel.estaOperando() && dados.temperatura.estaOperando()) {
            if (const auto pendente = estado.pendente()) {
                if (!comandoPublicado(pendente->comando, caminhoComando)) {
                    pendente->comando.publicar(caminhoComando);
                    std::cout << "Comando pendente recuperado: " << pendente->comando.id() << '\n';
                }
            } else {
                const Atuacao aplicada(dados.bombaAplicada, dados.aquecedorAplicado);
                const Atuacao solicitada = politica.calcular(
                    *dados.nivel.valor(), *dados.temperatura.valor(), aplicada);
                const Comando comando = estado.reservarComando(
                    dados.sessao, dados.ciclo, solicitada);

                // Salva a identidade e os percentuais antes de tornar o pedido visivel.
                estado.salvar(caminhoEstado);
                comando.publicar(caminhoComando);
                std::cout << "Comando publicado, aguardando confirmacao: "
                          << comando.id() << '\n';
            }
        } else {
            std::cerr << "Sensor em falha. Nenhum comando publicado.\n";
        }

        // Registra e salva a aquisicao processada.
        estado.registrar(dados.sessao, dados.ciclo);
        estado.salvar(caminhoEstado);

        // Exibe os dados recebidos.
        std::cout << "Aquisicao realizada!\n";
        std::cout << "Sessao: " << dados.sessao << '\n';
        std::cout << "Ciclo: " << dados.ciclo << '\n';
        std::cout << "Timestamp: " << dados.timestamp << '\n';

        std::cout << "\nMedicoes:\n";

        exibirMedicao(dados.nivel);
        exibirMedicao(dados.temperatura);

        std::cout << "\nAtuadores aplicados:\n";

        std::cout
            << "Bomba: "
            << dados.bombaAplicada
            << " %\n";

        std::cout
            << "Aquecedor: "
            << dados.aquecedorAplicado
            << " %\n";

        std::cout
            << "\nUltimo comando aceito: "
            << dados.ultimoComando
            << '\n';

        std::cout
            << "Telemetria publicada com sucesso.\n";

        std::cout
            << "Estado da aquisicao salvo com sucesso.\n\n";

        return EXIT_SUCCESS;

    } catch (const std::exception& erro) {

        std::cerr
            << "Erro na aquisicao: "
            << erro.what()
            << '\n';

        return EXIT_FAILURE;
    }
}

int main(int argc, char* argv[]) {

    try {

        std::filesystem::path diretorio = "dados";
        bool umaVez = false;
        std::string nomePolitica = "on-off";

        // Interpreta os argumentos da CLI.
        for (int i = 1; i < argc; ++i) {

            std::string argumento = argv[i];

            if (argumento == "--ajuda") {

                std::cout
                    << "Uso: controlador "
                    << "--dados DIRETORIO [--uma-vez] [--politica on-off|faixas]\n";

                return EXIT_SUCCESS;

            } else if (argumento == "--dados") {

                if (i + 1 >= argc) {
                    throw std::invalid_argument(
                        "Informe o diretorio apos --dados");
                }

                diretorio = argv[++i];

            } else if (argumento == "--politica") {

                if (i + 1 >= argc) {
                    throw std::invalid_argument("Informe on-off ou faixas apos --politica");
                }
                nomePolitica = argv[++i];
                if (nomePolitica != "on-off" && nomePolitica != "faixas") {
                    throw std::invalid_argument("Politica desconhecida: " + nomePolitica);
                }

            } else if (argumento == "--uma-vez") {

                umaVez = true;

            } else {

                throw std::invalid_argument(
                    "Argumento desconhecido: " + argumento);
            }
        }

        const PoliticaOnOff onOff;
        const PoliticaFaixas faixas;
        const PoliticaControle& politica = nomePolitica == "faixas"
            ? static_cast<const PoliticaControle&>(faixas)
            : static_cast<const PoliticaControle&>(onOff);

        // Executa somente uma aquisicao, sem esperar.
        if (umaVez) {
            return processarAquisicao(diretorio, true, politica);
        }

        // Executa continuamente.
        std::cout
            << "Controlador iniciado em modo continuo.\n";

        while (true) {

            processarAquisicao(diretorio, false, politica);

            std::this_thread::sleep_for(
                std::chrono::seconds(1));
        }

    } catch (const std::exception& erro) {

        std::cerr << "Erro: " << erro.what() << '\n';

        return EXIT_FAILURE;
    }
}
