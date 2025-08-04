#include "optimizacion.h"
#include <SFML/Graphics.hpp>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

void mostrarGrafica(const Solucion& solucion, const std::vector<Restriccion>& restricciones) {
    // Configuración de ventana
    sf::RenderWindow ventana(sf::VideoMode(800, 600), "Solución de Optimización");
    ventana.setFramerateLimit(60);

    // Carga de fuente
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
            std::cerr << "Error: No se pudo cargar ninguna fuente. Terminando.\n";
            ventana.close();
            return;
        }
    }

    // Validar solución
    bool solucionValida = true;
    const double TOLERANCIA = 1e-4;
    for (const auto& r : restricciones) {
        if (r.coef_mesas * solucion.mesas + r.coef_sillas * solucion.sillas > r.limite + TOLERANCIA) {
            solucionValida = false;
            std::cerr << "Error: La solución óptima no satisface las restricciones.\n";
            break;
        }
    }
    if (!solucionValida) {
        std::cerr << "Advertencia: La solución proporcionada no es válida.\n";
    }

    // Configuración del gráfico
    double max_x = std::max(10.0, solucion.mesas * 1.5);
    double max_y = std::max(10.0, solucion.sillas * 1.5);

    for (const auto& r : restricciones) {
        if (r.coef_mesas != 0) max_x = std::max(max_x, r.limite / r.coef_mesas * 1.2);
        if (r.coef_sillas != 0) max_y = std::max(max_y, r.limite / r.coef_sillas * 1.2);
    }
    max_x = std::min(max_x, 100.0); // Límite superior para casos ilimitados
    max_y = std::min(max_y, 100.0);

    double escala_x = 700.0 / max_x;
    double escala_y = 500.0 / max_y;

    // Dibujar ejes
    sf::Vertex ejeX[] = {
        {sf::Vector2f(50, 550), sf::Color::Black},
        {sf::Vector2f(750, 550), sf::Color::Black}
    };
    sf::Vertex ejeY[] = {
        {sf::Vector2f(50, 550), sf::Color::Black},
        {sf::Vector2f(50, 50), sf::Color::Black}
    };

    // Marcadores para ejes
    std::vector<sf::Text> marcadoresX;
    std::vector<sf::Text> marcadoresY;
    for (int i = 0; i <= max_x; i += std::max(1, static_cast<int>(max_x / 5))) {
        sf::Text marcador;
        marcador.setFont(font);
        marcador.setString(std::to_string(i));
        marcador.setCharacterSize(14);
        marcador.setFillColor(sf::Color::Black);
        marcador.setPosition(45 + i * escala_x, 555);
        marcadoresX.push_back(marcador);
    }
    for (int i = 0; i <= max_y; i += std::max(1, static_cast<int>(max_y / 5))) {
        sf::Text marcador;
        marcador.setFont(font);
        marcador.setString(std::to_string(i));
        marcador.setCharacterSize(14);
        marcador.setFillColor(sf::Color::Black);
        marcador.setPosition(20, 545 - i * escala_y);
        marcadoresY.push_back(marcador);
    }

    // Colores para restricciones
    std::vector<sf::Color> colores = {
        sf::Color::Blue, sf::Color::Green, sf::Color::Red,
        sf::Color::Magenta, sf::Color(255, 165, 0)
    };

    // Elementos gráficos
    std::vector<sf::VertexArray> lineasRestricciones;
    std::vector<sf::Text> textosRestricciones;
    sf::ConvexShape areaFactibleCombinada;

    // Calcular puntos factibles
    std::vector<sf::Vector2f> puntosFactibles;
    puntosFactibles.push_back(sf::Vector2f(50, 550)); // Origen
    puntosFactibles.push_back(sf::Vector2f(50 + max_x * escala_x, 550)); // (max_x, 0)
    puntosFactibles.push_back(sf::Vector2f(50, 550 - max_y * escala_y)); // (0, max_y)

    // Intersecciones con ejes y entre restricciones
    for (size_t i = 0; i < restricciones.size(); ++i) {
        const auto& r1 = restricciones[i];
        if (r1.coef_mesas != 0) {
            double x_axis = r1.limite / r1.coef_mesas;
            if (x_axis <= max_x && x_axis >= 0) {
                puntosFactibles.push_back(sf::Vector2f(50 + x_axis * escala_x, 550));
            }
        }
        if (r1.coef_sillas != 0) {
            double y_axis = r1.limite / r1.coef_sillas;
            if (y_axis <= max_y && y_axis >= 0) {
                puntosFactibles.push_back(sf::Vector2f(50, 550 - y_axis * escala_y));
            }
        }
        for (size_t j = i + 1; j < restricciones.size(); ++j) {
            const auto& r2 = restricciones[j];
            double det = r1.coef_mesas * r2.coef_sillas - r2.coef_mesas * r1.coef_sillas;
            if (std::abs(det) < TOLERANCIA) continue; // Restricciones paralelas
            double x = (r2.coef_sillas * r1.limite - r1.coef_sillas * r2.limite) / det;
            double y = (r1.coef_mesas * r2.limite - r2.coef_mesas * r1.limite) / det;
            if (x >= 0 && y >= 0 && x <= max_x && y <= max_y) {
                puntosFactibles.push_back(sf::Vector2f(50 + x * escala_x, 550 - y * escala_y));
            }
        }
    }

    // Filtrar puntos válidos
    std::vector<sf::Vector2f> puntosValidos;
    for (const auto& punto : puntosFactibles) {
        bool valido = true;
        double x = (punto.x - 50) / escala_x;
        double y = (550 - punto.y) / escala_y;
        for (const auto& r : restricciones) {
            if (r.coef_mesas * x + r.coef_sillas * y > r.limite + TOLERANCIA) {
                valido = false;
                break;
            }
        }
        if (valido) puntosValidos.push_back(punto);
    }

    // Eliminar duplicados
    std::vector<sf::Vector2f> puntosUnicos;
    std::sort(puntosValidos.begin(), puntosValidos.end(),
        [](const sf::Vector2f& a, const sf::Vector2f& b) {
            return a.x < b.x || (a.x == b.x && a.y < b.y);
        });
    if (!puntosValidos.empty()) {
        puntosUnicos.push_back(puntosValidos[0]);
        for (size_t i = 1; i < puntosValidos.size(); ++i) {
            if (std::abs(puntosValidos[i].x - puntosValidos[i - 1].x) > TOLERANCIA ||
                std::abs(puntosValidos[i].y - puntosValidos[i - 1].y) > TOLERANCIA) {
                puntosUnicos.push_back(puntosValidos[i]);
            }
        }
    }

    // Ordenar puntos en sentido horario (algoritmo simple basado en ángulo)
    if (puntosUnicos.size() >= 3) {
        // Calcular centroide
        sf::Vector2f centroide(0, 0);
        for (const auto& p : puntosUnicos) {
            centroide.x += p.x;
            centroide.y += p.y;
        }
        centroide.x /= puntosUnicos.size();
        centroide.y /= puntosUnicos.size();

        std::sort(puntosUnicos.begin(), puntosUnicos.end(),
            [centroide](const sf::Vector2f& a, const sf::Vector2f& b) {
                double angleA = std::atan2(a.y - centroide.y, a.x - centroide.x);
                double angleB = std::atan2(b.y - centroide.y, b.x - centroide.x);
                return angleA < angleB;
            });

        // Crear área factible
        areaFactibleCombinada.setPointCount(puntosUnicos.size());
        for (size_t i = 0; i < puntosUnicos.size(); ++i) {
            areaFactibleCombinada.setPoint(i, puntosUnicos[i]);
        }
        areaFactibleCombinada.setFillColor(sf::Color(100, 200, 100, 120));
    }
    else {
        std::cerr << "Advertencia: No hay suficientes puntos para formar un área factible.\n";
    }

    // Crear líneas y textos de restricciones
    for (size_t i = 0; i < restricciones.size(); ++i) {
        const auto& r = restricciones[i];
        sf::Color color = colores[i % colores.size()];
        sf::VertexArray linea(sf::Lines, 2);
        double x1, y1, x2, y2;

        if (r.coef_mesas == 0 && r.coef_sillas != 0) {
            y1 = y2 = r.limite / r.coef_sillas;
            x1 = 0;
            x2 = max_x;
        }
        else if (r.coef_sillas == 0 && r.coef_mesas != 0) {
            x1 = x2 = r.limite / r.coef_mesas;
            y1 = 0;
            y2 = max_y;
        }
        else {
            x1 = r.limite / r.coef_mesas;
            y1 = 0;
            x2 = 0;
            y2 = r.limite / r.coef_sillas;
            x1 = std::min(x1, max_x);
            y2 = std::min(y2, max_y);
        }

        linea[0].position = sf::Vector2f(50 + x1 * escala_x, 550 - y1 * escala_y);
        linea[1].position = sf::Vector2f(50 + x2 * escala_x, 550 - y2 * escala_y);
        linea[0].color = linea[1].color = color;
        lineasRestricciones.push_back(linea);

        std::ostringstream oss;
        oss << r.coef_mesas << "x + " << r.coef_sillas << "y <= " << r.limite;
        sf::Text textoRestriccion;
        textoRestriccion.setFont(font);
        textoRestriccion.setString(oss.str());
        textoRestriccion.setCharacterSize(16);
        textoRestriccion.setFillColor(color);
        textoRestriccion.setStyle(sf::Text::Bold);
        textoRestriccion.setPosition(450, 30 + i * 30);
        textosRestricciones.push_back(textoRestriccion);
    }

    // Texto de solución óptima
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

        // Dibujar área factible
        if (puntosUnicos.size() >= 3) {
            ventana.draw(areaFactibleCombinada);
        }

        // Dibujar ejes
        ventana.draw(ejeX, 2, sf::Lines);
        ventana.draw(ejeY, 2, sf::Lines);

        // Dibujar marcadores
        for (const auto& marcador : marcadoresX) ventana.draw(marcador);
        for (const auto& marcador : marcadoresY) ventana.draw(marcador);

        // Dibujar etiquetas
        ventana.draw(labelX);
        ventana.draw(labelY);

        // Dibujar restricciones
        for (const auto& linea : lineasRestricciones) ventana.draw(linea);
        for (const auto& texto : textosRestricciones) ventana.draw(texto);

        // Dibujar punto óptimo
        if (solucionValida) {
            sf::CircleShape puntoOptimo(6);
            puntoOptimo.setFillColor(sf::Color::Red);
            puntoOptimo.setOutlineColor(sf::Color::Black);
            puntoOptimo.setOutlineThickness(1);
            puntoOptimo.setPosition(50 + solucion.mesas * escala_x - 6, 550 - solucion.sillas * escala_y - 6);
            ventana.draw(puntoOptimo);
        }

        // Dibujar información de solución
        ventana.draw(textoSolucion);

        ventana.display();
    }
}