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

    // Márgenes responsivos (porcentajes del tamaño de ventana)
    float marginLeft = windowWidth * 0.07f;
    float marginRight = windowWidth * 0.05f;
    float marginTop = windowHeight * 0.1f;
    float marginBottom = windowHeight * 0.15f;

    // Área útil para el gráfico
    float graphWidth = windowWidth - marginLeft - marginRight;
    float graphHeight = windowHeight - marginTop - marginBottom;
    float originY = windowHeight - marginBottom; // Posición Y del origen (eje X)

    // Fuente - Manejo robusto
    sf::Font font;
    if (!font.loadFromFile("arial.ttf") && !font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
        std::cerr << "Error: No se pudo cargar la fuente Arial\n";
        return;
    }

    // 1. Determinar los rangos máximos para los ejes
    double max_x = std::max(10.0, solucion.mesas * 1.5); // Mínimo 10 unidades o 1.5 veces la solución
    double max_y = std::max(10.0, solucion.sillas * 1.5);

    // Ajustar según las restricciones
    for (const auto& r : restricciones) {
        if (r.coef_mesas != 0) max_x = std::max(max_x, r.limite / r.coef_mesas * 1.2);
        if (r.coef_sillas != 0) max_y = std::max(max_y, r.limite / r.coef_sillas * 1.2);
    }

    // 2. Calcular factores de escala
    double escala_x = graphWidth / max_x;  // Píxeles por unidad en X
    double escala_y = graphHeight / max_y; // Píxeles por unidad en Y

    // 3. Configuración de colores
    sf::Color backgroundColor(245, 245, 245); // Fondo gris claro
    sf::Color axisColor(50, 50, 50);         // Ejes gris oscuro
    sf::Color textColor(30, 30, 30);         // Texto casi negro

    // Paleta de colores para restricciones
    std::vector<sf::Color> constraintColors = {
        sf::Color(255, 87, 34),   // Naranja intenso
        sf::Color(33, 150, 243),  // Azul brillante
        sf::Color(76, 175, 80),   // Verde vibrante
        sf::Color(156, 39, 176),  // Púrpura
        sf::Color(244, 67, 54)    // Rojo intenso
    };

    sf::Color feasibleAreaColor(100, 255, 100, 150);  // Área factible verde semi-transparente
    sf::Color optimalPointColor(255, 23, 68);         // Punto óptimo rojo brillante

    // ==================== ELEMENTOS GRÁFICOS PRINCIPALES ====================

    // 1. Ejes coordenados
    sf::VertexArray ejeX(sf::Lines, 2);
    ejeX[0] = sf::Vertex(sf::Vector2f(marginLeft, originY), axisColor);
    ejeX[1] = sf::Vertex(sf::Vector2f(windowWidth - marginRight, originY), axisColor);

    sf::VertexArray ejeY(sf::Lines, 2);
    ejeY[0] = sf::Vertex(sf::Vector2f(marginLeft, originY), axisColor);
    ejeY[1] = sf::Vertex(sf::Vector2f(marginLeft, marginTop), axisColor);

    // 2. Flechas de los ejes
    sf::ConvexShape arrowX(3), arrowY(3);
    arrowX.setPoint(0, sf::Vector2f(windowWidth - marginRight, originY));
    arrowX.setPoint(1, sf::Vector2f(windowWidth - marginRight - 10, originY - 5));
    arrowX.setPoint(2, sf::Vector2f(windowWidth - marginRight - 10, originY + 5));
    arrowX.setFillColor(axisColor);

    arrowY.setPoint(0, sf::Vector2f(marginLeft, marginTop));
    arrowY.setPoint(1, sf::Vector2f(marginLeft - 5, marginTop + 10));
    arrowY.setPoint(2, sf::Vector2f(marginLeft + 5, marginTop + 10));
    arrowY.setFillColor(axisColor);

    // 3. Grid y marcadores numéricos
    sf::VertexArray gridLines(sf::Lines);
    std::vector<sf::Text> marcadoresX, marcadoresY;

    // Marcadores para eje X
    for (int i = 0; i <= max_x; i += std::max(1.0, max_x / 10)) {
        float xPos = marginLeft + i * escala_x;

        sf::Text marcador(std::to_string(i), font, 14);
        marcador.setFillColor(textColor);
        marcador.setPosition(xPos - 10, originY + 5);
        marcadoresX.push_back(marcador);

        // Líneas verticales del grid
        gridLines.append(sf::Vertex(sf::Vector2f(xPos, originY), sf::Color(200, 200, 200, 100)));
        gridLines.append(sf::Vertex(sf::Vector2f(xPos, marginTop), sf::Color(200, 200, 200, 100)));
    }

    // Marcadores para eje Y
    for (int i = 0; i <= max_y; i += std::max(1.0, max_y / 10)) {
        float yPos = originY - i * escala_y;

        sf::Text marcador(std::to_string(i), font, 14);
        marcador.setFillColor(textColor);
        marcador.setPosition(marginLeft - 30, yPos - 10);
        marcadoresY.push_back(marcador);

        // Líneas horizontales del grid
        gridLines.append(sf::Vertex(sf::Vector2f(marginLeft, yPos), sf::Color(200, 200, 200, 100)));
        gridLines.append(sf::Vertex(sf::Vector2f(windowWidth - marginRight, yPos), sf::Color(200, 200, 200, 100)));
    }

    // ==================== CÁLCULO DEL ÁREA FACTIBLE ====================

    std::vector<sf::Vector2f> puntosFactibles;

    // 1. Puntos básicos (origen y extremos de ejes)
    puntosFactibles.push_back(sf::Vector2f(marginLeft, originY)); // Origen (0,0)
    puntosFactibles.push_back(sf::Vector2f(marginLeft + max_x * escala_x, originY)); // (max_x, 0)
    puntosFactibles.push_back(sf::Vector2f(marginLeft, originY - max_y * escala_y)); // (0, max_y)

    // 2. Intersecciones de restricciones con ejes
    for (size_t i = 0; i < restricciones.size(); ++i) {
        const auto& r = restricciones[i];

        // Intersección con eje X (y=0)
        if (r.coef_mesas != 0) {
            double x_intercept = r.limite / r.coef_mesas;
            if (x_intercept <= max_x) {
                puntosFactibles.push_back(sf::Vector2f(
                    marginLeft + x_intercept * escala_x,
                    originY
                ));
            }
        }

        // Intersección con eje Y (x=0)
        if (r.coef_sillas != 0) {
            double y_intercept = r.limite / r.coef_sillas;
            if (y_intercept <= max_y) {
                puntosFactibles.push_back(sf::Vector2f(
                    marginLeft,
                    originY - y_intercept * escala_y
                ));
            }
        }

        // 3. Intersecciones entre restricciones
        for (size_t j = i + 1; j < restricciones.size(); ++j) {
            const auto& r2 = restricciones[j];

            // Resolver sistema de ecuaciones para encontrar intersección
            double det = r.coef_mesas * r2.coef_sillas - r2.coef_mesas * r.coef_sillas;
            if (std::abs(det) > 1e-6) { // Evitar divisiones por cero
                double x = (r2.coef_sillas * r.limite - r.coef_sillas * r2.limite) / det;
                double y = (r.coef_mesas * r2.limite - r2.coef_mesas * r.limite) / det;

                // Solo considerar puntos en el primer cuadrante y dentro de los límites
                if (x >= 0 && y >= 0 && x <= max_x && y <= max_y) {
                    puntosFactibles.push_back(sf::Vector2f(
                        marginLeft + x * escala_x,
                        originY - y * escala_y
                    ));
                }
            }
        }
    }

    // Filtrar puntos que cumplen todas las restricciones
    std::vector<sf::Vector2f> puntosValidos;
    for (const auto& punto : puntosFactibles) {
        // Convertir coordenadas de pantalla a valores del problema
        double x = (punto.x - marginLeft) / escala_x;
        double y = (originY - punto.y) / escala_y;

        bool valido = true;
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

    // Crear polígono del área factible
    sf::ConvexShape areaFactibleCombinada;
    if (puntosValidos.size() >= 3) {
        // Ordenar puntos en sentido horario
        sf::Vector2f centro(0, 0);
        for (const auto& p : puntosValidos) centro += p;
        centro.x /= puntosValidos.size();
        centro.y /= puntosValidos.size();

        std::sort(puntosValidos.begin(), puntosValidos.end(),
            [centro](const sf::Vector2f& a, const sf::Vector2f& b) {
                return std::atan2(a.y - centro.y, a.x - centro.x) < std::atan2(b.y - centro.y, b.x - centro.x);
            });

        areaFactibleCombinada.setPointCount(puntosValidos.size());
        for (size_t i = 0; i < puntosValidos.size(); ++i) {
            areaFactibleCombinada.setPoint(i, puntosValidos[i]);
        }
        areaFactibleCombinada.setFillColor(feasibleAreaColor);
        areaFactibleCombinada.setOutlineColor(sf::Color::Green);
        areaFactibleCombinada.setOutlineThickness(1.5f);
    }

    // ==================== DIBUJO DE RESTRICCIONES ====================

    std::vector<sf::VertexArray> lineasRestricciones;
    std::vector<sf::Text> textosRestricciones;

    for (size_t i = 0; i < restricciones.size(); ++i) {
        const auto& r = restricciones[i];
        sf::Color color = constraintColors[i % constraintColors.size()];

        // Calcular puntos de la línea de restricción
        double x1 = (r.coef_mesas != 0) ? r.limite / r.coef_mesas : 0;
        double y1 = 0;
        double x2 = 0;
        double y2 = (r.coef_sillas != 0) ? r.limite / r.coef_sillas : 0;

        // Ajustar a los límites del gráfico
        x1 = std::min(x1, max_x);
        y2 = std::min(y2, max_y);

        // Crear línea
        sf::VertexArray linea(sf::Lines, 2);
        linea[0].position = sf::Vector2f(marginLeft + x1 * escala_x, originY - y1 * escala_y);
        linea[1].position = sf::Vector2f(marginLeft + x2 * escala_x, originY - y2 * escala_y);
        linea[0].color = color;
        linea[1].color = color;
        lineasRestricciones.push_back(linea);

        // Texto de la restricción
        std::ostringstream oss;
        oss << r.coef_mesas << "x + " << r.coef_sillas << "y <= " << r.limite;

        sf::Text textoRestriccion(oss.str(), font, 16);
        textoRestriccion.setFillColor(color);
        textoRestriccion.setStyle(sf::Text::Bold);
        textoRestriccion.setPosition(windowWidth * 0.6f, marginTop + i * 30);
        textosRestricciones.push_back(textoRestriccion);
    }

    // ==================== INFORMACIÓN DE LA SOLUCIÓN ====================

    // Texto de solución óptima
    std::ostringstream ossSol;
    ossSol << "Solución Óptima:\n"
        << "Mesas: " << solucion.mesas << "\n"
        << "Sillas: " << solucion.sillas << "\n"
        << "Ganancia: $" << std::fixed << std::setprecision(2) << solucion.ganancia;

    sf::Text textoSolucion(ossSol.str(), font, 18);
    textoSolucion.setFillColor(textColor);
    textoSolucion.setStyle(sf::Text::Bold);
    textoSolucion.setPosition(windowWidth * 0.6f, marginTop + restricciones.size() * 30 + 20);

    // Etiquetas de ejes
    sf::Text labelX("Mesas (x)", font, 16);
    labelX.setFillColor(textColor);
    labelX.setPosition(windowWidth - marginRight - 50, originY + 20);

    sf::Text labelY("Sillas (y)", font, 16);
    labelY.setFillColor(textColor);
    labelY.setPosition(marginLeft - 40, marginTop);
    labelY.setRotation(-90);

    // ==================== PUNTO ÓPTIMO (VERSIÓN CORREGIDA) ====================

    sf::CircleShape puntoOptimo(8.0f);
    puntoOptimo.setFillColor(optimalPointColor);
    puntoOptimo.setOutlineColor(sf::Color::Black);
    puntoOptimo.setOutlineThickness(1.5f);
    puntoOptimo.setOrigin(8.0f, 8.0f); // Centrar el punto

    // Conversión exacta de coordenadas
    float punto_x = marginLeft + solucion.mesas * escala_x;
    float punto_y = originY - solucion.sillas * escala_y;
    puntoOptimo.setPosition(punto_x, punto_y);

    // Texto con coordenadas exactas (opcional)
    sf::Text textoCoordenadas;
    textoCoordenadas.setFont(font);
    textoCoordenadas.setString("(" + std::to_string(solucion.mesas) + ", " + std::to_string(solucion.sillas) + ")");
    textoCoordenadas.setCharacterSize(14);
    textoCoordenadas.setFillColor(sf::Color::Black);
    textoCoordenadas.setPosition(punto_x + 10, punto_y - 15);

    // ==================== BUCLE PRINCIPAL DE RENDERIZADO ====================

    while (ventana.isOpen()) {
        sf::Event event;
        while (ventana.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                ventana.close();
            }

            // Manejo de redimensionamiento de ventana
            if (event.type == sf::Event::Resized) {
                sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
                ventana.setView(sf::View(visibleArea));
            }
        }

        ventana.clear(backgroundColor);

        // 1. Dibujar área factible (fondo)
        if (puntosValidos.size() >= 3) {
            ventana.draw(areaFactibleCombinada);
        }

        // 2. Dibujar grid
        ventana.draw(gridLines);

        // 3. Dibujar ejes
        ventana.draw(ejeX);
        ventana.draw(ejeY);
        ventana.draw(arrowX);
        ventana.draw(arrowY);

        // 4. Dibujar marcadores de ejes
        for (const auto& marcador : marcadoresX) ventana.draw(marcador);
        for (const auto& marcador : marcadoresY) ventana.draw(marcador);

        // 5. Dibujar etiquetas de ejes
        ventana.draw(labelX);
        ventana.draw(labelY);

        // 6. Dibujar restricciones
        for (const auto& linea : lineasRestricciones) ventana.draw(linea);
        for (const auto& texto : textosRestricciones) ventana.draw(texto);

        // 7. Dibujar punto óptimo y sus coordenadas
        ventana.draw(puntoOptimo);
        ventana.draw(textoCoordenadas);

        // 8. Dibujar información de solución
        ventana.draw(textoSolucion);

        ventana.display();
    }
}