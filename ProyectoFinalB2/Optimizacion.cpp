#include "optimizacion.h"
#include <iostream>
#include <algorithm>
#include <climits>
#include <cmath>
#include <vector>

void ingresarPrecios(double& precio_mesas, double& precio_sillas) {
    std::cout << "Ingrese precio de mesas: ";
    std::cin >> precio_mesas;
    std::cout << "Ingrese precio de sillas: ";
    std::cin >> precio_sillas;

    if (precio_mesas <= 0 || precio_sillas <= 0) {
        std::cerr << "Error: Los precios no pueden ser negativos o cero.\n";
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

    if (r.coef_mesas < 0 || r.coef_sillas < 0) {
        std::cerr << "Error: Los coeficientes no pueden ser negativos.\n";
        throw std::invalid_argument("Coeficientes deben ser no negativos.");
    }
    if (r.limite < 0) {
        std::cerr << "Error: El límite no puede ser negativo.\n";
        throw std::invalid_argument("Límite debe ser no negativo.");
    }

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
    // Verificar si el problema tiene soluciones infinitas (acotamiento básico)
    bool limitado_mesas = false, limitado_sillas = false;
    for (const auto& r : restricciones) {
        if (r.coef_mesas > 0) limitado_mesas = true;
        if (r.coef_sillas > 0) limitado_sillas = true;
    }
    if (!restricciones.empty() && (!limitado_mesas || !limitado_sillas)) {
        std::cerr << "Error: El problema tiene soluciones infinitas.\n";
        throw std::runtime_error("El área factible es ilimitada.");
    }

    // Calcular vértices de intersección entre restricciones
    std::vector<std::pair<int, int>> vertices;
    const double TOLERANCIA = 1e-6;

    // Añadir intersecciones entre pares de restricciones
    for (size_t i = 0; i < restricciones.size(); ++i) {
        for (size_t j = i + 1; j < restricciones.size(); ++j) {
            const auto& r1 = restricciones[i];
            const auto& r2 = restricciones[j];
            double det = r1.coef_mesas * r2.coef_sillas - r2.coef_mesas * r1.coef_sillas;
            if (std::abs(det) > TOLERANCIA) {
                double x = (r2.coef_sillas * r1.limite - r1.coef_sillas * r2.limite) / det;
                double y = (r1.coef_mesas * r2.limite - r2.coef_mesas * r1.limite) / det;
                if (x >= -TOLERANCIA && y >= -TOLERANCIA) {
                    int mx = static_cast<int>(std::round(x));
                    int my = static_cast<int>(std::round(y));
                    vertices.push_back({ mx, my });
                }
            }
        }
    }

    // Añadir intersecciones con los ejes (casos límite)
    for (const auto& r : restricciones) {
        if (r.coef_mesas > 0) {
            int mx = static_cast<int>(std::round(r.limite / r.coef_mesas));
            vertices.push_back({ mx, 0 });
        }
        if (r.coef_sillas > 0) {
            int my = static_cast<int>(std::round(r.limite / r.coef_sillas));
            vertices.push_back({ 0, my });
        }
    }

    // Eliminar duplicados y verificar factibilidad
    std::vector<std::pair<int, int>> vertices_factibles;
    for (const auto& v : vertices) {
        int m = v.first;
        int s = v.second;
        bool factible = true;
        for (const auto& r : restricciones) {
            double valor = r.coef_mesas * m + r.coef_sillas * s;
            if (valor > r.limite + TOLERANCIA) {
                factible = false;
                break;
            }
        }
        if (factible) {
            vertices_factibles.push_back(v);
        }
    }

    // Verificar si hay soluciones infinitas (paralelismo con función objetivo o restricciones paralelas)
    if (vertices_factibles.size() > 0) {
        for (size_t i = 0; i < restricciones.size(); ++i) {
            for (size_t j = i + 1; j < restricciones.size(); ++j) {
                const auto& r1 = restricciones[i];
                const auto& r2 = restricciones[j];
                if (r1.coef_mesas != 0 && r2.coef_mesas != 0 && r1.coef_sillas != 0 && r2.coef_sillas != 0) {
                    double ratio1 = r1.coef_mesas / r2.coef_mesas;
                    double ratio2 = r1.coef_sillas / r2.coef_sillas;
                    if (std::abs(ratio1 - ratio2) < TOLERANCIA) {
                        // Verificar si la restricción con mayor límite no añade un nuevo vértice
                        bool is_redundant = true;
                        for (const auto& v : vertices_factibles) {
                            double val1 = r1.coef_mesas * v.first + r1.coef_sillas * v.second;
                            double val2 = r2.coef_mesas * v.first + r2.coef_sillas * v.second;
                            if (val1 <= r1.limite + TOLERANCIA && (val2 > r2.limite + TOLERANCIA || std::abs(val2 - r2.limite) > TOLERANCIA)) {
                                is_redundant = false;
                                break;
                            }
                        }
                        if (is_redundant && r2.limite > r1.limite) {
                            std::cerr << "Error: El problema tiene soluciones infinitas.\n";
                            throw std::runtime_error("Restricciones paralelas redundantes generan múltiples soluciones.");
                        }
                    }
                }
            }
        }
    }

    // Priorizar el primer vértice de intersección factible
    Solucion mejor = { 0, 0, 0.0 };
    bool found_intersection = false;
    for (const auto& v : vertices_factibles) {
        int m = v.first;
        int s = v.second;
        double ganancia = precio_mesas * m + precio_sillas * s;
        // Verificar si es un vértice de intersección (no eje)
        bool is_intersection = false;
        for (size_t i = 0; i < restricciones.size(); ++i) {
            for (size_t j = i + 1; j < restricciones.size(); ++j) {
                const auto& r1 = restricciones[i];
                const auto& r2 = restricciones[j];
                double val1 = r1.coef_mesas * m + r1.coef_sillas * s;
                double val2 = r2.coef_mesas * m + r2.coef_sillas * s;
                if (std::abs(val1 - r1.limite) < TOLERANCIA && std::abs(val2 - r2.limite) < TOLERANCIA) {
                    is_intersection = true;
                    break;
                }
            }
            if (is_intersection) break;
        }
        if (is_intersection && !found_intersection) {
            mejor = { m, s, ganancia };
            found_intersection = true;
            break; // Priorizar la primera intersección encontrada
        }
        else if (!found_intersection) {
            mejor = { m, s, ganancia };
        }
    }

    if (mejor.ganancia == 0.0) {
        // Verificar si realmente no hay solución o si es porque la solución es (0,0)
        bool origen_valido = true;
        for (const auto& r : restricciones) {
            if (0 > r.limite + TOLERANCIA) {
                origen_valido = false;
                break;
            }
        }

        if (!origen_valido) {
            throw std::runtime_error("No hay soluciones factibles con enteros");
        }
    }

    return mejor;
}