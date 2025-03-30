#include <iostream>
#include <chrono>
using namespace std;
using namespace std::chrono;

// 平凡算法
void normal(int* a, int* b, int* re, int n) {
    for(int j = 0; j < n; j++) {
        re[j] = 0;
        for(int i = 0; i < n; i++) {
            re[j] += a[i * n + j] * b[i];
        }
    }
}

// cache优化算法
void cache(int* a, int* b, int* re, int n) {
    for(int i = 0; i < n; i++) {
        re[i] = 0;
    }
    for(int i = 0; i < n; i++) {
        int p = b[i];
        for(int j = 0; j < n; j++) {
            re[j] += a[i * n + j] * p;
        }
    }
}

// 测试函数
void run_test(int* a, int* b, int* re, int n, const char* name, void (*func)(int*, int*, int*, int)) {
    const int iterations = 10000;
    volatile int dummy = 0;

    // 预热
    for(int i = 0; i < 10; i++) {
        func(a, b, re, n);
    }

    auto start = high_resolution_clock::now();
    for(int i = 0; i < iterations; i++) {
        func(a, b, re, n);
        dummy += re[0];  // 防止优化
    }
    auto end = high_resolution_clock::now();
    auto elapsed = duration_cast<nanoseconds>(end - start).count();

    cout << name << " time: " << elapsed / iterations << " ns/call" << endl;

    // 输出最后一次结果验证正确性
    if(dummy != 0) {  // 只是为了避免警告
        cout << "Result: [";
        for(int i = 0; i < n; i++) {
            cout << re[i] << (i < n-1 ? ", " : "");
        }
        cout << "]" << endl;
    }
}

int main() {
    const int n = 100;  // 更大的矩阵更能体现差异
    int matrix[n * n];
    int vec[n];
    int result[n] = {0};

    // 初始化数据
    for(int i = 0; i < n * n; i++) {
        matrix[i] = i % 10;
    }
    for(int i = 0; i < n; i++) {
        vec[i] = i % 5;
    }

    // 测试normal算法
    run_test(matrix, vec, result, n, "Normal", normal);

    // 测试cache算法
    run_test(matrix, vec, result, n, "Cache", cache);

    return 0;
}
