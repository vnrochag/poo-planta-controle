
#include "Aquisicao.hpp"

#include <nlohmann/json.hpp>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>

DadosPlanta Aquisicao::lerArquivo(
    const std::filesystem::path& caminho) {

    // 1. Tenta abrir o arquivo da planta.
    std::ifstream arquivo(caminho);

    if (!arquivo.is_open()) {
        throw std::runtime_error(
            "Nao foi possivel abrir o arquivo: " + caminho.string());
    }

    // 2. Tenta interpretar o conteudo como JSON.
    nlohmann::json documento;

    try {
        arquivo >> documento;
    } catch (const nlohmann::json::parse_error&) {
        throw std::runtime_error("Arquivo JSON invalido");
    }

    // 3. A raiz do JSON deve ser um objeto.
    if (!documento.is_object()) {
        throw std::runtime_error("JSON deve conter um objeto");
    }

    // 4. Verifica se a versao existe.
    if (!documento.contains("versao")) {
        throw std::runtime_error("Campo versao ausente");
    }

    // 5. Verifica se a versao e um numero inteiro.
    if (!documento.at("versao").is_number_integer()) {
        throw std::runtime_error("Versao deve ser um inteiro");
    }

    // 6. Aceita somente a versao 1 do contrato.
    if (documento.at("versao") != 1) {
        throw std::runtime_error(
            "Versao do contrato nao suportada");
    }

    // 7. Verifica se a sessao existe e e um texto.
    if (!documento.contains("sessao") ||
        !documento.at("sessao").is_string()) {
        throw std::runtime_error(
            "Sessao ausente ou invalida");
    }

    // 8. A sessao nao pode ser vazia.
    if (documento.at("sessao").get<std::string>().empty()) {
        throw std::runtime_error(
            "Sessao nao pode ser vazia");
    }

    // 9. Verifica se o ciclo existe e e um inteiro.
    if (!documento.contains("ciclo") ||
        !documento.at("ciclo").is_number_integer()) {
        throw std::runtime_error(
            "Ciclo ausente ou invalido");
    }

    const auto& campoCiclo = documento.at("ciclo");

    // 10. O ciclo deve ser maior que zero.
    if (campoCiclo.is_number_unsigned()) {
        if (campoCiclo.get<std::uint64_t>() == 0) {
            throw std::runtime_error(
                "Ciclo deve ser positivo");
        }
    } else if (campoCiclo.get<std::int64_t>() <= 0) {
        throw std::runtime_error(
            "Ciclo deve ser positivo");
    }

    // 11. Verifica se o timestamp existe e e um texto.
    if (!documento.contains("timestamp") ||
        !documento.at("timestamp").is_string()) {
        throw std::runtime_error(
            "Timestamp ausente ou invalido");
    }

    const std::string timestamp =
        documento.at("timestamp").get<std::string>();

    // 12. Verifica o formato AAAA-MM-DDTHH:MM:SSZ.
    if (timestamp.size() != 20 ||
        timestamp[4] != '-' ||
        timestamp[7] != '-' ||
        timestamp[10] != 'T' ||
        timestamp[13] != ':' ||
        timestamp[16] != ':' ||
        timestamp[19] != 'Z') {
        throw std::runtime_error(
            "Formato do timestamp invalido");
    }

    // 13. Verifica se as posicoes numericas possuem digitos.
    for (std::size_t i = 0; i < timestamp.size(); ++i) {
        if (i == 4 || i == 7 || i == 10 ||
            i == 13 || i == 16 || i == 19) {
            continue;
        }

        if (timestamp[i] < '0' || timestamp[i] > '9') {
            throw std::runtime_error(
                "Timestamp contem caracteres invalidos");
        }
    }

    // 14. Extrai os valores da data e do horario.
    const int ano = std::stoi(timestamp.substr(0, 4));
    const int mes = std::stoi(timestamp.substr(5, 2));
    const int dia = std::stoi(timestamp.substr(8, 2));

    const int hora = std::stoi(timestamp.substr(11, 2));
    const int minuto = std::stoi(timestamp.substr(14, 2));
    const int segundo = std::stoi(timestamp.substr(17, 2));

    // 15. Verifica os limites da data e do horario.
    if (ano < 1 || mes < 1 || mes > 12 ||
        hora > 23 || minuto > 59 || segundo > 59) {
        throw std::runtime_error(
            "Data ou horario invalido");
    }

    // 16. Define a quantidade de dias de cada mes.
    int diasPorMes[] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };

    // Verifica se o ano e bissexto.
    const bool bissexto = (ano % 4 == 0) &&
                          (ano % 100 != 0 || ano % 400 == 0);

    if (bissexto) {
        diasPorMes[1] = 29;
    }

    // 17. Verifica se o dia existe naquele mes.
    if (dia < 1 || dia > diasPorMes[mes - 1]) {
        throw std::runtime_error(
            "Dia invalido no timestamp");
    }

    // 18. Verifica se o campo leituras existe.
    if (!documento.contains("leituras")) {
        throw std::runtime_error(
            "Campo leituras ausente");
    }

    // 19. Verifica se leituras e uma lista JSON.
    if (!documento.at("leituras").is_array()) {
        throw std::runtime_error(
            "Leituras deve ser uma lista");
    }

    const auto& leituras = documento.at("leituras");

    // 20. Verifica se a lista nao esta vazia.
    if (leituras.empty()) {
        throw std::runtime_error(
            "Lista de leituras vazia");
    }

    // 21. Prepara os objetos que receberao as medicoes.
    std::optional<Medicao> nivel;
    std::optional<Medicao> temperatura;

    // 22. Percorre cada sensor recebido da planta.
    for (const auto& leitura : leituras) {

        // Cada leitura deve ser um objeto JSON.
        if (!leitura.is_object()) {
            throw std::runtime_error(
                "Leitura deve ser um objeto JSON");
        }

        // Verifica os cinco campos obrigatorios.
        for (const char* campo :
             {"tag", "tipo", "valor", "unidade", "status"}) {

            if (!leitura.contains(campo)) {
                throw std::runtime_error(
                    std::string("Campo ausente na leitura: ") + campo);
            }
        }

        // Verifica se os campos textuais sao strings.
        if (!leitura.at("tag").is_string() ||
            !leitura.at("tipo").is_string() ||
            !leitura.at("unidade").is_string() ||
            !leitura.at("status").is_string()) {

            throw std::runtime_error(
                "Campo textual invalido na leitura");
        }

        // O valor deve ser numerico ou null.
        const auto& valor = leitura.at("valor");

        if (!valor.is_number() && !valor.is_null()) {
            throw std::runtime_error(
                "Valor do sensor deve ser numero ou null");
        }

        // 23. Extrai as informacoes da leitura.
        const std::string tag =
            leitura.at("tag").get<std::string>();

        const std::string tipo =
            leitura.at("tipo").get<std::string>();

        const std::string unidade =
            leitura.at("unidade").get<std::string>();

        const std::string status =
            leitura.at("status").get<std::string>();

        // 24. Converte o valor, respeitando o null.
        std::optional<double> numero = std::nullopt;

        if (!valor.is_null()) {
            numero = valor.get<double>();
        }

        // 25. Identifica o sensor e cria a medicao.
        if (tag == "LT-01") {

            if (nivel.has_value()) {
                throw std::runtime_error(
                    "Sensor LT-01 duplicado");
            }

            nivel.emplace(
                tag, tipo, numero, unidade, status);

        } else if (tag == "TT-01") {

            if (temperatura.has_value()) {
                throw std::runtime_error(
                    "Sensor TT-01 duplicado");
            }

            temperatura.emplace(
                tag, tipo, numero, unidade, status);

        } else {
            throw std::runtime_error(
                "Sensor desconhecido: " + tag);
        }
    }

    // 26. Confirma que os dois sensores foram recebidos.
    if (!nivel.has_value() || !temperatura.has_value()) {
        throw std::runtime_error(
            "Sensor obrigatorio ausente");
    }

    // 27. Verifica se os atuadores existem.
    if (!documento.contains("atuadores") ||
        !documento.at("atuadores").is_object()) {
        throw std::runtime_error(
            "Campo atuadores ausente ou invalido");
    }

    const auto& atuadores = documento.at("atuadores");

    // 28. Verifica se os dois atuadores obrigatorios existem.
    if (!atuadores.contains("bomba") ||
        !atuadores.contains("aquecedor")) {
        throw std::runtime_error(
            "Atuador obrigatorio ausente");
    }

    // 29. Verifica se as potencias sao numeros.
    if (!atuadores.at("bomba").is_number() ||
        !atuadores.at("aquecedor").is_number()) {
        throw std::runtime_error(
            "Potencia de atuador invalida");
    }

    // 30. Extrai as potencias aplicadas.
    const double bomba =
        atuadores.at("bomba").get<double>();

    const double aquecedor =
        atuadores.at("aquecedor").get<double>();

    // 31. Verifica se as potencias sao finitas
    // e pertencem ao intervalo de 0 a 100%.
    if (!std::isfinite(bomba) ||
        !std::isfinite(aquecedor) ||
        bomba < 0.0 || bomba > 100.0 ||
        aquecedor < 0.0 || aquecedor > 100.0) {
        throw std::runtime_error(
            "Potencia de atuador fora dos limites");
    }

    // 32. Verifica o ultimo comando aceito pela planta.
    if (!documento.contains("ultimo_comando") ||
        !documento.at("ultimo_comando").is_number_integer()) {
        throw std::runtime_error(
            "Ultimo comando ausente ou invalido");
    }

    const auto& campoUltimoComando =
        documento.at("ultimo_comando");

    // O identificador nao pode ser negativo.
    if (!campoUltimoComando.is_number_unsigned() &&
        campoUltimoComando.get<std::int64_t>() < 0) {
        throw std::runtime_error(
            "Ultimo comando nao pode ser negativo");
    }

    const std::uint64_t ultimoComando =
        campoUltimoComando.get<std::uint64_t>();

    // 33. Verifica o campo erro_comando.
    if (!documento.contains("erro_comando")) {
        throw std::runtime_error(
            "Campo erro_comando ausente");
    }

    const auto& campoErro = documento.at("erro_comando");

    if (!campoErro.is_null() && !campoErro.is_string()) {
        throw std::runtime_error(
            "Campo erro_comando invalido");
    }

    std::optional<std::string> erroComando;

    if (campoErro.is_string()) {
        erroComando = campoErro.get<std::string>();
    }

    // 34. Verifica o campo motivo_seguro.
    if (!documento.contains("motivo_seguro")) {
        throw std::runtime_error(
            "Campo motivo_seguro ausente");
    }

    const auto& campoMotivo = documento.at("motivo_seguro");

    if (!campoMotivo.is_null() && !campoMotivo.is_string()) {
        throw std::runtime_error(
            "Campo motivo_seguro invalido");
    }

    std::optional<std::string> motivoSeguro;

    if (campoMotivo.is_string()) {
        const std::string motivo =
            campoMotivo.get<std::string>();

        if (motivo != "sem_arquivo" &&
            motivo != "comando_invalido" &&
            motivo != "comando_expirado") {
            throw std::runtime_error(
                "Motivo seguro desconhecido");
        }

        motivoSeguro = motivo;
    }

    // 35. Reune todas as informacoes validadas.
    return DadosPlanta{
        1,
        documento.at("sessao").get<std::string>(),
        campoCiclo.get<std::uint64_t>(),
        timestamp,
        *nivel,
        *temperatura,
        bomba,
        aquecedor,
        ultimoComando,
        erroComando,
        motivoSeguro
    };
}
