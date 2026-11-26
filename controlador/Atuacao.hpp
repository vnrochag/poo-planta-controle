#pragma once

class Atuacao {
public:
    Atuacao(double bomba, double aquecedor);

    double bomba() const;
    double aquecedor() const;

private:
    double bomba_;
    double aquecedor_;
};
