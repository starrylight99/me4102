#pragma GCC optimize("Ofast")
#include <bits/stdc++.h>

using namespace std;

double load[9] = {0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0};
double brandA[9] = {1.1,0.95,0.82,0.71,0.61,0.53,0.47,0.46,0.55};
double brandB[9] = {0.75,0.65,0.55,0.48,0.43,0.42,0.48,0.58,0.7};
const int min_rt = 18000;
const int years = 15;
const double tariff_base = 0.14;
const double demand_cost = 12.5;
int start_offpeak_rt = 2900, end_offpeak_rt = 10500;
int start_peak_rt = 3900, end_peak_rt = 18200;

int main() {
    ios::sync_with_stdio(0);
    cin.tie(0);
    string line;
    unordered_map<int,int> offpeak_freq, peak_freq;
    fstream fin("../offpeak.csv", ios::in);

    if (fin.is_open()) { // frequency of RT at offpeak hours
        while (start_offpeak_rt <= end_offpeak_rt) {
            getline(fin, line);
            offpeak_freq[start_offpeak_rt-50] = stoi(line);
            //cout << start_offpeak_rt-50 << ' ' << offpeak_freq[start_offpeak_rt-50] << '\n';
            start_offpeak_rt += 100;
        }
    } else {
        cerr << "Can't open offpeak file";
        return 0;
    }
    
    fstream fin2("../peak.csv", ios::in); // frequency of RT at peak hours
    if (fin2.is_open()) {
        while (start_peak_rt <= end_peak_rt) {
            getline(fin2, line);
            peak_freq[start_peak_rt-50] = stoi(line);
            //cout << start_peak_rt-50 << ' ' << peak_freq[start_peak_rt-50] << '\n';
            start_peak_rt += 100;
        }
    } else {
        cerr << "Can't open peak file";
        return 0;
    }

    vector<vector<int>> combi_cost(13, vector<int>(13, 0));
    
    long long opt_cost = INT64_MAX;
    int opt_rt, opt_ch, opt_a, opt_b, opt_ecost;
    for (int rt=18000;rt<=36000;rt+=100) { // iterates for possible total RT of system, frmo 18000-36000
        long long local_cost = 1800*rt/15; // yearly cost of equipment, assuming 15 years lifetime
        local_cost += 12696 * 12.5 * 12; // yearly demand cost
        int min_ch = ceil((double)rt/3000), max_ch = rt/1500;

        int local_opt_ch, local_opt_a, local_opt_b; // optimal chillers at current rt
        long long local_kw_opt_cost = INT64_MAX; // stores the optimal cost based on current RT of system

        for (int ch=min_ch;ch<=max_ch;ch++) {
            double rt_ch = rt/ch; // RT of each chiller
            int best_a, best_b; // stores best configuration of brand A/B, at current RT
            long long best_kw = INT64_MAX; // stores cost required based on best configuration of brand A/B
            for (int num_a=0;num_a<=ch;num_a++) {
                long long curr_kw = 0;
                int num_b = ch - num_a;
                for (auto&[rt_avg,freq]:offpeak_freq) {
                    long long local_opt_kw = INT64_MAX; 
                    for (int a=0;a<=num_a;a++) for (int b=0;b<=num_b;b++) {
                        if (rt_ch*(a+b)<rt_avg || 0.2*rt_ch*(a+b)>rt_avg) continue; // if loading of system is <20% or >100%, skip
                        double loading = rt_avg/(a+b)/rt_ch;
                        int id = upper_bound(load,load+9,loading) - load;
                        double kwrtA = min(brandA[id], brandA[id-1]) + (loading-load[id-1])*10*abs(brandA[id]-brandA[id-1]);
                        double kwrtB = min(brandB[id], brandB[id-1]) + (loading-load[id-1])*10*abs(brandB[id]-brandB[id-1]);
                        local_opt_kw = min(local_opt_kw, (long long)(((double)(kwrtA*a + kwrtB*b)/(a+b)+0.16)*rt_avg));
                    }
                    curr_kw += local_opt_kw * freq;
                }
                for (auto&[rt_avg,freq]:peak_freq) {
                    long long local_opt_kw = INT64_MAX; 
                    for (int a=0;a<=num_a;a++) for (int b=0;b<=num_b;b++) {
                        if (rt_ch*(a+b)<rt_avg || 0.2*rt_ch*(a+b)>rt_avg) continue; // if loading of system is <20% or >100%, skip
                        double loading = rt_avg/(a+b)/rt_ch;
                        int id = upper_bound(load,load+9,loading) - load;
                        double kwrtA = min(brandA[id], brandA[id-1]) + (loading-load[id-1])*10*abs(brandA[id]-brandA[id-1]);
                        double kwrtB = min(brandB[id], brandB[id-1]) + (loading-load[id-1])*10*abs(brandB[id]-brandB[id-1]);
                        local_opt_kw = min(local_opt_kw, (long long)(((double)(kwrtA*a + kwrtB*b)/(a+b)+0.16)*rt_avg));
                    }
                    curr_kw += local_opt_kw * freq * 2;
                }
                if (curr_kw/60/7<best_kw) best_kw = curr_kw/60/7, best_a = num_a, best_b = num_b;
            }
            long long local_kw_cost = 0.14 * best_kw * 365;
            if (local_kw_cost<local_kw_opt_cost)
                local_kw_opt_cost = local_kw_cost, local_opt_a = best_a, local_opt_b = best_b, local_opt_ch = ch;
        }
        if (local_kw_opt_cost + local_cost < opt_cost)
            opt_cost = local_kw_opt_cost + local_cost, opt_ecost = local_kw_opt_cost, 
                    opt_a = local_opt_a, opt_b = local_opt_b, opt_ch = local_opt_ch, opt_rt = rt;
    }
    cout << "Opt cost inc capital: " << opt_cost << '\n';
    cout << "Opt cost w/o captial: " << opt_cost - opt_rt*1800/15 << '\n';
    cout << "Opt energy cost: " << opt_ecost << '\n';
    cout << "Opt chillers: " << opt_ch << '\n';
    cout << "Opt brand A: " << opt_a  << '\n'; 
    cout << "Opt brand B: " << opt_b << '\n';
    cout << "Opt RT: " << opt_rt;

    double rt_ch = opt_rt/opt_ch;
    long long offpeak_energy = 0, peak_energy = 0;
    long long total_rt = 0;
    for (auto&[rt_avg,freq]:offpeak_freq) {
        long long local_opt_kw = INT64_MAX;
        for (int a=0;a<=opt_a;a++) for (int b=0;b<=opt_b;b++) {
            if (rt_ch*(a+b)<rt_avg || 0.2*rt_ch*(a+b)>rt_avg) continue;
            double loading = rt_avg/(a+b)/rt_ch;
            int id = upper_bound(load,load+9,loading) - load;
            double kwrtA = min(brandA[id], brandA[id-1]) + (loading-load[id-1])*10*abs(brandA[id]-brandA[id-1]);
            double kwrtB = min(brandB[id], brandB[id-1]) + (loading-load[id-1])*10*abs(brandB[id]-brandB[id-1]);
            local_opt_kw = min(local_opt_kw, (long long)(((double)(kwrtA*a + kwrtB*b)/(a+b)+0.16)*rt_avg));
        }
        offpeak_energy += local_opt_kw * freq;
        total_rt += rt_avg * freq;
    }
    for (auto&[rt_avg,freq]:peak_freq) {
        long long local_opt_kw = INT64_MAX;
        for (int a=0;a<=opt_a;a++) for (int b=0;b<=opt_b;b++) {
            if (rt_ch*(a+b)<rt_avg || 0.2*rt_ch*(a+b)>rt_avg) continue;
            double loading = rt_avg/(a+b)/rt_ch;
            int id = upper_bound(load,load+9,loading) - load;
            double kwrtA = min(brandA[id], brandA[id-1]) + (loading-load[id-1])*10*abs(brandA[id]-brandA[id-1]);
            double kwrtB = min(brandB[id], brandB[id-1]) + (loading-load[id-1])*10*abs(brandB[id]-brandB[id-1]);
            local_opt_kw = min(local_opt_kw, (long long)(((double)(kwrtA*a + kwrtB*b)/(a+b)+0.16)*rt_avg));
        }
        peak_energy += local_opt_kw * freq;
        total_rt += rt_avg * freq;
    }
    offpeak_energy = offpeak_energy*365/60/7;
    peak_energy = peak_energy*365/60/7;
    cout << "\nOffpeak energy: " << offpeak_energy << '\n';
    cout << "Peak energy: " << peak_energy << '\n';
    cout << "Total energy: " << offpeak_energy + peak_energy << '\n';
    cout << "Plant System Eff: " << (double)(offpeak_energy + peak_energy)/total_rt << '\n';
}
