// Probable final con medición de tiempo

#include <bits/stdc++.h>
#include <chrono>
#include "adn.h"

using namespace std;
using namespace std::chrono;

pair<string,int> LCS(const string &A, const string &B) {
    int n = A.size();
    int m = B.size();
    vector<vector<int>> dp(n+1, vector<int>(m+1, 0));

    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= m; ++j) {
            if (A[i-1] == B[j-1])
                dp[i][j] = dp[i-1][j-1] + 1;
            else
                dp[i][j] = max(dp[i-1][j], dp[i][j-1]);
        }
    }

    int i = n, j = m;
    string res;
    res.reserve(dp[n][m]);

    while (i > 0 && j > 0) {
        if (A[i-1] == B[j-1]) { 
            res.push_back(A[i-1]);
            i--; j--;
        }
        else if (dp[i-1][j] >= dp[i][j-1]) i--;
        else j--;
    }

    reverse(res.begin(), res.end());
    return {res, dp[n][m]};
}

int main(int argc, char** argv) {
    int len1 = 2000, len2 = 2000;
    if (argc >= 3) {
        len1 = atoi(argv[1]);
        len2 = atoi(argv[2]);
    }

    ADN *a1 = new ADN(len1);
    ADN *a2 = new ADN(len2);

    string S1 = a1->toString();
    string S2 = a2->toString();

    cout << "Longitudes: S1=" << S1.size() << " S2=" << S2.size() << endl;

    auto t0 = high_resolution_clock::now();
    auto out = LCS(S1, S2);
    auto t1 = high_resolution_clock::now();

    double elapsed = duration<double>(t1 - t0).count();

    cout << "LCS length = " << out.second << endl;
    cout << "LCS string = " << out.first << endl;
    cout << "Tiempo (s) = " << elapsed << endl;

    delete a1;
    delete a2;
    return 0;
}
