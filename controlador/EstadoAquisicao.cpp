
#include "EstadoAquisicao.hpp"

#include <nlohmann/json.hpp>

#include <fstream>
#include <stdexcept>

ResultadoCiclo EstadoAquisicao::avaliar(
    const std::string& sessao,
    std::uint64_t ciclo) const {

    // Valida os dados recebidos.
    if (sessao.empty()) {
        throw std::invalid_argument(
            "Sessao nao pode ser vazia");
    }

    if (ciclo == 0) {
        throw std::invalid_argument(
            "Ciclo deve ser positivo");
    }

    // Ainda nao existe uma aquisicao registrada.
    if (!ultimoRegistro_.has_value()) {
        return ResultadoCiclo::PrimeiraAquisicao;
    }

    // A planta iniciou uma nova sessao.
    if (sessao != ultimoRegistro_->sessao) {
        return ResultadoCiclo::NovaSessao;
    }

    // O ciclo ja foi processado.
    if (ciclo == ultimoRegistro_->ciclo) {
        return ResultadoCiclo::CicloRepetido;
    }

    // O ciclo recebido e anterior ao ultimo registrado.
    if (ciclo < ultimoRegistro_->ciclo) {
        return ResultadoCiclo::CicloAntigo;
    }

    // O ciclo pertence a mesma sessao e e mais recente.
    return ResultadoCiclo::NovoCiclo;
}

void EstadoAquisicao::registrar(
    const std::string& sessao,
    std::uint64_t ciclo) {

    ResultadoCiclo resultado = avaliar(sessao, ciclo);

    // Impede o registro de ciclos repetidos ou antigos.
    if (resultado == ResultadoCiclo::CicloRepetido ||
        resultado == ResultadoCiclo::CicloAntigo) {

        throw std::invalid_argument(
            "Nao e permitido registrar ciclo repetido ou antigo");
    }

    // Atualiza o ultimo registro.
    ultimoRegistro_ = Registro{sessao, ciclo};
}

void EstadoAquisicao::carregar(
    const std::filesystem::path& caminho) {

    // Se o arquivo nao existe, inicia sem registro.
    if (!std::filesystem::exists(caminho)) {
        ultimoRegistro_.reset();
        return;
    }

    // Abre o arquivo de estado.
    std::ifstream arquivo(caminho);

    if (!arquivo.is_open()) {
        throw std::runtime_error(
            "Nao foi possivel abrir o arquivo de estado");
    }

    // Le o documento JSON.
    nlohmann::json documento;

    try {
        documento = nlohmann::json::parse(arquivo);
    } catch (const nlohmann::json::parse_error&) {
        throw std::runtime_error(
            "Arquivo de estado JSON invalido");
    }

    // Verifica a estrutura do documento.
    if (!documento.is_object()) {
        throw std::runtime_error(
            "Estado deve conter um objeto JSON");
    }

    // Valida o campo sessao.
    if (!documento.contains("sessao") ||
        !documento.at("sessao").is_string()) {

        throw std::runtime_error(
            "Sessao do estado ausente ou invalida");
    }

    std::string sessao =
        documento.at("sessao").get<std::string>();

    if (sessao.empty()) {
        throw std::runtime_error(
            "Sessao do estado nao pode ser vazia");
    }

    // Valida o campo ciclo.
    if (!documento.contains("ciclo") ||
        !documento.at("ciclo").is_number_integer()) {

        throw std::runtime_error(
            "Ciclo do estado ausente ou invalido");
    }

    const auto& campoCiclo = documento.at("ciclo");

    // Verifica se o ciclo e positivo.
    if (campoCiclo.is_number_unsigned()) {

        if (campoCiclo.get<std::uint64_t>() == 0) {
            throw std::runtime_error(
                "Ciclo do estado deve ser positivo");
        }

    } else {

        if (campoCiclo.get<std::int64_t>() <= 0) {
            throw std::runtime_error(
                "Ciclo do estado deve ser positivo");
        }
    }

    std::uint64_t ciclo =
        campoCiclo.get<std::uint64_t>();

    // Atualiza o estado somente apos todas as validacoes.
    ultimoRegistro_ = Registro{sessao, ciclo};
}

void EstadoAquisicao::salvar(
    const std::filesystem::path& caminho) const {

    // Verifica se existe um registro para salvar.
    if (!ultimoRegistro_.has_value()) {
        throw std::runtime_error(
            "Nao existe estado de aquisicao para salvar");
    }

    // Monta o documento JSON.
    const nlohmann::json documento = {
        {"sessao", ultimoRegistro_->sessao},
        {"ciclo", ultimoRegistro_->ciclo}
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
                    "Nao foi possivel criar o arquivo temporario");
            }

            arquivo << documento.dump(4) << '\n';

            arquivo.close();

            if (!arquivo) {
                throw std::runtime_error(
                    "Falha ao gravar o estado da aquisicao");
            }
        }

        // Substitui o arquivo definitivo.
        std::filesystem::rename(temporario, caminho);

    } catch (...) {

        // Remove o arquivo temporario em caso de falha.
        std::error_code erro;
        std::filesystem::remove(temporario, erro);

        throw;
    }
}
