#include<bits/stdc++.h>
using namespace std;
const int N=1010;

int n,m;

char g[N][N];
bool st[N][N];
const int dx[]={0,1,0,-1},dy[]={1,0,-1,0};
int c1,c2,c3;

void dfs(int x,int y)
{
    st[x][y]=1;
    for(int i=0;i<4;i++)
    {
        int a=x+dx[i],b=y+dy[i];
        if(a<0||a>n+1||b<0||b>m+1) continue;
        if(st[a][b]) continue;
        if(g[a][b]=='#') continue;
        dfs(a,b);
    }
}

int dfs2(int x,int y)
{
    int cnt=0;
    st[x][y]=1;
    if(g[x][y]=='.') cnt++;
    for(int i=0;i<4;i++)
    {
        int a=x+dx[i],b=y+dy[i];
        if(a<=0||a>=n+1||b<=0||b>=m+1) continue;
        if(st[a][b]) continue;
        cnt+=dfs2(a,b);
    }
    return cnt;
}

void solve(){
    cin >> n >> m;
    for(int i=1;i<=n;i++) scanf("%s",g[i]+1);

    dfs(0,0);
    for(int i=1;i<=n;i++)
        for(int j=1;j<=m;j++)
        {
            if(st[i][j]) continue;
            int t=dfs2(i,j);
            // cout << t << "\n";
            if(t==0) c1++;
            else if(t==1) c2++;
            else c3++;
        }

    // for(int i=0;i<=n+1;i++)
    // {
    //     for(int j=0;j<=m+1;j++)
    //         printf("%d ",st[i][j]);
    //     puts("");
    // }
    printf("%d %d %d\n",c1,c2,c3);
}

signed main(){
    int T=1;
    while(T--){
        solve();
    }
    return 0;
}