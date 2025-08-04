#include "optimizacion.h"
#include <SFML/Graphics.hpp>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

void mostrarGrafica(const Solucion& solucion,
    const std::vector<Restriccion>& restricciones) {

    // Configuración de ventana responsiva
    sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
    unsigned int windowWidth = std::min(desktopMode.width - 100, 1200u);
    unsigned int windowHeight = std::min(desktopMode.height - 100, 800u);
    sf::RenderWindow ventana(sf::VideoMode(windowWidth, windowHeight), "Solución de Optimización");
    ventana.setFramerateLimit(60);

    // Márgenes responsivos
    float marginLeft = windowWidth * 0.07f;
    float marginRight = windowWidth * 0.05f;
    float marginTop = windowHeight * 0.1f;
    float marginBottom = windowHeight * 0.15f;

    float graphWidth = windowWidth - marginLeft - marginRight;
    float graphHeight = windowHeight - marginTop - marginBottom;
    float originY = windowHeight - marginBottom;

    // Fuente
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
            std::cerr << "Error al cargar la fuente\n";
            return;
        }
    }

    // Configuración del gráfico
    double max_x = std::max(10.0, solucion.mesas * 1.5);
    double max_y = std::max(10.0, solucion.sillas * 1.5);

    for (const auto& r : restricciones) {
        if (r.coef_mesas != 0) max_x = std::max(max_x, r.limite / r.coef_mesas * 1.2);
        if (r.coef_sillas != 0) max_y = std::max(max_y, r.limite / r.coef_sillas * 1.2);
    }

    double escala_x = graphWidth / max_x;
    double escala_y = graphHeight / max_y;

    // Colores modernos
    sf::Color backgroundColor(245, 245, 245);
    sf::Color axisColor(50, 50, 50);
    sf::Color textColor(30, 30, 30);

    std::vector<sf::Color> constraintColors = {
        sf::Color(255, 87, 34),   // Naranja
        sf::Color(33, 150, 243),  // Azul
        sf::Color(76, 175, 80),   // Verde
        sf::Color(156, 39, 176),  // Púrpura
        sf::Color(244, 67, 54)    // Rojo
    };

    sf::Color feasibleAreaColor(100, 255, 100, 150);
    sf::Color optimalPointColor(255, 23, 68);

    // Ejes
    sf::VertexArray ejeX(sf::Lines, 2);
    ejeX[0] = sf::Vertex(sf::Vector2f(marginLeft, originY), axisColor);
    ejeX[1] = sf::Vertex(sf::Vector2f(windowWidth - marginRight, originY), axisColor);

    sf::VertexArray ejeY(sf::Lines, 2);
    ejeY[0] = sf::Vertex(sf::Vector2f(marginLeft, originY), axisColor);
    ejeY[1] = sf::Vertex(sf::Vector2f(marginLeft, marginTop), axisColor);

    // Flechas
    sf::ConvexShape arrowX(3), arrowY(3);
    arrowX.setPoint(0, sf::Vector2f(windowWidth - marginRight, originY));
    arrowX.setPoint(1, sf::Vector2f(windowWidth - marginRight - 10, originY - 5));
    arrowX.setPoint(2, sf::Vector2f(windowWidth - marginRight - 10, originY + 5));
    arrowX.setFillColor(axisColor);

    arrowY.setPoint(0, sf::Vector2f(marginLeft, marginTop));
    arrowY.setPoint(1, sf::Vector2f(marginLeft - 5, marginTop + 10));
    arrowY.setPoint(2, sf::Vector2f(marginLeft + 5, marginTop + 10));
    arrowY.setFillColor(axisColor);

    // Grid y marcadores
    sf::VertexArray gridLines(sf::Lines);
    std::vector<sf::Text> marcadoresX, marcadoresY;

    for (int i = 0; i <= max_x; i += std::max(1, static_cast<int>(max_x / 10))) {
        float xPos = marginLeft + i * escala_x;

        sf::Text marcador;
        marcador.setFont(font);
        marcador.setString(std::to_string(i));
        marcador.setCharacterSize(14);
        marcador.setFillColor(textColor);
        marcador.setPosition(xPos - 10, originY + 5);
        marcadoresX.push_back(marcador);

        gridLines.append(sf::Vertex(sf::Vector2f(xPos, originY), sf::Color(200, 200, 200, 100)));
        gridLines.append(sf::Vertex(sf::Vector2f(xPos, marginTop), sf::Color(200, 200, 200, 100)));
    }

    for (int i = 0; i <= max_y; i += std::max(1, static_cast<int>(max_y / 10))) {
        float yPos = originY - i * escala_y;

        sf::Text marcador;
        marcador.setFont(font);
        marcador.setString(std::to_string(i));
        marcador.setCharacterSize(14);
        marcador.setFillColor(textColor);
        marcador.setPosition(marginLeft - 30, yPos - 10);
        marcadoresY.push_back(marcador);

        gridLines.append(sf::Vertex(sf::Vector2f(marginLeft, yPos), sf::Color(200, 200, 200, 100)));
        gridLines.append(sf::Vertex(sf::Vector2f(windowWidth - marginRight, yPos), sf::Color(200, 200, 200, 100)));
    }

    // Calcular puntos factibles
    std::vector<sf::Vector2f> puntosFactibles;

    // Puntos en los ejes
    puntosFactibles.push_back(sf::Vector2f(marginLeft, originY)); // Origen
    puntosFactibles.push_back(sf::Vector2f(marginLeft + max_x * escala_x, originY));
    puntosFactibles.push_back(sf::Vector2f(marginLeft, originY - max_y * escala_y));

    // Intersecciones entre restricciones
    for (size_t i = 0; i < restricciones.size(); ++i) {
        const auto& r1 = restricciones[i];

        // Con ejes
        if (r1.coef_mesas != 0) {
            double x_axis = r1.limite / r1.coef_mesas;
            if (x_axis <= max_x) {
                puntosFactibles.push_back(sf::Vector2f(marginLeft + x_axis * escala_x, originY));
            }
        }

        if (r1.coef_sillas != 0) {
            double y_axis = r1.limite / r1.coef_sillas;
            if (y_axis <= max_y) {
                puntosFactibles.push_back(sf::Vector2f(marginLeft, originY - y_axis * escala_y));
            }
        }

        // Con otras restricciones
        for (size_t j = i + 1; j < restricciones.size(); ++j) {
            const auto& r2 = restricciones[j];

            double det = r1.coef_mesas * r2.coef_sillas - r2.coef_mesas * r1.coef_sillas;
            if (det != 0) {
                double x = (r2.coef_sillas * r1.limite - r1.coef_sillas * r2.limite) / det;
                double y = (r1.coef_mesas * r2.limite - r2.coef_mesas * r1.limite) / det;

                if (x >= 0 && y >= 0 && x <= max_x && y <= max_y) {
                    puntosFactibles.push_back(sf::Vector2f(
                        marginLeft + x * escala_x,
                        originY - y * escala_y
                    ));
                }
            }
        }
    }

    // Filtrar puntos válidos
    std::vector<sf::Vector2f> puntosValidos;
    for (const auto& punto : puntosFactibles) {
        bool valido = true;
        double x = (punto.x - marginLeft) / escala_x;
        double y = (originY - punto.y) / escala_y;

        for (const auto& r : restricciones) {
            if (r.coef_mesas * x + r.coef_sillas * y > r.limite + 1e-6) {
                valido = false;
                break;
            }
        }

        if (valido) {
            puntosValidos.push_back(punto);
        }
    }

    // Crear área factible
    sf::ConvexShape areaFactibleCombinada;
    if (!puntosValidos.empty()) {
        // Ordenar puntos en sentido horario
        sf::Vector2f centro(0, 0);
        for (const auto& p : puntosValidos) {
            centro += p;
        }
        centro.x /= puntosValidos.size();
        centro.y /= puntosValidos.size();

        std::sort(puntosValidos.begin(), puntosValidos.end(),
            [centro](const sf::Vector2f& a, const sf::Vector2f& b) {
                return atan2(a.y - centro.y, a.x - centro.x) < atan2(b.y - centro.y, b.x - centro.x);
            });

        areaFactibleCombinada.setPointCount(puntosValidos.size());
        for (size_t i = 0; i < puntosValidos.size(); ++i) {
            areaFactibleCombinada.setPoint(i, puntosValidos[i]);
        }
        areaFactibleCombinada.setFillColor(feasibleAreaColor);
        areaFactibleCombinada.setOutlineColor(sf::Color::Green);
        areaFactibleCombinada.setOutlineThickness(1.5f);
    }

    // Resto del código (restricciones, texto, etc.)...
    std::vector<sf::VertexArray> lineasRestricciones;
    std::vector<sf::Text> textosRestricciones;

    for (size_t i = 0; i < restricciones.size(); ++i) {
        const auto& r = restricciones[i];
        sf::Color color = constraintColors[i % constraintColors.size()];

        sf::VertexArray linea(sf::Lines, 2);
        double x1 = (r.coef_mesas != 0) ? r.limite / r.coef_mesas : 0;
        double y1 = 0;
        double x2 = 0;
        double y2 = (r.coef_sillas != 0) ? r.limite / r.coef_sillas : 0;

        x1 = std::min(x1, max_x);
        y2 = std::min(y2, max_y);

        linea[0].position = sf::Vector2f(marginLeft + x1 * escala_x, originY - y1 * escala_y);
        linea[1].position = sf::Vector2f(marginLeft + x2 * escala_x, originY - y2 * escala_y);
        linea[0].color = color;
        linea[1].color = color;
        lineasRestricciones.push_back(linea);

        std::ostringstream oss;
        oss << r.coef_mesas << "x + " << r.coef_sillas << "y <= " << r.limite;

        sf::Text textoRestriccion;
        textoRestriccion.setFont(font);
        textoRestriccion.setString(oss.str());
        textoRestriccion.setCharacterSize(16);
        textoRestriccion.setFillColor(color);
        textoRestriccion.setStyle(sf::Text::Bold);
        textoRestriccion.setPosition(windowWidth * 0.6f, marginTop + i * 30);
        textosRestricciones.push_back(textoRestriccion);
    }

    // Texto solución óptima
    std::ostringstream ossSol;
    ossSol << "Solución Óptima:\n";
    ossSol << "Mesas: " << solucion.mesas << "\n";
    ossSol << "Sillas: " << solucion.sillas << "\n";
    ossSol << "Ganancia: $" << std::fixed << std::setprecision(2) << solucion.ganancia;

    sf::Text textoSolucion;
    textoSolucion.setFont(font);
    textoSolucion.setString(ossSol.str());
    textoSolucion.setCharacterSize(18);
    textoSolucion.setFillColor(textColor);
    textoSolucion.setStyle(sf::Text::Bold);
    textoSolucion.setPosition(windowWidth * 0.6f, marginTop + restricciones.size() * 30 + 20);

    // Etiquetas ejes
    sf::Text labelX;
    labelX.setFont(font);
    labelX.setString("Mesas (x)");
    labelX.setCharacterSize(16);
    labelX.setFillColor(textColor);
    labelX.setPosition(windowWidth - marginRight - 50, originY + 20);

    sf::Text labelY;
    labelY.setFont(font);
    labelY.setString("Sillas (y)");
    labelY.setCharacterSize(16);
    labelY.setFillColor(textColor);
    labelY.setPosition(marginLeft - 40, marginTop);
    labelY.setRotation(-90);

    // Punto óptimo
    sf::CircleShape puntoOptimo(8);
    puntoOptimo.setFillColor(optimalPointColor);
    puntoOptimo.setOutlineColor(sf::Color::Black);
    puntoOptimo.setOutlineThickness(1.5f);
    puntoOptimo.setOrigin(8, 8);
    puntoOptimo.setPosition(
        marginLeft + solucion.mesas * escala_x,
        originY - solucion.sillas * escala_y
    );

    // Bucle principal
    while (ventana.isOpen()) {
        sf::Event event;
        while (ventana.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                ventana.close();
            }

            if (event.type == sf::Event::Resized) {
                sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
                ventana.setView(sf::View(visibleArea));
            }
        }

        ventana.clear(backgroundColor);

        // Dibujar área factible primero
        if (puntosValidos.size() >= 3) {
            ventana.draw(areaFactibleCombinada);
        }

        // Dibujar grid
        ventana.draw(gridLines);

        // Dibujar ejes
        ventana.draw(ejeX);
        ventana.draw(ejeY);
        ventana.draw(arrowX);
        ventana.draw(arrowY);

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
        ventana.draw(puntoOptimo);

        // Dibujar solución
        ventana.draw(textoSolucion);

        ventana.display();
    }
}