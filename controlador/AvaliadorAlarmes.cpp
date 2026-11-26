#include "AvaliadorAlarmes.hpp"

std::vector<Alarme> AvaliadorAlarmes::avaliar(const Medicao& nivel,
                                             const Medicao& temperatura) {
    std::vector<Alarme> alarmes;

    if (nivel.estaOperando()) {
        const double valor = *nivel.valor();
        if (valor < 5.0) {
            alarmes.push_back({"NIVEL_CRITICO", "Nivel abaixo de 5%", "critico"});
        } else if (valor < 20.0) {
            alarmes.push_back({"NIVEL_BAIXO", "Nivel entre 5% e abaixo de 20%", "aviso"});
        } else if (valor > 90.0) {
            alarmes.push_back({"NIVEL_ALTO", "Nivel acima de 90%", "aviso"});
        }
    }

    if (temperatura.estaOperando()) {
        const double valor = *temperatura.valor();
        if (valor < 22.0) {
            alarmes.push_back({"TEMP_BAIXA", "Temperatura abaixo de 22 C", "aviso"});
        } else if (valor > 40.0) {
            alarmes.push_back({"TEMP_ALTA", "Temperatura acima de 40 C", "aviso"});
        }
    }

    return alarmes;
}
