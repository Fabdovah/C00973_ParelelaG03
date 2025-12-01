//Versión final MPI para LCS usando diagonales (wavefront)


#include <bits/stdc++.h>
#include <mpi.h>
#include "adn.h"
using namespace std;

pair<string,int> LCS_serial_local(const string &A,const string &B){
    int n=A.size(), m=B.size();
    vector<vector<int>> dp(n+1, vector<int>(m+1,0));
    for(int i=1;i<=n;i++)
        for(int j=1;j<=m;j++)
            dp[i][j] = (A[i-1]==B[j-1]) ? dp[i-1][j-1]+1 : max(dp[i-1][j], dp[i][j-1]);

    int i=n,j=m;
    string r; r.reserve(dp[n][m]);
    while(i>0 && j>0){
        if(A[i-1]==B[j-1]){ r.push_back(A[i-1]); i--; j--; }
        else if(dp[i-1][j]>=dp[i][j-1]) i--;
        else j--;
    }
    reverse(r.begin(), r.end());
    return {r, dp[n][m]};
}

int main(int argc,char**argv){
    MPI_Init(&argc,&argv);

    int rank,nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&nprocs);

    int len1=2000,len2=2000;
    if(argc>=3){len1=atoi(argv[1]);len2=atoi(argv[2]);}

    string S1,S2;
    if(rank==0){
        ADN *a1=new ADN(len1);
        ADN *a2=new ADN(len2);
        S1=a1->toString();
        S2=a2->toString();
        delete a1; delete a2;
    }

    int n=0,m=0;
    if(rank==0){n=S1.size();m=S2.size();}
    MPI_Bcast(&n,1,MPI_INT,0,MPI_COMM_WORLD);
    MPI_Bcast(&m,1,MPI_INT,0,MPI_COMM_WORLD);

    if(rank!=0){S1.resize(n);S2.resize(m);}
    MPI_Bcast(&S1[0],n,MPI_CHAR,0,MPI_COMM_WORLD);
    MPI_Bcast(&S2[0],m,MPI_CHAR,0,MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    double t0 = MPI_Wtime();

    vector<int> prev1,prev2,cur;

    for(int k=2;k<=n+m;k++){
        int start_i=max(1,k-m);
        int end_i=min(n,k-1);
        int Lk=end_i-start_i+1;
        if(Lk<=0) continue;

        int base=Lk/nprocs;
        int rem =Lk%nprocs;

        int local_len = base + (rank<rem?1:0);
        int offset;
        if(rank<rem) offset = rank*(base+1);
        else offset = rem*(base+1)+(rank-rem)*base;

        vector<int> local_cur(local_len);

        int start_prev1=max(1,(k-1)-m);
        int start_prev2=max(1,(k-2)-m);

        for(int t=0;t<local_len;t++){
            int idx_global=offset+t;
            int i=start_i+idx_global;
            int j=k-i;

            int valA=0,valB=0,valC=0;

            int idxB=(i-1)-start_prev1;
            if(idxB>=0 && idxB<(int)prev1.size())
                valB=prev1[idxB];

            int idxC=i-start_prev1;
            if(idxC>=0 && idxC<(int)prev1.size())
                valC=prev1[idxC];

            int idxA=(i-1)-start_prev2;
            if(idxA>=0 && idxA<(int)prev2.size())
                valA=prev2[idxA];

            if(S1[i-1]==S2[j-1]) local_cur[t]=valA+1;
            else local_cur[t]=max(valB,valC);
        }

        vector<int> counts(nprocs), displs(nprocs);
        for(int r=0;r<nprocs;r++)
            counts[r]=base+(r<rem?1:0);
        displs[0]=0;
        for(int r=1;r<nprocs;r++)
            displs[r]=displs[r-1]+counts[r-1];

        cur.assign(Lk,0);
        MPI_Allgatherv(local_cur.data(), local_len, MPI_INT,
                       cur.data(), counts.data(), displs.data(), MPI_INT,
                       MPI_COMM_WORLD);

        prev2.swap(prev1);
        prev1.swap(cur);
    }

    int result=0;
    if(!prev1.empty()){
        int k_last=n+m;
        int start_last=max(1,k_last-m);
        int idx=n-start_last;
        if(idx>=0 && idx<(int)prev1.size())
            result=prev1[idx];
    }

    double t1=MPI_Wtime();
    double elapsed=t1-t0;

    double max_elapsed;
    MPI_Reduce(&elapsed,&max_elapsed,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);

    if(rank==0){
        cout<<"[MPI] LCS length = "<<result<<endl;
        cout<<"[MPI] Tiempo (s) = "<<max_elapsed<<endl;

        long long mem_est=(long long)(n+1)*(long long)(m+1)*sizeof(int);
        if(mem_est < 800ll*1024ll*1024ll){
            auto rec=LCS_serial_local(S1,S2);
            cout<<"[Reconstruction] LCS length = "<<rec.second<<endl;
            cout<<"[Reconstruction] LCS = "<<rec.first<<endl;
        } else {
            cout<<"Reconstrucción omitida (matriz muy grande).\n";
        }
    }

    MPI_Finalize();
    return 0;
}
