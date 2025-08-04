#include "optimizacion.h"
#include <SFML/Graphics.hpp>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <vector>
#include <algorithm>

void mostrarGrafica(const Solucion& solucion,
    const std::vector<Restriccion>& restricciones) {

    // Configuración de ventana
    sf::RenderWindow ventana(sf::VideoMode(800, 600), "Solución de Optimización");
    ventana.setFramerateLimit(60);

    // Solución para la fuente - IMPORTANTE: Asegúrate de tener arial.ttf en tu directorio
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Error cargando la fuente arial.ttf - Usando fuente por defecto\n";
        // Intentar con una fuente básica si arial.ttf no está disponible
        if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
            font.loadFromMemory(nullptr, 0); // Fuente por defecto (limitada)
        }
    }

    // Configuración del gráfico
    double max_x = std::max(10.0, solucion.mesas * 1.5);
    double max_y = std::max(10.0, solucion.sillas * 1.5);

    for (const auto& r : restricciones) {
        if (r.coef_mesas != 0) max_x = std::max(max_x, r.limite / r.coef_mesas * 1.2);
        if (r.coef_sillas != 0) max_y = std::max(max_y, r.limite / r.coef_sillas * 1.2);
    }

    double escala_x = 700.0 / max_x;
    double escala_y = 500.0 / max_y;

    // Dibujar ejes con marcadores
    sf::Vertex ejeX[] = {
        {sf::Vector2f(50, 550), sf::Color::Black},
        {sf::Vector2f(750, 550), sf::Color::Black}
    };
    sf::Vertex ejeY[] = {
        {sf::Vector2f(50, 550), sf::Color::Black},
        {sf::Vector2f(50, 50), sf::Color::Black}
    };

    // Marcadores para ejes (mejorados)
    std::vector<sf::Text> marcadoresX;
    std::vector<sf::Text> marcadoresY;

    // Marcadores eje X
    for (int i = 0; i <= max_x; i += std::max(1, static_cast<int>(max_x / 5))) {
        sf::Text marcador;
        marcador.setFont(font);
        marcador.setString(std::to_string(i));
        marcador.setCharacterSize(14); // Tamaño aumentado
        marcador.setFillColor(sf::Color::Black);
        // Ajustar posición para mejor visibilidad
        marcador.setPosition(45 + i * escala_x, 555);
        marcadoresX.push_back(marcador);
    }

    // Marcadores eje Y
    for (int i = 0; i <= max_y; i += std::max(1, static_cast<int>(max_y / 5))) {
        sf::Text marcador;
        marcador.setFont(font);
        marcador.setString(std::to_string(i));
        marcador.setCharacterSize(14); // Tamaño aumentado
        marcador.setFillColor(sf::Color::Black);
        // Ajustar posición para mejor visibilidad
        marcador.setPosition(20, 545 - i * escala_y);
        marcadoresY.push_back(marcador);
    }

    // Colores para restricciones
    std::vector<sf::Color> colores = {
        sf::Color::Blue,
        sf::Color::Green,
        sf::Color::Red,
        sf::Color::Magenta,
        sf::Color(255, 165, 0) // Naranja
    };

    // Elementos gráficos
    std::vector<sf::VertexArray> lineasRestricciones;
    std::vector<sf::Text> textosRestricciones;
    sf::ConvexShape areaFactibleCombinada;

    // Calcular puntos para el área factible
    std::vector<sf::Vector2f> puntosFactibles;

    // 1. Puntos en los ejes
    puntosFactibles.push_back(sf::Vector2f(50, 550)); // Origen (0,0)
    puntosFactibles.push_back(sf::Vector2f(50 + max_x * escala_x, 550)); // (max_x, 0)
    puntosFactibles.push_back(sf::Vector2f(50, 550 - max_y * escala_y)); // (0, max_y)

    // 2. Puntos de intersección entre restricciones
    for (size_t i = 0; i < restricciones.size(); ++i) {
        const auto& r1 = restricciones[i];

        // Intersección con ejes
        if (r1.coef_mesas != 0) {
            double x_axis = r1.limite / r1.coef_mesas;
            if (x_axis <= max_x) {
                puntosFactibles.push_back(sf::Vector2f(50 + x_axis * escala_x, 550));
            }
        }

        if (r1.coef_sillas != 0) {
            double y_axis = r1.limite / r1.coef_sillas;
            if (y_axis <= max_y) {
                puntosFactibles.push_back(sf::Vector2f(50, 550 - y_axis * escala_y));
            }
        }

        // Intersección con otras restricciones
        for (size_t j = i + 1; j < restricciones.size(); ++j) {
            const auto& r2 = restricciones[j];

            double det = r1.coef_mesas * r2.coef_sillas - r2.coef_mesas * r1.coef_sillas;
            if (det != 0) {
                double x = (r2.coef_sillas * r1.limite - r1.coef_sillas * r2.limite) / det;
                double y = (r1.coef_mesas * r2.limite - r2.coef_mesas * r1.limite) / det;

                if (x >= 0 && y >= 0 && x <= max_x && y <= max_y) {
                    puntosFactibles.push_back(sf::Vector2f(
                        50 + x * escala_x,
                        550 - y * escala_y
                    ));
                }
            }
        }
    }

    // Filtrar puntos que están dentro de todas las restricciones
    std::vector<sf::Vector2f> puntosValidos;
    for (const auto& punto : puntosFactibles) {
        bool valido = true;
        double x = (punto.x - 50) / escala_x;
        double y = (550 - punto.y) / escala_y;

        for (const auto& r : restricciones) {
            if (r.coef_mesas * x + r.coef_sillas * y > r.limite + 1e-6) { // Pequeña tolerancia
                valido = false;
                break;
            }
        }

        if (valido) {
            puntosValidos.push_back(punto);
        }
    }

    // Ordenar puntos en sentido horario
    std::sort(puntosValidos.begin(), puntosValidos.end(),
        [](const sf::Vector2f& a, const sf::Vector2f& b) {
            return a.x < b.x || (a.x == b.x && a.y > b.y);
        });

    // Crear área factible
    if (!puntosValidos.empty()) {
        areaFactibleCombinada.setPointCount(puntosValidos.size());
        for (size_t i = 0; i < puntosValidos.size(); ++i) {
            areaFactibleCombinada.setPoint(i, puntosValidos[i]);
        }
        areaFactibleCombinada.setFillColor(sf::Color(100, 200, 100, 120)); // Verde semi-transparente
    }

    // Crear líneas y textos de restricciones
    for (size_t i = 0; i < restricciones.size(); ++i) {
        const auto& r = restricciones[i];
        sf::Color color = colores[i % colores.size()];

        // Línea de restricción
        sf::VertexArray linea(sf::Lines, 2);
        double x1 = (r.coef_mesas != 0) ? r.limite / r.coef_mesas : 0;
        double y1 = 0;
        double x2 = 0;
        double y2 = (r.coef_sillas != 0) ? r.limite / r.coef_sillas : 0;

        // Asegurar que los puntos estén dentro de los límites
        x1 = std::min(x1, max_x);
        y2 = std::min(y2, max_y);

        linea[0].position = sf::Vector2f(50 + x1 * escala_x, 550 - y1 * escala_y);
        linea[1].position = sf::Vector2f(50 + x2 * escala_x, 550 - y2 * escala_y);
        linea[0].color = color;
        linea[1].color = color;
        lineasRestricciones.push_back(linea);

        // Texto de restricción (mejorado)
        std::ostringstream oss;
        oss << r.coef_mesas << "x + " << r.coef_sillas << "y <= " << r.limite;

        sf::Text textoRestriccion;
        textoRestriccion.setFont(font);
        textoRestriccion.setString(oss.str());
        textoRestriccion.setCharacterSize(16); // Tamaño aumentado
        textoRestriccion.setFillColor(color);
        textoRestriccion.setStyle(sf::Text::Bold);
        textoRestriccion.setPosition(450, 30 + i * 30); // Posición mejorada
        textosRestricciones.push_back(textoRestriccion);
    }

    // Texto de solución óptima (preparado aquí para reutilizar font)
    std::ostringstream ossSol;
    ossSol << "Solución Óptima:\n";
    ossSol << "Mesas: " << solucion.mesas << "\n";
    ossSol << "Sillas: " << solucion.sillas << "\n";
    ossSol << "Ganancia: $" << std::fixed << std::setprecision(2) << solucion.ganancia;

    sf::Text textoSolucion;
    textoSolucion.setFont(font);
    textoSolucion.setString(ossSol.str());
    textoSolucion.setCharacterSize(18);
    textoSolucion.setFillColor(sf::Color::Black);
    textoSolucion.setStyle(sf::Text::Bold);
    textoSolucion.setPosition(500, 100);

    // Etiquetas de ejes
    sf::Text labelX, labelY;
    labelX.setFont(font);
    labelX.setString("Mesas (x)");
    labelX.setCharacterSize(16);
    labelX.setFillColor(sf::Color::Black);
    labelX.setPosition(700, 560);

    labelY.setFont(font);
    labelY.setString("Sillas (y)");
    labelY.setCharacterSize(16);
    labelY.setFillColor(sf::Color::Black);
    labelY.setPosition(20, 30);
    labelY.setRotation(-90);

    // Bucle principal
    while (ventana.isOpen()) {
        sf::Event event;
        while (ventana.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                ventana.close();
            }
        }

        ventana.clear(sf::Color::White);

        // Dibujar área factible (si existe)
        if (puntosValidos.size() >= 3) {
            ventana.draw(areaFactibleCombinada);
        }

        // Dibujar ejes
        ventana.draw(ejeX, 2, sf::Lines);
        ventana.draw(ejeY, 2, sf::Lines);

        // Dibujar marcadores de ejes
        for (const auto& marcador : marcadoresX) {
            ventana.draw(marcador);
        }
        for (const auto& marcador : marcadoresY) {
            ventana.draw(marcador);
        }

        // Dibujar etiquetas de ejes
        ventana.draw(labelX);
        ventana.draw(labelY);

        // Dibujar restricciones
        for (const auto& linea : lineasRestricciones) {
            ventana.draw(linea);
        }

        // Dibujar textos de restricciones
        for (const auto& texto : textosRestricciones) {
            ventana.draw(texto);
        }

        // Dibujar punto óptimo
        sf::CircleShape puntoOptimo(6);
        puntoOptimo.setFillColor(sf::Color::Red);
        puntoOptimo.setOutlineColor(sf::Color::Black);
        puntoOptimo.setOutlineThickness(1);
        puntoOptimo.setPosition(
            50 + solucion.mesas * escala_x - 6,
            550 - solucion.sillas * escala_y - 6
        );
        ventana.draw(puntoOptimo);

        // Dibujar información de solución
        ventana.draw(textoSolucion);

        ventana.display();
    }
}