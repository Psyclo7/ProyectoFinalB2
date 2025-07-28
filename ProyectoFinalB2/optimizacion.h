#ifndef OPTIMIZACION_H
#define OPTIMIZACION_H

#include <vector>
#include <stdexcept>

struct Restriccion {
    double coef_mesas;
    double coef_sillas;
    double limite;
};

struct Solucion {
    int mesas;    
    int sillas;   
    double ganancia;
};

// Funciones basicas
void ingresarPrecios(double& precio_mesas, double& precio_sillas);
void ingresarRestricciones(std::vector<Restriccion>& restricciones);
void mostrarFuncionGanancia(double precio_mesas, double precio_sillas,
    const std::vector<Restriccion>& restricciones);
Solucion calcularSolucionOptima(const std::vector<Restriccion>& restricciones,
    double precio_mesas, double precio_sillas);

// Función graficaa de mierdaaaa
void mostrarGrafica(const Solucion& solucion,
    const std::vector<Restriccion>& restricciones);

#endif