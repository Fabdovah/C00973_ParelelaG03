Propuesta FINAL:

Algoritmo LCS-DP(S1, S2):
    // S1 largo n, S2 largo m
    n ← longitud(S1)
    m ← longitud(S2)

    Crear matriz DP (n+1) × (m+1) inicializada a 0

    para i ← 1 hasta n:
        para j ← 1 hasta m:
            si S1[i-1] == S2[j-1]:
                DP[i][j] ← DP[i-1][j-1] + 1
            sino:
                DP[i][j] ← max(DP[i-1][j], DP[i][j-1])

    // Reconstrucción:
    i ← n; j ← m; resultado ← ""
    mientras i > 0 y j > 0:
        si S1[i-1] == S2[j-1]:
            resultado ← S1[i-1] + resultado
            i ← i - 1; j ← j - 1
        sino si DP[i-1][j] >= DP[i][j-1]:
            i ← i - 1
        sino:
            j ← j - 1

    retornar (resultado, DP[n][m])



Pseudocódigo MPI


MPI_LCS(S1, S2):
    n ← longitud(S1)
    m ← longitud(S2)
    numDiag ← n + m

    // Para diagonal k (k = 2 .. n+m):
    // índices válidos i: max(1, k-m) .. min(n, k-1)
    // longitud diag Lk = min(n, k-1) - max(1, k-m) + 1

    Inicializar prev2 = vector vacío (diagonal k-2)
    Inicializar prev1 = vector de ceros (diagonal k-1)  // para k=2 puede tener longitud 1

    para k ← 2 hasta n + m:
        calcular start_i ← max(1, k-m)
        calcular end_i ← min(n, k-1)
        Lk ← end_i - start_i + 1
        repartir [0..Lk-1] entre procesos (bloque contiguo)
        cada proceso para su subrango t:
             i ← start_i + t
             j ← k - i
             // mapear índices en prev1/prev2 correctamente:
             // dp[i][j] depende de dp[i-1][j-1] (en prev2), dp[i-1][j] y dp[i][j-1] (en prev1)
             obtener valA = (S1[i-1] == S2[j-1]) ? (valor_prev2_correspondiente+1) : 0
             valB = dp[i-1][j]  (en prev1 índice)
             valC = dp[i][j-1]  (en prev1 índice)
             cur[t] = max(valA, valB, valC)
        sincronizar
        juntar subrangos en cur (gather) → prev2 ← prev1 ; prev1 ← cur
    final: dp[n][m] está en la última diagonal; reconstrucción se hace (puede hacerse en rank 0) 
            guardando las diagonales necesarias o recomputando ruta hacia atrás (ver limitaciones)