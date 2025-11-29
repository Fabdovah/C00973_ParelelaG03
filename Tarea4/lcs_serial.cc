//Crear matriz DP e inicializarla
#include <bits/stdc++.h>
#include "adn.h"

using namespace std;

int main(int argc, char** argv) {
    int len1 = 20, len2 = 20;
    if (argc >= 3) {
        len1 = atoi(argv[1]);
        len2 = atoi(argv[2]);
    }

    ADN *a1 = new ADN(len1);
    ADN *a2 = new ADN(len2);

    string S1 = a1->toString();
    string S2 = a2->toString();

    int n = S1.size();
    int m = S2.size();

    // Matriz DP (n+1) x (m+1)
    vector<vector<int>> dp(n+1, vector<int>(m+1, 0));

    // Inicialización
    for (int i = 0; i <= n; ++i) dp[i][0] = 0;
    for (int j = 0; j <= m; ++j) dp[0][j] = 0;

    cout << "DP inicializada con tamaño (" << n+1 << " x " << m+1 << ")\n";

    delete a1;
    delete a2;
    return 0;
}
