#include "optimizacion.h"
#include <iostream>
#include <algorithm>
#include <climits>

void ingresarPrecios(double& precio_mesas, double& precio_sillas) {
    std::cout << "Ingrese precio de mesas: ";
    std::cin >> precio_mesas;
    std::cout << "Ingrese precio de sillas: ";
    std::cin >> precio_sillas;

    if (precio_mesas <= 0 || precio_sillas <= 0) {
        throw std::invalid_argument("Precios deben ser positivos.");
    }
}

void ingresarRestricciones(std::vector<Restriccion>& restricciones) {
    Restriccion r;
    std::cout << "Coef. para mesas: ";
    std::cin >> r.coef_mesas;
    std::cout << "Coef. para sillas: ";
    std::cin >> r.coef_sillas;
    std::cout << "Limite: ";
    std::cin >> r.limite;

    restricciones.push_back(r);
}

void mostrarFuncionGanancia(double precio_mesas, double precio_sillas,
    const std::vector<Restriccion>& restricciones) {
    std::cout << "\n--- Sistema Actual ---\n";
    std::cout << "Funcion a maximizar: Z = "
        << precio_mesas << "x + " << precio_sillas << "y\n\n";
    std::cout << "Restricciones:\n";
    for (const auto& r : restricciones) {
        std::cout << r.coef_mesas << "x + " << r.coef_sillas << "y <= "
            << r.limite << "\n";
    }
}

Solucion calcularSolucionOptima(const std::vector<Restriccion>& restricciones,
    double precio_mesas, double precio_sillas) {
    
    int max_mesas = 0, max_sillas = 0;

    
    for (const auto& r : restricciones) {
        if (r.coef_sillas == 0) {
            max_mesas = std::max(max_mesas, static_cast<int>(r.limite / r.coef_mesas));
        }
        if (r.coef_mesas == 0) {
            max_sillas = std::max(max_sillas, static_cast<int>(r.limite / r.coef_sillas));
        }
    }

    
    if (max_mesas == 0) max_mesas = 100;
    if (max_sillas == 0) max_sillas = 100;

    
    Solucion mejor = { 0, 0, 0.0 };

    for (int m = 0; m <= max_mesas; ++m) {
        for (int s = 0; s <= max_sillas; ++s) {
            
            bool factible = true;
            for (const auto& r : restricciones) {
                double valor = r.coef_mesas * m + r.coef_sillas * s;
                if (valor > r.limite + 1e-6) {  
                    factible = false;
                    break;
                }
            }

            
            if (factible) {
                double ganancia = precio_mesas * m + precio_sillas * s;
                if (ganancia > mejor.ganancia) {
                    mejor = { m, s, ganancia };
                }
            }
        }
    }

    if (mejor.ganancia == 0.0 && !(max_mesas == 0 && max_sillas == 0)) {
        throw std::runtime_error("No hay soluciones factibles con enteros");
    }

    return mejor;
}