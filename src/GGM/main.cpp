#include <iostream>
#include <cmath>
#include <random>
#include <vector>
#include "libxl.h"
#include <Eigen/Dense>

using namespace libxl;
using namespace Eigen;
using namespace std;

// Estructura para reporte en Excel
struct ReporteXLS {
    Book* book;
    Sheet* resultados;
    int fila_actual;
} *reporte_xls;

// Parámetros del sistema
float alpha1 = 3.5, lambda1;
float alpha2 = 4.3, lambda2 = 7.5;
int num_experimentos = 20;
int num_clientes_espera;
vector<double> lambda_vals;
vector<double> p_cola_vals;

// Generador de números aleatorios
random_device rd;
mt19937 gen(rd());

double gamma_distribution(float alpha, float lambda) {
    gamma_distribution<double> gamma_dist(alpha, 1.0 / lambda);
    return gamma_dist(gen);
}

void simular(float lambda1) {
    num_clientes_espera = 0;
    int clientes_en_cola = 0;
    float tiempo_simulacion = 0.0;
    float tiempo_sig_llegada = gamma_distribution(alpha1, lambda1);
    float tiempo_sig_salida = 1.0e+30;

    for (int i = 0; i < num_experimentos; i++) {
        if (tiempo_sig_llegada < tiempo_sig_salida) {
            tiempo_simulacion = tiempo_sig_llegada;
            if (clientes_en_cola == 0)
                tiempo_sig_salida = tiempo_simulacion + gamma_distribution(alpha2, lambda2);
            else
                clientes_en_cola++;
            tiempo_sig_llegada = tiempo_simulacion + gamma_distribution(alpha1, lambda1);
        } else {
            tiempo_simulacion = tiempo_sig_salida;
            if (clientes_en_cola > 0) {
                clientes_en_cola--;
                tiempo_sig_salida = tiempo_simulacion + gamma_distribution(alpha2, lambda2);
            } else {
                tiempo_sig_salida = 1.0e+30;
            }
        }
        if (clientes_en_cola > 0) num_clientes_espera++;
    }
    float p_cola = (float)num_clientes_espera / num_experimentos;
    reporte_xls->resultados->writeNum(reporte_xls->fila_actual++, 1, lambda1);
    reporte_xls->resultados->writeNum(reporte_xls->fila_actual, 2, p_cola);
    
    lambda_vals.push_back(lambda1);
    p_cola_vals.push_back(p_cola);
}

void ajustar_polinomio() {
    int grado = 3;
    int n = lambda_vals.size();
    MatrixXd A(n, grado + 1);
    VectorXd b(n);

    for (int i = 0; i < n; i++) {
        double x = lambda_vals[i];
        for (int j = 0; j <= grado; j++) {
            A(i, j) = pow(x, j);
        }
        b(i) = p_cola_vals[i];
    }

    VectorXd coeficientes = A.householderQr().solve(b);
    cout << "Coeficientes del polinomio ajustado:" << endl;
    for (int i = 0; i <= grado; i++) {
        cout << "a" << i << " = " << coeficientes[i] << endl;
    }
}

int main() {
    reporte_xls = new ReporteXLS;
    reporte_xls->book = xlCreateBook();
    reporte_xls->resultados = reporte_xls->book->addSheet("Resultados");
    reporte_xls->fila_actual = 1;
    reporte_xls->resultados->writeStr(0, 1, "Lambda1");
    reporte_xls->resultados->writeStr(0, 2, "P_cola");
    
    for (lambda1 = 1.0; lambda1 <= 10.0; lambda1 += 0.5) {
        simular(lambda1);
    }
    
    reporte_xls->book->save("resultados.xls");
    reporte_xls->book->release();
    delete reporte_xls;
    
    ajustar_polinomio();
    return 0;
}
