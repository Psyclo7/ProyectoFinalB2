#include "optimizacion.h"
#include <iostream>

int main() {
    double precio_mesas = 7.0, precio_sillas = 5.0;
    std::vector<Restriccion> restricciones;
    int opcion;

    do {
        std::cout << "\n--- Menu ---\n"
            << "1. Ingresar precios\n"
            << "2. Ingresar restricciones\n"
            << "3. Mostrar funcion\n"
            << "4. Calcular solucion\n"
            << "5. Mostrar grafica\n"
            << "6. Salir\n"
            << "Opcion: ";
        std::cin >> opcion;

        try {
            switch (opcion) {
            case 1: ingresarPrecios(precio_mesas, precio_sillas); break;
            case 2: ingresarRestricciones(restricciones); break;
            case 3: mostrarFuncionGanancia(precio_mesas, precio_sillas, restricciones); break;
            case 4: {
                Solucion sol = calcularSolucionOptima(restricciones, precio_mesas, precio_sillas);
                std::cout << "Solucion: " << sol.mesas << " mesas, "
                    << sol.sillas << " sillas, Ganancia: $" << sol.ganancia << "\n";
                break;
            }
            case 5: {
                if (restricciones.empty()) {
                    std::cout << "Ingrese restricciones primero.\n";
                }
                else {
                    Solucion sol = calcularSolucionOptima(restricciones, precio_mesas, precio_sillas);
                    mostrarGrafica(sol, restricciones);
                }
                break;
            }
            case 6: std::cout << "Saliendo...\n"; break;
            default: std::cout << "Opcion invalida.\n";
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n"; 
        }
    } while (opcion != 6);

    return 0;
}