
#include "Aquisicao.hpp"
#include "EstadoAquisicao.hpp"
#include "Telemetria.hpp"
#include "ValidadeTemporal.hpp"

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <filesystem>
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

// Processa uma aquisicao da planta.
int processarAquisicao(
    const std::filesystem::path& diretorio,
    bool umaVez) {

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

        // Verifica o funcionamento dos sensores.
        if (!dados.nivel.estaOperando() ||
            !dados.temperatura.estaOperando()) {

            std::cerr
                << "Erro: sensor em falha. "
                << "Controle nao autorizado.\n";

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

        // Publica a telemetria validada.
        Telemetria::publicar(dados, caminhoTelemetria);

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

        // Interpreta os argumentos da CLI.
        for (int i = 1; i < argc; ++i) {

            std::string argumento = argv[i];

            if (argumento == "--ajuda") {

                std::cout
                    << "Uso: controlador "
                    << "--dados DIRETORIO [--uma-vez]\n";

                return EXIT_SUCCESS;

            } else if (argumento == "--dados") {

                if (i + 1 >= argc) {
                    throw std::invalid_argument(
                        "Informe o diretorio apos --dados");
                }

                diretorio = argv[++i];

            } else if (argumento == "--uma-vez") {

                umaVez = true;

            } else {

                throw std::invalid_argument(
                    "Argumento desconhecido: " + argumento);
            }
        }

        // Executa somente uma aquisicao, sem esperar.
        if (umaVez) {
            return processarAquisicao(diretorio, true);
        }

        // Executa continuamente.
        std::cout
            << "Controlador iniciado em modo continuo.\n";

        while (true) {

            processarAquisicao(diretorio, false);

            std::this_thread::sleep_for(
                std::chrono::seconds(1));
        }

    } catch (const std::exception& erro) {

        std::cerr << "Erro: " << erro.what() << '\n';

        return EXIT_FAILURE;
    }
}
