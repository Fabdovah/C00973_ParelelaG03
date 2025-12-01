
//Recorrer todas las diagonales y distribuirlas, Falta DP

#include <bits/stdc++.h>
#include <mpi.h>
#include "adn.h"
using namespace std;

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank,nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&nprocs);

    int len1=20,len2=20;
    if(argc>=3){len1=atoi(argv[1]);len2=atoi(argv[2]);}

    string S1,S2;
    if(rank==0){
        ADN *a1=new ADN(len1);
        ADN *a2=new ADN(len2);
        S1=a1->toString();
        S2=a2->toString();
        delete a1;delete a2;
    }

    int n=0,m=0;
    if(rank==0){n=S1.size();m=S2.size();}
    MPI_Bcast(&n,1,MPI_INT,0,MPI_COMM_WORLD);
    MPI_Bcast(&m,1,MPI_INT,0,MPI_COMM_WORLD);

    if(rank!=0){S1.resize(n);S2.resize(m);}
    MPI_Bcast(&S1[0],n,MPI_CHAR,0,MPI_COMM_WORLD);
    MPI_Bcast(&S2[0],m,MPI_CHAR,0,MPI_COMM_WORLD);

    vector<int> prev1, prev2, cur;

    for(int k=2; k<=n+m; k++){
        int start_i=max(1, k-m);
        int end_i=min(n, k-1);
        int Lk=end_i-start_i+1;

        if(Lk<=0) continue;

        int base=Lk/nprocs;
        int rem =Lk%nprocs;

        int local_len = base+(rank<rem?1:0);
        int offset;
        if(rank<rem) offset = rank*(base+1);
        else offset = rem*(base+1) + (rank-rem)*base;

        vector<int> local_cur(local_len);

        for(int t=0;t<local_len;t++)
            local_cur[t]=1000*rank + t;  // dummy

        vector<int> counts(nprocs), displs(nprocs);
        for(int r=0;r<nprocs;r++)
            counts[r]= base + (r<rem?1:0);
        displs[0]=0;
        for(int r=1;r<nprocs;r++) displs[r]=displs[r-1]+counts[r-1];

        cur.assign(Lk,0);
        MPI_Allgatherv(local_cur.data(), local_len, MPI_INT,
                       cur.data(), counts.data(), displs.data(), MPI_INT,
                       MPI_COMM_WORLD);

        prev2.swap(prev1);
        prev1.swap(cur);
    }

    if(rank==0) cout<<"Completo \n";

    MPI_Finalize();
    return 0;
}
