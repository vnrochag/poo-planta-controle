
#include "ValidadeTemporal.hpp"

#include <iomanip>
#include <sstream>
#include <stdexcept>

ResultadoTemporal ValidadeTemporal::avaliar(
    const std::string& timestamp,
    std::time_t agora) {

    // Estrutura utilizada para representar data e horario.
    std::tm data{};

    // Interpreta o timestamp recebido.
    std::istringstream entrada(timestamp);

    entrada >> std::get_time(
        &data,
        "%Y-%m-%dT%H:%M:%SZ");

    // Verifica se a conversao funcionou
    // e se nao existem caracteres restantes.
    if (entrada.fail() ||
        entrada.peek() != std::char_traits<char>::eof()) {

        throw std::invalid_argument(
            "Timestamp invalido para avaliacao temporal");
    }

    // Converte a data UTC em segundos.
    std::time_t instante = timegm(&data);

    // Calcula a idade da aquisicao em segundos.
    double idade = std::difftime(agora, instante);

    // Verifica se o timestamp esta no futuro.
    if (idade < 0) {
        return ResultadoTemporal::Futuro;
    }

    // Verifica se a aquisicao ultrapassou o limite.
    if (idade > LIMITE_SEGUNDOS) {
        return ResultadoTemporal::Expirado;
    }

    // A aquisicao esta dentro do periodo de validade.
    return ResultadoTemporal::Valido;
}
