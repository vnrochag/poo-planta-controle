
#include "EstadoAquisicao.hpp"

#include <nlohmann/json.hpp>

#include <fstream>
#include <limits>
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

    if (ultimoRegistro_->ciclo == 0) {
        return ResultadoCiclo::PrimeiraAquisicao;
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
    if (!ultimoRegistro_ || resultado == ResultadoCiclo::NovaSessao) {
        ultimoRegistro_ = Registro{sessao, ciclo};
    } else {
        ultimoRegistro_->ciclo = ciclo;
    }
}

void EstadoAquisicao::reconciliar(const std::string& sessao, std::uint64_t ciclo,
                                 std::uint64_t ultimoComando) {
    const auto resultado = avaliar(sessao, ciclo);
    if (resultado == ResultadoCiclo::CicloAntigo) {
        throw std::invalid_argument("Confirmacao em ciclo antigo");
    }
    if (!ultimoRegistro_ || resultado == ResultadoCiclo::NovaSessao) {
        ultimoRegistro_ = Registro{sessao, 0, ultimoComando};
        return;
    }

    auto& registro = *ultimoRegistro_;
    if (ultimoComando < registro.confirmado) {
        throw std::invalid_argument("ID confirmado regressivo na mesma sessao");
    }
    if (const auto& solicitacao = registro.solicitacao) {
        const auto id = static_cast<std::uint64_t>(solicitacao->comando.id());
        if (ciclo < solicitacao->ciclo ||
            (registro.confirmado < id && ultimoComando >= id &&
             ciclo == solicitacao->ciclo)) {
            throw std::invalid_argument("Ciclo incompativel com a solicitacao");
        }
    }
    registro.confirmado = ultimoComando;
}

Comando EstadoAquisicao::reservarComando(const std::string& sessao, std::uint64_t ciclo,
                                        const Atuacao& atuacao) {
    const auto resultado = avaliar(sessao, ciclo);
    if (!ultimoRegistro_ || resultado == ResultadoCiclo::NovaSessao ||
        resultado == ResultadoCiclo::CicloAntigo ||
        resultado == ResultadoCiclo::CicloRepetido) {
        throw std::invalid_argument("Ciclo invalido para reservar comando");
    }
    auto& registro = *ultimoRegistro_;
    if (registro.solicitacao &&
        (ciclo <= registro.solicitacao->ciclo ||
         registro.confirmado < static_cast<std::uint64_t>(registro.solicitacao->comando.id()))) {
        throw std::logic_error("Ja existe solicitacao para o ciclo ou comando pendente");
    }
    if (registro.confirmado >=
        static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
        throw std::overflow_error("Nao ha ID de comando disponivel na sessao");
    }
    const Comando comando(sessao, static_cast<std::int64_t>(registro.confirmado) + 1, atuacao);
    registro.solicitacao = Solicitacao{ciclo, comando};
    return comando;
}

std::optional<EstadoAquisicao::Solicitacao> EstadoAquisicao::pendente() const {
    if (ultimoRegistro_ && ultimoRegistro_->solicitacao &&
        ultimoRegistro_->confirmado <
            static_cast<std::uint64_t>(ultimoRegistro_->solicitacao->comando.id())) {
        return ultimoRegistro_->solicitacao;
    }
    return std::nullopt;
}

std::uint64_t EstadoAquisicao::ultimoComandoConfirmado() const {
    return ultimoRegistro_ ? ultimoRegistro_->confirmado : 0;
}

namespace {

std::uint64_t lerInteiroNaoNegativo(const nlohmann::json& campo,
                                  const char* mensagem) {
    if (!campo.is_number_integer() ||
        (!campo.is_number_unsigned() && campo.get<std::int64_t>() < 0)) {
        throw std::runtime_error(mensagem);
    }
    return campo.get<std::uint64_t>();
}

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

    const auto ciclo = lerInteiroNaoNegativo(campoCiclo, "Ciclo do estado deve ser positivo");
    const bool formatoEstendido = documento.contains("ultimo_comando_confirmado");
    if (ciclo == 0 && !formatoEstendido) {
        throw std::runtime_error("Ciclo do estado deve ser positivo");
    }

    Registro recuperado{sessao, ciclo};
    if (formatoEstendido) {
        recuperado.confirmado = lerInteiroNaoNegativo(
            documento.at("ultimo_comando_confirmado"), "ID confirmado invalido");
    }
    if (documento.contains("solicitacao")) {
        const auto& pedido = documento.at("solicitacao");
        if (!formatoEstendido || !pedido.is_object() || !pedido.contains("ciclo") ||
            !pedido.contains("id") || !pedido.contains("bomba") ||
            !pedido.contains("aquecedor") || !pedido.at("bomba").is_number() ||
            !pedido.at("aquecedor").is_number()) {
            throw std::runtime_error("Solicitacao persistida invalida");
        }
        const auto origem = lerInteiroNaoNegativo(pedido.at("ciclo"), "Ciclo da solicitacao invalido");
        const auto id = lerInteiroNaoNegativo(pedido.at("id"), "ID da solicitacao invalido");
        if (origem == 0 || id == 0 ||
            id > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()) ||
            (id > recuperado.confirmado && id - recuperado.confirmado != 1)) {
            throw std::runtime_error("Identidade da solicitacao persistida invalida");
        }
        try {
            recuperado.solicitacao = Solicitacao{
                origem, Comando(sessao, static_cast<std::int64_t>(id),
                                Atuacao(pedido.at("bomba").get<double>(),
                                        pedido.at("aquecedor").get<double>()))};
        } catch (const std::invalid_argument& erro) {
            throw std::runtime_error(erro.what());
        }
    }

    ultimoRegistro_ = recuperado;
}

void EstadoAquisicao::salvar(
    const std::filesystem::path& caminho) const {

    // Verifica se existe um registro para salvar.
    if (!ultimoRegistro_.has_value()) {
        throw std::runtime_error(
            "Nao existe estado de aquisicao para salvar");
    }

    // Monta o documento JSON.
    nlohmann::json documento = {
        {"sessao", ultimoRegistro_->sessao},
        {"ciclo", ultimoRegistro_->ciclo}
    };

    if (ultimoRegistro_->ciclo == 0 || ultimoRegistro_->confirmado != 0 ||
        ultimoRegistro_->solicitacao) {
        documento["ultimo_comando_confirmado"] = ultimoRegistro_->confirmado;
    }
    if (const auto& solicitacao = ultimoRegistro_->solicitacao) {
        const auto& comando = solicitacao->comando;
        documento["solicitacao"] = {
            {"ciclo", solicitacao->ciclo},
            {"id", comando.id()},
            {"bomba", comando.atuacao().bomba()},
            {"aquecedor", comando.atuacao().aquecedor()}
        };
    }

    // Define o caminho do arquivo temporario.
    std::filesystem::path temporario = caminho;
    temporario += ".tmp";

    const std::string conteudo = documento.dump(4);
    std::ofstream arquivo(temporario);
    if (!arquivo.is_open()) {
        throw std::runtime_error("Nao foi possivel criar o arquivo temporario");
    }

    try {
        arquivo << conteudo << '\n';
        arquivo.close();
        if (!arquivo) {
            throw std::runtime_error("Falha ao gravar o estado da aquisicao");
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
