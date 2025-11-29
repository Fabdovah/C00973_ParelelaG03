//Reconstruir la subsecuencia LCS completa


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

    vector<vector<int>> dp(n+1, vector<int>(m+1, 0));

    // Llenar DP
    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= m; ++j) {
            if (S1[i-1] == S2[j-1])
                dp[i][j] = dp[i-1][j-1] + 1;
            else
                dp[i][j] = max(dp[i-1][j], dp[i][j-1]);
        }
    }

    // Reconstrucción
    int i = n, j = m;
    string lcs;

    while (i > 0 && j > 0) {
        if (S1[i-1] == S2[j-1]) {
            lcs.push_back(S1[i-1]);
            i--; j--;
        }
        else if (dp[i-1][j] >= dp[i][j-1]) {
            i--;
        }
        else {
            j--;
        }
    }

    reverse(lcs.begin(), lcs.end());

    cout << "Longitud LCS = " << dp[n][m] << endl;
    cout << "LCS = " << lcs << endl;

    delete a1;
    delete a2;
    return 0;
}
