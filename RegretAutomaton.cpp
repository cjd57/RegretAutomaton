#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <sstream>
#include <iomanip>
#include <future>
#include <thread>
#include <algorithm>
#include <windows.h>

using namespace std;

// 从文件读取第一个整数（忽略空白）
bool read_int_from_file(const string& filename, int& value) {
    ifstream fin(filename);
    if (!fin.is_open()) return false;
    fin >> value;
    fin.close();
    return true;
}

// 运行指定的 exe，输入从 infile 读取，输出捕获到临时文件，
// 并与答案文件 ansfile 中的整数比较。
// 返回：耗时（毫秒），若超时返回 60000，若运行失败或答案错误返回 -1。
long long run_exe(const string& exe_name, const string& infile, const string& ansfile) {
    // 临时输出文件
    const string tmp_out = "temp_out.txt";
    string cmd = exe_name + " < " + infile + " > " + tmp_out;

    auto start = chrono::high_resolution_clock::now();

    auto future = std::async(std::launch::async, [&]() -> int {
        return system(cmd.c_str());
        });

    // 等待最多 60 秒
    if (future.wait_for(std::chrono::seconds(60)) == std::future_status::timeout) {
        // 超时，强制杀死进程
        string kill_cmd = "taskkill /F /IM " + exe_name;
        system(kill_cmd.c_str());
        future.wait();  // 等待进程结束
        // 删除临时文件（如果有）
        remove(tmp_out.c_str());
        return 60000;   // 超时标记
    }

    int ret = future.get();
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);

    // 如果程序返回非零，视为失败
    if (ret != 0) {
        remove(tmp_out.c_str());
        return -1;
    }

    // 读取程序输出和答案文件
    int output_value, answer_value;
    bool ok_out = read_int_from_file(tmp_out, output_value);
    bool ok_ans = read_int_from_file(ansfile, answer_value);

    remove(tmp_out.c_str());  // 清理临时文件

    // 任何文件读取失败或数值不相等，都视为答案错误
    if (!ok_out || !ok_ans || output_value != answer_value) {
        return -1;
    }

    return duration.count();
}

// 从输入文件第一行读取 n, K, L（用于记录）
bool read_nkl(const string& infile, int& n, int& K, int& L) {
    ifstream fin(infile);
    if (!fin.is_open()) return false;
    fin >> n >> K >> L;
    fin.close();
    return true;
}

int main() {
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    vector<string> exes = { "DP.exe", "flow.exe", "regret.exe", "auto.exe" };

    ofstream csv("timing.csv");
    if (!csv.is_open()) {
        cerr << "无法创建 timing.csv" << endl;
        return 1;
    }

    csv << "Test,n,K,L,DP_ms,flow_ms,regret_ms,auto_ms\n";

    const string data_dir = "data-test\\";  // 输入/答案文件所在目录

    for (int i = 1; i <= 30; ++i) {
        string infile = data_dir + "test" + to_string(i) + ".in";
        string ansfile = data_dir + "test" + to_string(i) + ".out";

        int n, K, L;
        if (!read_nkl(infile, n, K, L)) {
            cerr << "无法读取 " << infile << "，跳过。\n";
            continue;
        }

        csv << i << "," << n << "," << K << "," << L;

        for (const string& exe : exes) {
            long long ms = run_exe(exe, infile, ansfile);
            csv << "," << ms;

            if (ms == 60000) {
                cout << "Test " << i << " " << exe << " 超时（>60秒）" << endl;
            }
            else if (ms == -1) {
                cout << "Test " << i << " " << exe << " 错误（运行失败或答案不匹配）" << endl;
            }
            else {
                cout << "Test " << i << " " << exe << " 耗时 " << ms << " ms" << endl;
            }
        }
        csv << "\n";
        csv.flush();
    }

    csv.close();
    cout << "计时完成，结果已写入 timing.csv" << endl;
    return 0;
}